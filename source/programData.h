#ifndef PROGRAMDATA_H
#define PROGRAMDATA_H

#include <stdint.h>
#include "scratch.h"
#include "graphics.h"

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

bool getEvent(enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input);
void setEvent(enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input, bool state);

struct image* getImage(const pixel* images[IMAGEMAX], int spriteIndex, int costumeIndex);

void initData(const struct SCRATCH_header header, const uint8_t* buffer, struct SCRATCH_sprite* sprites[SPRITEMAX], const pixel* images[IMAGEMAX]);
#endif
