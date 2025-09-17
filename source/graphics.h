#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "scratch.h"

#define LCDWIDTH 320
#define LCDHEIGHT 240

#define SCRATCHWIDTH 480
#define SCRATCHHEIGHT 360

#define WIDTHRATIO (((float) LCDWIDTH) / SCRATCHWIDTH)
#define HEIGHTRATIO (((float) LCDHEIGHT) / SCRATCHHEIGHT)



void drawSprites(struct SCRATCH_sprite** sprites, int spriteCount, const uint16_t** imageTable, uint16_t screen[LCDHEIGHT][LCDWIDTH]);
void debugImage(uint16_t *img, int width, int height);

#endif
