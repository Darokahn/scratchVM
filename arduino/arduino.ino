/*
I WOULD RATHER CODE USING A KEYBOARD MADE OF MY OWN TOENAILS AND FINGERNAILS THAT I HAD TO RIP OFF OF MY BODY
AND GINGERLY PLACE ONTO THE KEYCAPS THAN MAKE AN ENTIRE PROJECT IN C++
*/

#pragma GCC optimize ("O3")

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <TFT_eSPI.h>

#include "types.h"

#define TFT_MISO 255
#define TFT_MOSI   5
#define TFT_SCLK  18
#define TFT_CS    15  // Chip select control pin
#define TFT_DC     4  // Data Command control pin
#define TFT_RST    2  // Reset pin (could connect to RST pin)
#define TFT_LED   19

#define THUMBSTICK_VCC 26
#define THUMBSTICK_GND 27

#define THUMBSTICK_Y    25
#define THUMBSTICK_X    33
#define THUMBSTICK_SW   32

extern "C" {
    #include <stddef.h>
    #include <sys/param.h>
    #include "scratch.h"
    #include "graphics.h"
    #include "programData.h"
    #include "firmwareData.h"
    #include "globals.h"
}

TFT_eSPI tft; 
TFT_eSprite tftSpriteUpper(&tft);
TFT_eSprite tftSpriteLower(&tft);

firmwareHeader* apps = NULL;

const char* PARTITION_LABEL = "program_data"; 

// Global handle for the data partition, found once at setup
static const esp_partition_t* programDataPartition = NULL;
static esp_partition_mmap_handle_t mappedRegion = NULL;
static int lastMappingLine = NULL;

void partitionInit() {
    programDataPartition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        (esp_partition_subtype_t)0x80,
        PARTITION_LABEL
    );
}

bool verifySectorWrite(const esp_partition_t* partition, uint8_t* data, size_t dataSize, int chunkOffset) {
    uint8_t bytes[4096];
    readPartition(partition, bytes, dataSize, chunkOffset);
    int diff = memcmp(data, bytes, dataSize);
    return diff == 0;
}

