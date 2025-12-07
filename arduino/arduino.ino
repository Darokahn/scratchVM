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
  #include "firmwareData.h"
  #include "globals.h"
}

struct firmwareHeader* apps = NULL;

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
    machineLog("writing size %d at sector offset %d\n\r", dataSize, chunkOffset);
    if (!partition || !data) return ESP_ERR_INVALID_ARG;

    // Make sure data fits in the partition
    if (dataSize > partition->size) {
        ESP_LOGE("FLASH", "Data size %u exceeds partition size %u",
                 (unsigned)dataSize, (unsigned)partition->size);
        return ESP_ERR_INVALID_SIZE;
    }

    size_t sectorSize = partition->erase_size;
    size_t writeSize = partition->erase_size;

    machineLog("\tsector size: %d\n\r", sectorSize);
    size_t fullSectors = dataSize / sectorSize;
    machineLog("\tfullSectors: %d\n\r", fullSectors);
    size_t hangingBytes = dataSize % sectorSize;
    machineLog("\thanging bytes: %d\n\r", hangingBytes);
    esp_err_t err;

    // Erase one sector at a time
    for (size_t offset = 0; offset <= fullSectors; offset += 1) {
        size_t sectorOffset = (offset + chunkOffset) * sectorSize;
        size_t dataOffset = offset * sectorSize;
        if (offset == fullSectors) {
            if (hangingBytes == 0) break;
            writeSize = hangingBytes;
        }
        machineLog("\toffset: %d\n\r", offset + chunkOffset);
        err = esp_partition_erase_range(partition, sectorOffset, sectorSize);
        if (err != ESP_OK) return err;
        err = esp_partition_write(partition, sectorOffset, data + dataOffset, writeSize);
        if (err != ESP_OK) return err;
    }

    return ESP_OK;
}

esp_err_t readPartition(const esp_partition_t* partition,
                                 uint8_t* out,
                                 size_t dataSize,
                                 size_t chunkOffset
                                 ) {
    // Make sure data fits in the partition
    machineLog("reading size %d at sector offset %d\n\r", dataSize, chunkOffset);
    if (dataSize > partition->size) {
        ESP_LOGE("FLASH", "Data size %u exceeds partition size %u",
                 (unsigned)dataSize, (unsigned)partition->size);
        return ESP_ERR_INVALID_SIZE;
    }

    size_t sectorSize = partition->erase_size;
    size_t readSize = partition->erase_size;

    size_t fullSectors = dataSize / sectorSize;
    machineLog("\tfullSectors: %d\n\r", fullSectors);
    size_t hangingBytes = dataSize & (sectorSize - 1);
    esp_err_t err;

    // Erase one sector at a time
    for (size_t offset = 0; offset <= fullSectors; offset += 1) {
        machineLog("\toffset: %d\n\r", offset);
        size_t sectorOffset = (offset + chunkOffset) * sectorSize;
        size_t dataOffset = offset * sectorSize;
        if (offset == fullSectors) {
            if (hangingBytes == 0) break;
            readSize = hangingBytes;
        }
        err = esp_partition_read(partition, sectorOffset, out + dataOffset, readSize);
        if (err != ESP_OK) return err;
    }

    return ESP_OK;
}

void* mapPartition(const esp_partition_t* partition, size_t size, size_t sectorOffset) {
    machineLog("map size %d at sector offset %d\n\r", size, sectorOffset);
    if (mappedRegion != NULL) {
        machineLog("FATAL: Previous mapping has not been freed.\n\r");
        return NULL;
    }
    void* out;
    esp_err_t err = esp_partition_mmap(partition, sectorOffset * partition->erase_size, size,
                           ESP_PARTITION_MMAP_DATA, (const void**) &out,
                           &mappedRegion);
    machineLog("testing dereference: %c\n\r", *(char*)out);
    if (err != ESP_OK) {
        machineLog("FATAL: Failed to mmap program_data partition\n\r");
        return NULL;
    }
    machineLog("data mapped to pointer %p\n\r", out);
    return out;
}

void unmapPartition() {
    if (mappedRegion == NULL) return;
    esp_partition_munmap(mappedRegion);
    mappedRegion = NULL;
}

int readChar() {
    int c = Serial.read();
    //Serial.println(c);
    return c;
}

extern "C" int main();
extern "C" int runApp(app_t* app);
extern "C" void startIO() {
    pinMode(33, INPUT);
    pinMode(32, INPUT);
    pinMode(25, INPUT_PULLUP);
    tft.init();
    tft.setRotation(1);
    tftSprite.createSprite(LCDWIDTH, LCDHEIGHT);
    tft.fillScreen(TFT_BLACK);
}

