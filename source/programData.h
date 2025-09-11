#ifndef PROGRAMDATA_H
#define PROGRAMDATA_H

#include <stdint.h>
#include "scratch.h"

// Note that per scratch terminology, a "sprite" is a game object rather than the more conventional sense of being a game object's associated image.

// Program data consists of:
// - Header:
// -- Code length
// -- Image data length
// -- Sprite count
// - Data buffer:
// -- Code
// -- Images
// -- Sprites
//
// Each sprite is variable length, but needs no entry in the header. 
// Each sprite has a fixed-length descriptor which can be used to determine how much additional memory it occupies.

// The program data is used to initialize two data structures:
// - An image map that can be given a sprite index and a costume index, returning a pointer into the image section of the data buffer
// - An unordered sprite array

extern struct SCRATCH_sprite* sprites[SPRITEMAX];
extern int spriteCount;
extern enum SCRATCH_opcode* code;
extern uint8_t* imageTable[IMAGEMAX];

struct SCRATCH_header {
    uint32_t codeLength;
    uint32_t imageLength;
    uint32_t spriteCount;
};

struct SCRATCH_spriteHeader {
    bool visible;
    int8_t layer;
    scaledInt32 x;
    scaledInt32 y;
    uint8_t size;
    uint16_t rotation; // Rotation maps (0 -> 360) to the entire range of a 16-bit integer
    bool rotationStyle;
    uint8_t costumeIndex;
    uint8_t costumeMax;
    uint8_t threadCount;
    uint8_t variableCount;
};

struct SCRATCH_threadHeader {
    enum SCRATCH_EVENTTYPE startEvent;
    union SCRATCH_eventInput eventCondition;
    uint16_t programCounter;
};

uint8_t* getImage(uint8_t* images[IMAGEMAX], int spriteIndex, int costumeIndex);

void initData(struct SCRATCH_header header, uint8_t* buffer, struct SCRATCH_sprite* sprites[SPRITEMAX], uint8_t* images[IMAGEMAX]);
#endif