esp_err_t writeSector(const esp_partition_t* partition, uint8_t* data, size_t dataSize, int chunkOffset) {
    if (!partition || !data) return ESP_ERR_INVALID_ARG;
    int err;
    // Make sure data fits in the partition
    if (dataSize > partition->erase_size) {
        ESP_LOGE("FLASH", "Data size %u exceeds partition sector size %u",
                 (unsigned)dataSize, (unsigned)partition->size);
        return ESP_ERR_INVALID_SIZE;
    }

    size_t sectorSize = partition->erase_size;
    size_t writeSize = MIN(partition->erase_size, dataSize);
    machineLog("writing sector size %u at offset %d (partition size: %d)\n\r", writeSize, chunkOffset, partition->size);

    size_t offset = 0;
    size_t sectorOffset = chunkOffset * sectorSize;
    err = esp_partition_erase_range(partition, sectorOffset, sectorSize);
    if (err != ESP_OK) return err;
    err = esp_partition_write(partition, sectorOffset, data, writeSize);
    if (err != ESP_OK) return err;

    verifySectorWrite(partition, data, dataSize, chunkOffset);

    return ESP_OK;
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
    if (dataSize > partition->size) {
        ESP_LOGE("FLASH", "Data size %u exceeds partition size %u",
                 (unsigned)dataSize, (unsigned)partition->size);
        return ESP_ERR_INVALID_SIZE;
    }

    size_t sectorSize = partition->erase_size;
    size_t readSize = partition->erase_size;

    size_t fullSectors = dataSize / sectorSize;
    size_t hangingBytes = dataSize & (sectorSize - 1);
    esp_err_t err;

    // one sector at a time
    for (size_t offset = 0; offset <= fullSectors; offset += 1) {
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

void* mapPartitionDebug(const esp_partition_t* partition, size_t size, size_t sectorOffset, int line) {
    if (mappedRegion != NULL) {
        machineLog("FATAL: Previous mapping has not been freed");
        if (lastMappingLine) machineLog("(on line %d)", lastMappingLine);
        machineLog(".\n\r");
        return NULL;
    }
    lastMappingLine = line;
    void* out;
    esp_err_t err = esp_partition_mmap(partition, sectorOffset * partition->erase_size, size,
                           ESP_PARTITION_MMAP_DATA, (const void**) &out,
                           &mappedRegion);
    if (err != ESP_OK) {
        machineLog("FATAL: Failed to mmap program_data partition\n\r");
        return NULL;
    }
    return out;
}

void* mapPartition(const esp_partition_t* partition, size_t size, size_t sectorOffset) {
    if (mappedRegion != NULL) {
        machineLog("FATAL: Previous mapping has not been freed.\n\r");
        machineLog(".\n\r");
        return NULL;
    }
    void* out;
    esp_err_t err = esp_partition_mmap(partition, sectorOffset * partition->erase_size, size,
                           ESP_PARTITION_MMAP_DATA, (const void**) &out,
                           &mappedRegion);
    if (err != ESP_OK) {
        machineLog("FATAL: Failed to mmap program_data partition\n\r");
        return NULL;
    }
    return out;
}

void unmapPartition() {
    if (mappedRegion == NULL) return;
    esp_partition_munmap(mappedRegion);
    mappedRegion = NULL;
}

int readCharInteractive() {
    char c = Serial.read();
    Serial.print(c);
    if (c == '\n') Serial.print('\r');
    else if (c == 0x7f) {
        Serial.print('\b');
        Serial.print(' ');
        Serial.print('\b');
    }
    return c;
}

int readChar() {
    char c = Serial.read();
    return c;
}

extern "C" int main();
extern "C" int runApp(app_t* app);

const int maxHeapAllocation = 100000;
int TOPHEIGHT = 170;
int BOTTOMHEIGHT = FULLLCDHEIGHT - 170;

extern "C" void startIO() {
    hardwareData hardware = apps->hardware;
    printf("hardware data: "); printHardwareData(hardware);
    pinMode(hardware.controls.xAxis, INPUT_PULLUP);
    pinMode(hardware.controls.yAxis, INPUT_PULLUP);
    pinMode(hardware.controls.button, INPUT_PULLUP);
    pinMode(hardware.controls.vcc, OUTPUT);
    digitalWrite(hardware.controls.vcc, HIGH);
    pinMode(hardware.controls.gnd, OUTPUT);
    digitalWrite(hardware.controls.gnd, LOW);

    pinMode(hardware.screen.led, OUTPUT);
    digitalWrite(hardware.screen.led, HIGH);

    tft.~TFT_eSPI();
    int width = hardware.screen.orientation % 2 == 0 ? hardware.screen.width : hardware.screen.height;
    int height = hardware.screen.orientation % 2 == 0 ? hardware.screen.height : hardware.screen.width;
    new (&tft) TFT_eSPI(width, height);
    tftSpriteLower.~TFT_eSprite();
    tftSpriteUpper.~TFT_eSprite();
    new (&tftSpriteLower) TFT_eSprite(&tft);
    new (&tftSpriteUpper) TFT_eSprite(&tft);
    tft.init();
    tft.setRotation(hardware.screen.orientation);
    int remainingHeight = hardware.screen.height;
    TOPHEIGHT = MIN(maxHeapAllocation / (hardware.screen.width * 2), remainingHeight);
    remainingHeight -= TOPHEIGHT;
    BOTTOMHEIGHT = MIN(maxHeapAllocation / hardware.screen.width, remainingHeight);
    tftSpriteUpper.createSprite(hardware.screen.width, TOPHEIGHT);
    tftSpriteLower.createSprite(hardware.screen.width, BOTTOMHEIGHT);
    tft.fillScreen(TFT_BLUE);
}

int ADC_CENTER = -1;       // Midpoint of 12-bit ADC
const int DEADZONE  = 1500;         // Joystick deadzone threshold

bool inputs[5];
extern "C" int updateIO(app_t* app) {
    if (Serial.available()) {
        return -1;
    }
    tftSpriteUpper.pushSprite(0, 0);
    tftSpriteLower.pushSprite(0, TOPHEIGHT);
    int vrx = analogRead(THUMBSTICK_X);
    int vry = analogRead(THUMBSTICK_Y);
    bool sw  = digitalRead(THUMBSTICK_SW) == LOW;  // Active-low button
    if (ADC_CENTER == -1) ADC_CENTER = vry;

    inputs[0] = vry < ADC_CENTER - DEADZONE;
    inputs[1] = vrx > ADC_CENTER + DEADZONE;
    inputs[2] = vry > ADC_CENTER + DEADZONE;
    inputs[3] = vrx < ADC_CENTER - DEADZONE;
    inputs[4] = sw;
    return 0;
}

extern "C" bool getInput(int index) {
    return inputs[index];
}

extern "C" void* mallocDMA(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_DMA);
}

extern "C" void drawPixel(int x, int y, pixel color) {
    static TFT_eSprite* halves[] = {&tftSpriteUpper, &tftSpriteLower};
    TFT_eSprite* screenHalf = halves[y >= TOPHEIGHT];
    y -= (TOPHEIGHT * (y >= TOPHEIGHT));
    screenHalf->drawPixel(x, y, color);
}

int frameInterval = 1000 / FRAMESPERSEC;

extern "C" const size_t stampSize = sizeof millis();

extern "C" uint64_t getNow() {
    return millis();
}

extern "C" int machineLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = Serial.vprintf(fmt, args);
    va_end(args);
    return n;
}

