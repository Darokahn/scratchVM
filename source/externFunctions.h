#ifndef EXTERNALDEFINITIONS_H
#define EXTERNALDEFINITIONS_H

#include <stdint.h>
#include <stdio.h>

#include "graphics.h"
#include "scratch.h"
#include "programData.h"
#include "externGlobals.h"

void startGraphics();
void updateGraphics();
int machineLog(const char* fmt, ...);

void* mallocDMA(size_t size);

void drawSprites(struct SCRATCH_sprite** sprites, int spriteCount, const pixel** imageTable);
void debugImage(pixel *img, int width, int height);

bool getInput(int inputIndex);

#endif
