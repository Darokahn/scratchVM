#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "scratch.h"

#define ALIGN8(ptr) ((void*) (((uint64_t) ptr + 7) & ~7))

struct SCRATCH_header header;
uint8_t programData[4096 * 10];
enum SCRATCH_opcode* code;

#define SIZE 128

#define SIZE128 128
#define SIZE32  32

uint16_t pattern128[SIZE128 * SIZE128];
uint16_t pattern32[SIZE32 * SIZE32];

uint8_t letters[] = {
    "! !! !!!!! !! !!!!!  !!!!  !!!!  !  !  !  !!!!!!! !! !! !!!!! !! !!!!!!!! !!!!! !!! ! !! !!! ! !! !! !!! "
};

uint8_t getLetterValue(int letter, int row, int col) {
    int index = (letter * 15) + (row * 3) + col;
    return letters[index] - ' ';
}

uint8_t getGradientPeriod(int index) {
    index %= 1536;
    if (index < 0) return 0;
    // low phase
    if (index < 512) {
        return 0;
    }
    index -= 512;
    // rising phase
    if (index < 256) {
        return index;
    }
    index -= 256;
    // high phase
    if (index < 512) {
        return 255;
    }
    // falling phase
    index -= 512;
    if (index < 256) {
        return 255 - index;
    }
    else return 0;
}

uint16_t getGradientAt(int x) {
    int redOffset = 1024;
    int greenOffset = 512;
    int blueOffset = 0;
    int red = getGradientPeriod((redOffset + x));
    int green = getGradientPeriod((greenOffset + x));
    int blue = getGradientPeriod((blueOffset + x));
    int tempRed = red;
    int tempGreen = green;
    int tempBlue = blue;
    tempRed = (tempRed * 31) / 255;
    tempGreen = (tempGreen * 63) / 255;
    tempBlue = (tempBlue * 31) / 255;
    uint16_t color = tempRed << 11 | tempGreen << 5 | tempBlue;
    return color;
}

void generatePatterns() {
    // Generate 128x128 pattern (1 pixel per square)
    int redOffset = 1024;
    int greenOffset = 512;
    int blueOffset = 0;
    for (int x = 0; x < 128; x++) {
        int index = x * (1536 / 128);
        int red = getGradientPeriod((redOffset + index));
        int green = getGradientPeriod((greenOffset + index));
        int blue = getGradientPeriod((blueOffset + index));
        for (int y = 0; y < 128; y++) {
            int tempRed = red;
            int tempGreen = green;
            int tempBlue = blue;
            /*
            tempRed = (tempRed * y) / 128;
            tempGreen = (tempGreen * y) / 128;
            tempBlue = (tempBlue * y) / 128;
            */
            tempRed = (tempRed * 31) / 255;
            tempGreen = (tempGreen * 63) / 255;
            tempBlue = (tempBlue * 31) / 255;
            uint16_t color = tempRed << 11 | tempGreen << 5 | tempBlue;
            if (color == 0) color = 1;
            pattern128[y * 128 + x] = color;
        }
    }

    // Generate 32x32 pattern (1 pixel per square)
    for (int x = 0; x < 32; x++) {
        int alt = sqrt(-(x - 16) * (x - 16) + 256);
        for (int y = 0; y < 16; y++) {
            if (y <= alt) {
                pattern32[(16 + y) * 32 + x] = 28;
                pattern32[(16 - y) * 32 + x] = 28;
            }
            else {
                pattern32[(16 + y) * 32 + x] = 0x00;
                pattern32[(16 - y) * 32 + x] = 0x00;
            }
        }
    }
}

void drawLetters() {
    int letterWidth = 8; // (row width + space) * 2
    int letterCount = 11; // "hello world"
    int phraseWidth = letterWidth * letterCount;
    int letterHeight = 10; // column width * 2
    int xCursor = (128 - phraseWidth) / 2;
    int yCursor = (128 - letterHeight) / 2;
    uint8_t message[] = {0, 1, 2, 2, 3, 255, 4, 3, 5, 2, 6};
    for (int i = 0; i < 11; (i++, xCursor += 8)) {
        uint8_t letter = message[i];
        if (letter == 255) continue;
        for (int x = 0; x < 6; x++) {
            for (int y = 0; y < 10; y++) {
                if (!getLetterValue(letter, y / 2, x / 2)) continue;
                pattern128[(y + yCursor) * 128 + (x + xCursor)] = getGradientAt((phraseWidth - (x + xCursor)) / 2);
            }
        }
    }
}

