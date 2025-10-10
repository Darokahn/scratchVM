#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "scratch.h"

#define LCDWIDTH 260//320
#define LCDHEIGHT 200//240

#define FULLLCDWIDTH 320
#define FULLLCDHEIGHT 240

#define SCRATCHWIDTH 480
#define SCRATCHHEIGHT 360

#define WIDTHRATIO (((float) LCDWIDTH) / SCRATCHWIDTH)
#define HEIGHTRATIO (((float) LCDHEIGHT) / SCRATCHHEIGHT)

typedef uint16_t pixel;

struct image {
    uint8_t widthRatio;
    uint8_t heightRatio;
    pixel pixels[];
};
#endif