int MAXOFFSET = 14;

#define MAGICSTRING "magic"
#define MAGICSTRINGSIZE (sizeof MAGICSTRING)
#define MAGICSTRINGLEN (MAGICSTRINGSIZE - 1)
#define CURRENTVERSION "f2026.0"
#define CURRENTVERSIONLEN ((sizeof CURRENTVERSION) - 1)

enum colorOrders {
    RGB,
    BGR
};

hardwareData hardwareTemplate = {
    .screen = {
        .cs=TFT_CS,
        .reset=TFT_RST,
        .dc=TFT_DC,
        .sdi=TFT_MOSI,
        .sck=TFT_SCLK,
        .led=TFT_LED,
        .sdo=TFT_MISO,

        .colorOrder=RGB,
        .width=FULLLCDWIDTH,
        .height=FULLLCDHEIGHT,
        .orientation=1,
    },
    .controls = {
        .vcc=THUMBSTICK_VCC,
        .gnd=THUMBSTICK_GND,
        .xAxis=THUMBSTICK_Y,
        .yAxis=THUMBSTICK_X,
        .button=THUMBSTICK_SW
    }
};

hardwareData hardwareTemplateDeprecated = {
    .screen = {
        .cs=15,
        .reset=4,
        .dc=2,
        .sdi=23,
        .sck=18,
        .led=255,
        .sdo=19,
        .width=FULLLCDWIDTH,
        .height=FULLLCDHEIGHT,
        .orientation=1,
    },
    .controls = {
        .vcc=255,
        .gnd=255,
        .xAxis=THUMBSTICK_Y,
        .yAxis=THUMBSTICK_X,
        .button=THUMBSTICK_SW
    }
};

bool overwriteHardware = false;

void syncApps() {
    esp_err_t err = writePartition(programDataPartition, (uint8_t*) apps, programDataPartition->erase_size, 0);
    if (err != ESP_OK) {
        machineLog("Sync app write failed: %s\n", esp_err_to_name(err));
    }
}

bool correctDeprecatedApps(firmwareHeader* appsObject) {
    firmwareHeaderOld* appsDeprecated = (firmwareHeaderOld*) appsObject;
    int comparison = strncmp(appsDeprecated->magic, MAGICSTRING, MAGICSTRINGSIZE);
    machineLog("comparison ('%s', '%s'): %d\n", appsDeprecated->magic, MAGICSTRING, comparison);
    if (comparison != 0) return false;
    *appsObject = (firmwareHeader) {0};
    machineLog("deprecated, updating\n\r");
    appsObject->appCount = 0;
    appsObject->nextOffset = 0;
    appsObject->hardware = hardwareTemplateDeprecated;
    strncpy(appsObject->magic, MAGICSTRING, MAGICSTRINGLEN);
    strncpy(appsObject->initialVersion, "p2026.0", (sizeof "p2026") - 1);
    strncpy(appsObject->version, CURRENTVERSION, CURRENTVERSIONLEN);
    return true;
}

void handleUpdateTransition(char* current, char* next) {
    // currently nothing to do
    return;
}

