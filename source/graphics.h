#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "scratch.h"

#define LCDWIDTH 405//320
#define LCDHEIGHT 305//240

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
