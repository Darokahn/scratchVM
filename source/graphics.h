#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "scratch.h"

#define LCDWIDTH 240
#define LCDHEIGHT 180

#define SCRATCHWIDTH 480
#define SCRATCHHEIGHT 360

#define WIDTHRATIO (((float) LCDWIDTH) / SCRATCHWIDTH)
#define HEIGHTRATIO (((float) LCDHEIGHT) / SCRATCHHEIGHT)

typedef uint16_t pixel;
#endif