void initApps() {
    apps = (firmwareHeader*) malloc(programDataPartition->erase_size);
    esp_err_t err = readPartition(programDataPartition, (uint8_t*) apps, programDataPartition->erase_size, 0);
    if (err != ESP_OK) {
        machineLog("Init app read failed: %s\n", esp_err_to_name(err));
    }

    bool performSync = false;

    // some apps were distributed to hardware before this versioning system and need to be corrected in a way that the regular version system won't
    performSync |= correctDeprecatedApps(apps);
    if (strncmp(apps->magic, MAGICSTRING, sizeof MAGICSTRING) != 0) {
        machineLog("Expected magic bytes (ascii) say: \"%s\", read \"%s\"\n\r", MAGICSTRING, apps->magic);
        machineLog("No firmware header found. Initializing...\n\r");
        strcpy(apps->magic, MAGICSTRING);
        strcpy(apps->initialVersion, CURRENTVERSION);
        strcpy(apps->version, CURRENTVERSION);
        apps->appCount = 0;
        apps->nextOffset = 1;
        apps->hardware = hardwareTemplate;
        performSync = true;
    }
    if (strncmp(apps->version, CURRENTVERSION, sizeof CURRENTVERSION) != 0) {
        handleUpdateTransition(apps->version, CURRENTVERSION);
        strcpy(apps->version, CURRENTVERSION);
        performSync = true;
    }
    if (overwriteHardware) {
        apps->hardware = hardwareTemplate;
        performSync = true;
    }
    if (performSync) syncApps();
}

void clearApps() {
    strcpy(apps->magic, MAGICSTRING);
    apps->appCount = 0;
    apps->nextOffset = 1;
}

void listApps() {
    machineLog("app header with {\n\r\tmagic: %.*s,\n\r\tversion: %.*s,\n\r\tinitialVersion: %.*s,\n\r\tappCount: %hu,\n\r\tnextOffset: %hu\n\r}\n\r", 
    sizeof apps->magic, apps->magic, sizeof apps->version, apps->version, sizeof apps->initialVersion, apps->initialVersion, apps->appCount, apps->nextOffset);
    for (int i = 0; i < apps->appCount; i++) {
        appDescriptor d = apps->apps[i];
        machineLog("\tapp \"%s\" with size %d at offset %d\n\r", d.name, d.size, d.offset);
    }
}

void awaitString(char* string, int len) {
    int index = 0;
    while (index < len) {
        while (Serial.available() < 1);
        int c = readCharInteractive();
        if (c == 0x7f) index--;
        else if (c == string[index]) index++;
        else index = 0;
    }
}

void* saveApp(char* appName, size_t sectorOffset, uint8_t* newData, size_t size, bool alsoMap) {
    writePartition(programDataPartition, newData, size, sectorOffset);
    void* toReturn = NULL;
    if (alsoMap) {
        toReturn = mapPartition(programDataPartition, size, sectorOffset);
    }
    return toReturn;
}

dataHeader readHeader() {
    char magicBytes[] = "scratch!";
    awaitString(magicBytes, (sizeof magicBytes) - 1);
    machineLog("%c\n\r", 6);
    dataHeader h;
    uint8_t* ptr = (uint8_t*)&h.spriteCount;
    while (ptr < (uint8_t*)((&h) + 1)) {
        if (Serial.available() < 1) continue;
        uint8_t b = readChar();
        *ptr++ = b;
    }
    return h;
}

// TODO
void pollInstruction() {
    return;
}

// This is cursed and I had a lot of fun writing it
void* memdiffNext(memdiffGenerator* state) {
    if (state->index >= state->target) {
        if (!state->runGoing) return NULL;
        state->runEnding = state->target;
        state->runGoing = false;
        return &state->runGoing;
    }
    if (state->a[state->index] == state->b[state->index]) {
        if (!state->runGoing) {
            state->index++;
            return &state->index;
        }
        state->runEnding = state->index;
        state->runGoing = false;
        state->index++;
        return &state->runGoing;
    }
    else {
        state->differenceCount++;
        if (state->runGoing) {
            state->index++;
            return &state->index;
        }
        state->runBeginning = state->index;
        state->runGoing = true;
        state->index++;
        return &state->runGoing;
    }
}

void memdiffPrint(memdiffGenerator* state, void* returnHandle) {
    bool printSatisfied = false;
    if (returnHandle == NULL) {
        machineLog("(difference count: %d).\n\r", state->differenceCount);
    }
    if (state->printed >= state->maxPrint) return;
    if (state->maxPrint - state->printed < 20) {
        state->printed = state->maxPrint;
        printSatisfied = true;
    }
    if (returnHandle == &state->runGoing) {
        if (state->runGoing) {
            state->printed += machineLog("%d", state->runBeginning);
        }
        else {
            if (state->runEnding - state->runBeginning == 1) {
                state->printed += machineLog(": [%d, %d]", state->a[state->runBeginning], state->b[state->runBeginning]);
            }
            else {
                state->printed += machineLog("..%d", state->runEnding);
            }
            machineLog(", ");
        }
    }
    if (printSatisfied) {
        machineLog(".....");
    }
}

