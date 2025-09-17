#ifndef PROGRAMDATA_H
#define PROGRAMDATA_H

#include <stdint.h>
#include "scratch.h"

#define ALIGN8(ptr) ((void*) (((uint64_t) ptr + 7) & ~7))


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

extern const struct SCRATCH_header header;
extern const uint8_t programData[];
extern enum SCRATCH_opcode* code;

const uint16_t* getImage(const uint16_t* images[IMAGEMAX], int spriteIndex, int costumeIndex);

void initData(const struct SCRATCH_header header, const uint8_t* buffer, struct SCRATCH_sprite* sprites[SPRITEMAX], const uint16_t* images[IMAGEMAX]);
#endif
