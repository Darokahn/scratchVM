#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "scratch.h"

#define LCDWIDTH 200
#define LCDHEIGHT 200

#define SCRATCHWIDTH 480
#define SCRATCHHEIGHT 360

#define WIDTHRATIO (((float) LCDWIDTH) / SCRATCHWIDTH)
#define HEIGHTRATIO (((float) LCDHEIGHT) / SCRATCHHEIGHT)

typedef uint8_t pixel;
#endif
