#ifndef EXTERNALDEFINITIONS_H
#define EXTERNALDEFINITIONS_H

#include <stdint.h>
#include <stdio.h>

#include "graphics.h"
#include "scratch.h"
#include "externGlobals.h"

void startIO();
void updateIO();
int machineLog(const char* fmt, ...);

void* mallocDMA(size_t size);

void drawSprites(struct SCRATCH_spriteContext* context);
void debugImage(struct image *img, int width, int height);

bool getInput(int inputIndex);

#endif
