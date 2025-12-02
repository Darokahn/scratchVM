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
  #include "letters.h"
  #include "externFunctions.h"
}

const char* PARTITION_LABEL = "program_data"; 

// Global handle for the data partition, found once at setup
static const esp_partition_t* g_data_partition = NULL;

void setup_program_data_partition() {
    g_data_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        (esp_partition_subtype_t)0x80, // Custom SubType (matching your CSV)
        PARTITION_LABEL
    );

    if (!g_data_partition) {
        machineLog("FATAL: Partition '%s' not found! Check partitions.csv.", PARTITION_LABEL);
    } else {
        machineLog("Partition '%s' found at 0x%X, size %u bytes.", 
                 PARTITION_LABEL, g_data_partition->address, g_data_partition->size);
    }
}

bool write_program_data(const uint8_t* buffer, size_t length) {
    if (!g_data_partition) {
        machineLog("Partition handle is NULL. Call setup_program_data_partition() first.");
        return false;
    }

    if (length == 0) {
        machineLog("Data length is zero. Skipping write.");
        return true;
    }

    if (length > g_data_partition->size) {
        machineLog("Data length (%u) exceeds partition size (%u). Aborting.", length, g_data_partition->size);
        return false;
    }

    // --- 1. Erase the required flash area ---
    // We must erase blocks of 4KB (0x1000). The erase size must be rounded up 
    // to the nearest 4KB boundary that covers the entire data length.
    size_t erase_size = (length + 0xFFF) & ~0xFFF;
    
    machineLog("Erasing %u bytes for new data...", erase_size);
    esp_err_t err = esp_partition_erase_range(g_data_partition, 0, erase_size);
    if (err != ESP_OK) {
        machineLog("Erase failed: %s", esp_err_to_name(err));
        return false;
    }

    // --- 2. Write the new data payload ---
    machineLog("Writing %u bytes to partition...", length);
    err = esp_partition_write(g_data_partition, 0, buffer, length);
    if (err != ESP_OK) {
        machineLog("Write failed: %s", esp_err_to_name(err));
        return false;
    }

    machineLog("Program data updated successfully.");
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
extern "C" void updateIO() {
    tftSprite.pushSprite((FULLLCDWIDTH - LCDWIDTH) / 2, (FULLLCDHEIGHT - LCDHEIGHT) / 2);
    //Serial.println(ESP.getFreeHeap());
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

void pollApp(app_t* out) {
    uint8_t blockSizeBytes[5];
    blockSizeBytes[4] = 0;
    while (Serial.available() < 4);
    Serial.readBytes(blockSizeBytes, 4);
    Serial.println((char*)blockSizeBytes);
    int blockSize = (blockSizeBytes[3] << 24) + (blockSizeBytes[2] << 16) + (blockSizeBytes[1] << 8) + blockSizeBytes[0];
    strcpy(out->name, "app");
    out->programData = (uint8_t*)programData;
}

void loop() {
}

void setup() {
    delay(1000);
    Serial.begin(115200);
    setup_program_data_partition();
    main();
}
