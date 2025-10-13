#ifndef EXTERNGLOBALS_H
#define EXTERNGLOBALS_H

#include "scratch.h"

extern const struct SCRATCH_header header;
extern const uint8_t programData[];
extern enum SCRATCH_opcode* code;
extern bool events[];
extern int eventCount;
extern int eventTypeOffsets[3];
extern bool inputState[5];

extern struct SCRATCH_sprite* sprites[SPRITEMAX];
extern int spriteCount;

#endif
