#ifndef EXTERNGLOBALS_H
#define EXTERNGLOBALS_H

#include "scratch.h"

extern bool events[];
extern int eventCount;
extern int eventTypeOffsets[__EVENTTYPECOUNT];
extern bool inputState[];
extern int cursorX;
extern int cursorY;
extern const uint8_t programData[];

#endif