void memdiffAll(const void* a, const void* b, size_t n, void(*callback)(memdiffGenerator*, void*)) {
    memdiffGenerator g = {0};
    g = (memdiffGenerator) {
        .a=(uint8_t*)a,
        .b=(uint8_t*)b,
        .target=n,
        .maxPrint = 100
    };
    while (true) {
        void* handle = memdiffNext(&g);
        callback(&g, handle);
        if (handle == NULL) break;
    }
}

void memdiffPrintAll(const void* a, const void* b, size_t n) {
    memdiffAll(a, b, n, memdiffPrint);
}

// `Serial.readBytes` is not behaving correctly; it can mistakenly return `size` when it has not read any bytes.
int timeout = 1000;
int readBytes(uint8_t* buf, int size) {
    size_t start = millis();
    while (Serial.available() == 0) {
        if ((millis() - start) > timeout) break;
        delay(1);
    }
    if (Serial.available() == 0) return 0;
    return Serial.readBytes(buf, size);
}

#define assert(condition, message, ...) if (!(condition)) {machineLog(message, __VA_ARGS__); return NULL;}
extern "C" void* pollApp(char* name) {
    dataHeader h = readHeader();

    machineLog("Reading %u bytes of program data...\n\n\r", h.dataSize - (sizeof h));

    size_t sectorSize = programDataPartition->erase_size;
    uint8_t* tempBuffer = (uint8_t*)malloc(sectorSize);
    if (!tempBuffer) {
        machineLog("FATAL: Failed to allocate %u bytes as a buffer for program data\n\r", sectorSize);
        return NULL;
    }

    uint8_t* programDataNoHeader = (uint8_t*) programData + 8;

    int baseOffset = 1;
    int bytesWritten = 0;
    memcpy(tempBuffer, &h, sizeof h);
    int remainder = sectorSize - sizeof h;
    int bytesRead = readBytes(tempBuffer + sizeof h, remainder);
    memdiffPrintAll(tempBuffer, programDataNoHeader, sectorSize);
    assert(bytesRead == remainder, "failed to read %u bytes (read %u)\n", remainder, bytesRead);
    writeSector(programDataPartition, tempBuffer, sectorSize, baseOffset);
    machineLog("%c\n\r", 6);
    bytesWritten += sectorSize;

    while (true) {
        bool breakOut = false;

        int maxRemainder = h.dataSize - bytesWritten;
        remainder = sectorSize;
        if (remainder >= maxRemainder) {
            remainder = maxRemainder;
            breakOut = true;
        }

        bytesRead = readBytes(tempBuffer, remainder);
        assert(bytesRead == remainder, "failed to read %u bytes (read %u)\n", remainder, bytesRead);

        writeSector(programDataPartition, tempBuffer, remainder, baseOffset + (bytesWritten / sectorSize));
        machineLog("%c", 6);
        bytesWritten += remainder;

        if (breakOut) break;
    }
    free(tempBuffer);

    void* toReturn = mapPartition(programDataPartition, h.dataSize, baseOffset);

    machineLog("testing dereference of at least header size\n\r");
    for (int i = 0; i < sizeof h * 2; i++) {
        machineLog("char at %d: %d\n\r", i, ((char*)toReturn)[i]);
    }
    machineLog("success\n\r");
    apps->appCount = 1;
    syncApps();
    return toReturn;
}
#undef assert

extern "C" int selectApp(app_t* out, char* appName) {
    for (int i = 0; i < apps->appCount; i++) {
        appDescriptor* d = &(apps->apps[i]);
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

void loop() {}

void setup() {
    Serial.setRxBufferSize(4096 + 64);
    //Serial.begin(115200);
    Serial.begin(921600);
    partitionInit();
    initApps();
    startIO();
    delay(1000);
    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_PANIC || reason == ESP_RST_INT_WDT) {
        clearApps();
    }
    listApps();
    app_t app;
    bool override = false;
    if (override) {
        app.programData = (uint8_t*)programData + 8;
    }
    else if (apps->appCount < 1) {
        app.programData = (uint8_t*)pollApp("");
    }
    else {
        app.programData = (uint8_t*)loadApp(1, 65536);
    }
    updateIO(&app);
    while (true) {
        runApp(&app);
        closeApp(&app, 0);
        app.programData = (uint8_t*)pollApp("");
    }
    ESP.restart();
}
