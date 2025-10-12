#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "scratch.h"

#define LCDWIDTH 265 // The area of the screen utilized for graphics (using more than this crashes due to the processor having no contiguous allocation of the correct size)
#define LCDHEIGHT 208

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