const enum SCRATCH_opcode codeTemplate[] = {
    SCRATCH_DEBUGSTATEMENT,
    SCRATCH_push, SCRATCH_NUMBER, 30, 0,
    SCRATCH_push, SCRATCH_NUMBER, 255, 0,
    SCRATCH_push, SCRATCH_NUMBER, 255, 10,
    SCRATCH_motionGlideto,
    SCRATCH_motion_glideIteration,
    SCRATCH_DEBUGSTATEMENT,
    SCRATCH_stop,

    SCRATCH_DEBUGSTATEMENT,
    SCRATCH_stop,
};

void mockProgram() {
    struct SCRATCH_spriteHeader stageTemplate = {
        .x = {0},
        .y = {0},
        .rotation = 0,
        .visible = true,
        .layer = 0,
        .size = 128,
        .widthRatio = 255,
        .heightRatio = 255,
        .rotationStyle = 0,
        .costumeIndex = 0,
        .costumeMax = 1,
        .threadCount = 1,
        .variableCount = 1
    };

    struct SCRATCH_spriteHeader spriteTemplate = {
        .x = {.halves.high=0},
        .y = {0},
        .rotation = 0,
        .visible = true,
        .layer = 0,
        .size = 128,
        .widthRatio = 64,
        .heightRatio = 64,
        .rotationStyle = 0,
        .costumeIndex = 0,
        .costumeMax = 1,
        .threadCount = 1,
        .variableCount = 1
    };

    struct SCRATCH_threadHeader threadTemplate1 = {
        .entryPoint = (sizeof codeTemplate) - 2
    };
    struct SCRATCH_threadHeader threadTemplate2 = {
        .entryPoint = 0
    };

    generatePatterns();
    drawLetters();
    uint8_t* data = (uint8_t*) programData;
    memcpy(data, codeTemplate, sizeof codeTemplate);
    data += sizeof codeTemplate;
    data = ALIGN8(data);
    memcpy(data, pattern128, sizeof pattern128);
    data += sizeof pattern128; // no need for alignment in the image buffer
    memcpy(data, pattern32, sizeof pattern32);
    data += sizeof pattern32;
    data = ALIGN8(data);
    memcpy(data, &stageTemplate, sizeof stageTemplate);
    data += sizeof stageTemplate;
    data = ALIGN8(data);
    memcpy(data, &threadTemplate1, sizeof threadTemplate1);
    data += sizeof threadTemplate1;
    data = ALIGN8(data);
    memcpy(data, &spriteTemplate, sizeof spriteTemplate);
    data += sizeof spriteTemplate;
    data = ALIGN8(data);
    memcpy(data, &threadTemplate2, sizeof threadTemplate2);
    data += sizeof threadTemplate2;
    data = ALIGN8(data);
    header.spriteCount = 2;
    header.codeLength = (int)(uintptr_t)ALIGN8(sizeof codeTemplate);
    header.imageLength = sizeof pattern128 + sizeof pattern32;
}

void writeMock() {
    printf("%s", "// THIS IS A GENERATED FILE! Check generateDefinitions.c to see how it was created.\n#include <string.h>\n#include <stdlib.h>\n#include \"programData.h\"\n#include \"scratch.h\"\nenum SCRATCH_opcode* code;\n");
    printf("const struct SCRATCH_header header = {.spriteCount = %d, .codeLength = %d, .imageLength = %d};\n", header.spriteCount, header.codeLength, header.imageLength);
    printf("const uint8_t programData[] = {");
    for (int i = 0; i < sizeof programData; i++) {
        printf("0x%x, ", programData[i]);
    }
    printf("%s", "\n};");
}

int main() {
    mockProgram();
    writeMock();
}
