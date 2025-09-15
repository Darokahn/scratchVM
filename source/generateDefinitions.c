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

uint8_t pattern128[SIZE128 * SIZE128];
uint8_t pattern32[SIZE32 * SIZE32];

enum phases {
    HIGH,
    RISING,
    LOW,
    FALLING
};

void generatePatterns() {
    // Generate 128x128 pattern (1 pixel per square)
    uint8_t colors[4] = {0};
    unsigned int rPhase = HIGH;
    unsigned int gPhase = RISING;
    unsigned int bPhase = LOW;
    colors[HIGH] = 255;
    colors[LOW] = 0;
    for (int phase = 0; phase < 4; phase++) {
        colors[RISING] = 0;
        colors[FALLING] = 255;
        for (int i = 0; i < 32; i++) {
            int red = (colors[rPhase]);
            int green = (colors[gPhase]);
            int blue = (colors[bPhase]);
            int x = (phase * 32) + i;
            for (int y = 0; y < 128; y++) {
                int redTemp = red * 7 / 256;
                int greenTemp = green * 7 / 256;
                int blueTemp = blue * 3 / 256;
                int color = (redTemp << 5) | (greenTemp << 3) | blueTemp;
                pattern128[y * 128 + x] = color;
            }
            colors[RISING] += 4;
            colors[FALLING] -= 4;
        }
        rPhase = (rPhase - 1) % 4;
        gPhase = (gPhase - 1) % 4;
        bPhase = (bPhase - 1) % 4;
    }

    // Generate 32x32 pattern (1 pixel per square)
    for (int x = 0; x < 32; x++) {
        int alt = sqrt(-(x - 16) * (x - 16) + 256);
        for (int y = 0; y < 16; y++) {
            if (y <= alt) {
                pattern32[(16 + y) * 32 + x] = 0xff;
                pattern32[(16 - y) * 32 + x] = 0xff;
            }
            else {
                pattern32[(16 + y) * 32 + x] = 0x00;
                pattern32[(16 - y) * 32 + x] = 0x00;
            }
        }
    }
}

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

    struct SCRATCH_threadHeader threadTemplate = {
        .entryPoint = 0
    };

    generatePatterns();
    const enum SCRATCH_opcode codeTemplate[] = {
        SCRATCH_DEBUGSTATEMENT,
        SCRATCH_push, SCRATCH_NUMBER, 30, 0,
        SCRATCH_push, SCRATCH_NUMBER, 30, 0,
        SCRATCH_push, SCRATCH_NUMBER, 3, 0,
        SCRATCH_motionGlideto,
        SCRATCH_motion_glideIteration,
        SCRATCH_DEBUGSTATEMENT,
        SCRATCH_stop,

        SCRATCH_DEBUGSTATEMENT,
        SCRATCH_stop,
    };

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
    memcpy(data, &threadTemplate, sizeof threadTemplate);
    data += sizeof threadTemplate;
    data = ALIGN8(data);
    memcpy(data, &spriteTemplate, sizeof spriteTemplate);
    data += sizeof spriteTemplate;
    data = ALIGN8(data);
    memcpy(data, &threadTemplate, sizeof threadTemplate);
    data += sizeof threadTemplate;
    data = ALIGN8(data);
    header.spriteCount = 2;
    header.codeLength = (int) ALIGN8(sizeof codeTemplate);
    header.imageLength = sizeof pattern128 + sizeof pattern32;
}

void writeMock() {
    printf("%s", "#include <string.h>\n#include <stdlib.h>\n#include \"programData.h\"\n#include \"scratch.h\"\nenum SCRATCH_opcode* code;\n");
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
