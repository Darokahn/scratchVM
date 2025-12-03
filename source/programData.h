#pragma once
#include "scratch.h"
#include "ioFuncs.h"
#include "graphics.h"
#include "opcodeEnum.h"

struct dataHeader {
    uint16_t spriteCount;
    uint16_t codeLength;
    uint16_t inputCount;
    uint16_t broadcastCount;
    uint16_t backdropCount;
    uint16_t codeOffset;
    uint32_t spriteOffset;
    uint32_t threadOffset;
    uint32_t imageOffset;
    uint32_t dataSize;
};

void setEvent(enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input, bool state);
bool getEvent(enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input);
struct image* getImage(struct SCRATCH_spriteContext* context, struct SCRATCH_sprite* operand);
void initImages(struct SCRATCH_spriteContext* context, const uint8_t* buffer);
void debugSprite(struct SCRATCH_sprite* sprite);
void initProgram(const uint8_t* buffer, struct SCRATCH_spriteContext* context, const uint8_t** code, int* eventCount, int eventTypeOffsets[]);
