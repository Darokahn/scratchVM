/*
I WOULD RATHER CODE USING A KEYBOARD MADE OF MY OWN TOENAILS AND FINGERNAILS THAT I HAD TO RIP OFF OF MY BODY
AND GINGERLY PLACE ONTO THE KEYCAPS THAN MAKE AN ENTIRE PROJECT IN C++
*/

#pragma GCC optimize ("O3")

#include <cstdarg>
#include <cstdio>
#include <TFT_eSPI.h>

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite tftSprite = TFT_eSprite(&tft);

#define TS_CS 33
#define TS_IRQ 32

extern "C" {
  #include "scratch.h"
  #include "graphics.h"
  #include "programData.h"
}

int readChar() {
    int c = Serial.read();
    //Serial.println(c);
    return c;
}

const char* PARTITION_LABEL = "program_data"; 

// Global handle for the data partition, found once at setup
static const esp_partition_t* programDataPartition = NULL;
static esp_partition_mmap_handle_t mappedRegion = NULL;

int mappedSections = 0;
#define SECTIONSIZE 65355

void partitionInit() {
    programDataPartition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        (esp_partition_subtype_t)0x80, // Custom SubType (matching your CSV)
        PARTITION_LABEL
    );
}

struct writeProcess {
    int baseOffset;
    int offset;
    int currentIndex;
    uint8_t buffer[4096];
    bool writing = false;
};

struct writeProcess process;

void startReadWrite(int offset) {
    process.baseOffset = offset;
    process.offset = offset;
    process.currentIndex = 0;
    process.writing = true;
}

void endWriteData() {
    writeChunk();
    process.writing = false;
}

void writeChunk() {
    esp_partition_erase_range(programDataPartition, process.offset, programDataPartition->erase_size);
    esp_partition_write(programDataPartition, process.offset, process.buffer, programDataPartition->erase_size);
    process.offset += programDataPartition->erase_size;
    if (process.offset - process.baseOffset > SECTIONSIZE) {
        mappedSections++;
    }
    process.currentIndex = 0;
}

void writeData(const uint8_t* buffer, size_t length) {
    for (int i = 0; i < length; i++) {
        process.buffer[process.currentIndex++] = buffer[i];
        if (process.currentIndex >= programDataPartition->erase_size) {
            writeChunk();
        }
    }
}

#define APPMAX 46

app_t* appStore[APPMAX];
int appOffsets[APPMAX];
int appSizes[APPMAX];
int appCount = 0;

app_t* checkoutApp(char* name) {
    for (int i = 0; i < appCount; i++) {
        app_t* app = appStore[i];
        if (strcmp(app->name, name) != 0) continue;
        int offset = appOffsets[i];
        startReadWrite(offset);
        esp_partition_mmap(programDataPartition, offset, SECTIONSIZE * appSizes[i], ESP_PARTITION_MMAP_DATA, (const void**)&(app->programData), &mappedRegion);
    }
    return NULL;
}


void returnApp() {
}
extern "C" int main();
extern "C" void startIO() {
    tft.init();
    tft.setRotation(1);
    tftSprite.createSprite(LCDWIDTH, LCDHEIGHT);
    tft.fillScreen(TFT_BLACK);
    pinMode(33, INPUT);
    pinMode(32, INPUT);
    pinMode(25, INPUT_PULLUP);
}
extern "C" void updateIO(app_t* app) {
    if (Serial.available()) {
    }
    tftSprite.pushSprite((FULLLCDWIDTH - LCDWIDTH) / 2, (FULLLCDHEIGHT - LCDHEIGHT) / 2);
}

extern "C" void* mallocDMA(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_DMA);
}

extern "C" void drawPixel(int x, int y, pixel color) {
    tftSprite.drawPixel(x, y, color);
}

int frameInterval = 1000 / FRAMESPERSEC;

extern "C" const size_t stampSize = sizeof millis();

extern "C" unsigned long getNow() {
    return millis();
}

#define VRX_PIN 32
#define VRY_PIN 33
#define SW_PIN 25

const int ADC_CENTER = 1880;       // Midpoint of 12-bit ADC
const int DEADZONE  = 200;         // Joystick deadzone threshold

extern "C" bool getInput(int index) {
    int vrx = analogRead(VRX_PIN);
    int vry = analogRead(VRY_PIN);
    bool sw  = digitalRead(SW_PIN) == LOW;  // Active-low button

    bool activated = false;
    switch (index) {
        case 0: // UP
            activated = vry > ADC_CENTER + DEADZONE;
            break;
        case 1: // LEFT
            activated = vrx > ADC_CENTER + DEADZONE;
            break;
        case 2: // DOWN
            activated = vry < ADC_CENTER - DEADZONE;
            break;
        case 3: // RIGHT
            activated = vrx < ADC_CENTER - DEADZONE;
            break;
        case 4: // SPACE / action button
            activated = sw;
            break;
    }
    delay(1);
    return activated;
}

extern "C" int machineLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = Serial.vprintf(fmt, args);
    va_end(args);
    return n;
}

// clear serial noise
struct dataHeader readHeader() {
    char magicBytes[] = "scratch!";
    int index = 0;
    while (index < (sizeof magicBytes) - 1) {
        while (Serial.available() < 1);
        if (readChar() == magicBytes[index]) index++;
        else index = 0;
    }
    struct dataHeader h;
    uint8_t* ptr = (uint8_t*)&h.spriteCount;
    while (ptr < (uint8_t*)((&h) + 1)) {
        if (Serial.available() < 1) continue;
        uint8_t b = readChar();
        machineLog("%d\n\r", b);
        *ptr++ = b;
    }
    return h;
}

void pollApp(app_t* out) {
    struct dataHeader h = readHeader();
    
    machineLog("Reading %u bytes of program data...\n\n\r", h.dataSize);
    
    // Allocate memory for the program data blob
    uint8_t* programDataBuffer = (uint8_t*)malloc(h.dataSize);
    if (!programDataBuffer) {
        machineLog("FATAL: Failed to allocate %u bytes as a buffer for program data\n\r", h.dataSize);
        strcpy(out->name, "app");
        out->programData = NULL;
        return;
    }
    
    // Read the blob data from serial
    *(struct dataHeader*)programDataBuffer = h;
    h = *(struct dataHeader*)programDataBuffer;
    int bytesRead = Serial.readBytes(programDataBuffer + h.codeOffset, h.dataSize - h.codeOffset);
    
    machineLog("Successfully read %u bytes\n\r", bytesRead);
    
    strcpy(out->name, "app");
    out->programData = programDataBuffer;
}

void loop() {
}

void setup() {
    delay(1000);
    Serial.begin(115200);
    partitionInit();
    main();
}
