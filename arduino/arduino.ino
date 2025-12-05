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

const char* PARTITION_LABEL = "program_data"; 

// Global handle for the data partition, found once at setup
static const esp_partition_t* programDataPartition = NULL;
static esp_partition_mmap_handle_t mappedRegion = NULL;

void partitionInit() {
    programDataPartition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        (esp_partition_subtype_t)0x80, // Custom SubType (matching your CSV)
        PARTITION_LABEL
    );
}

esp_err_t writePartition(const esp_partition_t* partition,
                                 const uint8_t* data,
                                 size_t dataSize,
                                 size_t chunkOffset
                                 ) {
    if (!partition || !data) return ESP_ERR_INVALID_ARG;

    // Make sure data fits in the partition
    if (dataSize > partition->size) {
        ESP_LOGE("FLASH", "Data size %u exceeds partition size %u",
                 (unsigned)dataSize, (unsigned)partition->size);
        return ESP_ERR_INVALID_SIZE;
    }

    size_t sectorSize = partition->erase_size;

    size_t fullSectors = dataSize / sectorSize;
    machineLog("fullSectors: %d\n\r", fullSectors);
    size_t hangingBytes = dataSize & (sectorSize - 1);
    esp_err_t err;

    // Erase one sector at a time
    for (size_t offset = 0; offset <= fullSectors; offset += 1) {
        machineLog("offset: %d\n\r", offset);
        size_t sectorOffset = (offset + chunkOffset) * sectorSize;
        size_t dataOffset = offset * sectorSize;
        err = esp_partition_erase_range(partition, sectorOffset, sectorSize);
        if (err != ESP_OK) return err;
        if (offset == fullSectors) {
            sectorSize = hangingBytes;
        }
        err = esp_partition_write(partition, sectorOffset, data + dataOffset, sectorSize);
        if (err != ESP_OK) return err;
    }

    return ESP_OK;
}

esp_err_t readPartition(const esp_partition_t* partition,
                                 const uint8_t* out,
                                 size_t outSize,
                                 size_t chunkOffset
                                 ) {
/*
    size_t sectorSize = partition->erase_size;
    size_t hangingBytes = outSize &
    size_t targetSize = outSize + (sectorSize - 1) & (~(sectorSize - 1));
    for (int i = 0; i < targetSize; i+= sectorSize) {
        esp_partition_read(partition, offset +);
    }
*/
}
int readChar() {
    int c = Serial.read();
    //Serial.println(c);
    return c;
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

void awaitString(char* string, int len) {
    int index = 0;
    while (index < len) {
        while (Serial.available() < 1);
        if (readChar() == string[index]) index++;
        else index = 0;
    }
}

// clear serial noise
struct dataHeader readHeader() {
    char magicBytes[] = "scratch!";
    awaitString(magicBytes, (sizeof magicBytes) - 1);
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

    // Allocate a temporary buffer to receive the serial data
    uint8_t* tempBuffer = (uint8_t*)malloc(h.dataSize);
    if (!tempBuffer) {
        machineLog("FATAL: Failed to allocate %u bytes as a buffer for program data\n\r", h.dataSize);
        strcpy(out->name, "app");
        out->programData = NULL;
        return;
    }

    *(struct dataHeader*)tempBuffer = h;
    int bytesRead = Serial.readBytes(tempBuffer + h.codeOffset, h.dataSize - h.codeOffset);
    machineLog("Successfully read %u bytes\n\r", bytesRead);

    //writePartition(programDataPartition, tempBuffer, h.dataSize, 1);
    free(tempBuffer);

    // Map the flash region to get a pointer
    if (esp_partition_mmap(programDataPartition, 1 * programDataPartition->erase_size, h.dataSize,
                           ESP_PARTITION_MMAP_DATA, (const void**)&out->programData,
                           &mappedRegion) != ESP_OK) {
        machineLog("FATAL: Failed to mmap program_data partition\n\r");
        out->programData = NULL;
        return;
    }

    //out->programData = tempBuffer;

    strcpy(out->name, "app");
    machineLog("Program data written and mapped to pointer %p\n\r", out->programData);
}

void loop() {
}

void setup() {
    Serial.begin(115200);
    partitionInit();

    main();
}