extern "C" int updateIO(app_t* app) {
    if (Serial.available()) {
        pollApp(appName);
        return -1;
    }
    tftSprite.pushSprite((FULLLCDWIDTH - LCDWIDTH) / 2, (FULLLCDHEIGHT - LCDHEIGHT) / 2);
    return 0;
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

int MAXOFFSET = 14;

void syncApps() {
    esp_err_t err = writePartition(programDataPartition, (uint8_t*) apps, programDataPartition->erase_size, 0);
    if (err != ESP_OK) {
        machineLog("Sync app write failed: %s\n", esp_err_to_name(err));
    }
}

void initApps() {
    apps = (struct firmwareHeader*) malloc(programDataPartition->erase_size);
    esp_err_t err = readPartition(programDataPartition, (uint8_t*) apps, programDataPartition->erase_size, 0);
    if (err != ESP_OK) {
        machineLog("Init app read failed: %s\n", esp_err_to_name(err));
    }
    if (strncmp(apps->magic, "magic", 6) != 0) {
        machineLog("Expected magic bytes (ascii) say: \"magic\", read \"%s\"\n\r", apps->magic);
        machineLog("No firmware header found. Initializing...\n\r");
        strcpy(apps->magic, "magic");
        apps->version = 0;
        apps->appCount = 0;
        apps->nextOffset = 1;
        syncApps();
    }
}

void clearApps() {
    strcpy(apps->magic, "magic");
    apps->appCount = 0;
    apps->nextOffset = 1;
}

void listApps() {
    machineLog(
        "app header with magic \"%s\", app count %hu, version %hu, next available offset %hu\n\r", 
        apps->magic, apps->appCount, apps->version, apps->nextOffset
    );
    for (int i = 0; i < apps->appCount; i++) {
        struct appDescriptor d = apps->apps[i];
        machineLog("\tapp \"%s\" with size %d at offset %d\n\r", d.name, d.size, d.offset);
    }
}

void awaitString(char* string, int len) {
    int index = 0;
    while (index < len) {
        while (Serial.available() < 1);
        if (readChar() == string[index]) index++;
        else index = 0;
    }
}

extern "C" void* saveApp(char* appName, size_t sectorOffset, uint8_t* programData, size_t programDataSize, bool alsoMap) {
    writePartition(programDataPartition, programData, programDataSize, sectorOffset);
    void* toReturn = NULL;
    if (alsoMap) {
        toReturn = mapPartition(programDataPartition, programDataSize, sectorOffset);
    }
    return toReturn;
}

struct dataHeader readHeader() {
    char magicBytes[] = "scratch!";
    awaitString(magicBytes, (sizeof magicBytes) - 1);
    struct dataHeader h;
    uint8_t* ptr = (uint8_t*)&h.spriteCount;
    while (ptr < (uint8_t*)((&h) + 1)) {
        if (Serial.available() < 1) continue;
        uint8_t b = readChar();
        *ptr++ = b;
    }
    return h;
}

extern "C" void* pollApp(char* nameOut) {
    struct dataHeader h = readHeader();

    machineLog("Reading %u bytes of program data...\n\n\r", h.dataSize - (sizeof h));

    // Allocate a temporary buffer to receive the serial data
    uint8_t* tempBuffer = (uint8_t*)malloc(h.dataSize);
    if (!tempBuffer) {
        machineLog("FATAL: Failed to allocate %u bytes as a buffer for program data\n\r", h.dataSize);
        strcpy(nameOut, "app");
        return NULL;
    }

    *(struct dataHeader*)tempBuffer = h;
    int bytesRead = Serial.readBytes(tempBuffer + (sizeof h), h.dataSize - (sizeof h));
    machineLog("Successfully read %u bytes\n\r", bytesRead);

    saveApp(appName, 1, tempBuffer, h.dataSize, false);
    apps->appCount = 1;

    syncApps();
    void* toReturn = tempBuffer;
    machineLog("testing dereference of at least header size\n\r");
    for (int i = 0; i < sizeof h; i++) {
        machineLog("char at %d: %d\n\r", i, ((char*)toReturn)[i]);
    }
    machineLog("success\n\r");
    apps->appCount = 1;
    //free(tempBuffer);
    return toReturn;
}

extern "C" int selectApp(app_t* out, char* appName) {
    for (int i = 0; i < apps->appCount; i++) {
        struct appDescriptor* d = &(apps->apps[i]);
        int diff = strcmp(d->name, appName);
        machineLog("strcmp(%s, %s) returned %d\n\r", d->name, appName, diff);
        if (diff != 0) continue;
        machineLog("selectApp calling mapPartition\n\r");
        out->programData = (uint8_t*) mapPartition(programDataPartition, d->size, d->offset);
        if (out->programData == NULL) return -1;
        return 0;
    }
    return -1;
}

app_t* loadApp(int offset, int size) {
    return (app_t*) mapPartition(programDataPartition, size, offset);
}

extern "C" void closeApp(app_t* app, int flags) {
    unmapPartition();
}

void loop() {
}

void setup() {
    Serial.begin(115200);
    startIO();
    partitionInit();
    initApps();
    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_PANIC || reason == ESP_RST_INT_WDT) {
        clearApps();
    }
    listApps();
    app_t app;
    if (apps->appCount < 1) {
        app.programData = (uint8_t*)pollApp("");
    }
    else {
        app.programData = (uint8_t*)loadApp(1, 65536);
    }
    runApp(&app);
    ESP.restart();
}
