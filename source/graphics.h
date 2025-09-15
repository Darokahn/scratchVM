#ifndef GRAPHICS_H
#define GRAPHICS_H

#define LCDWIDTH 128 // 220
#define LCDHEIGHT 128 // 176

#define SCRATCHWIDTH 480
#define SCRATCHHEIGHT 360

#define WIDTHRATIO (((float) LCDWIDTH) / SCRATCHWIDTH)
#define HEIGHTRATIO (((float) LCDHEIGHT) / SCRATCHHEIGHT)



void drawSprites(struct SCRATCH_sprite** sprites, int spriteCount, const uint8_t** imageTable, uint8_t screen[LCDWIDTH][LCDHEIGHT]);
void debugImage(uint8_t *img, int width, int height);

#endif
