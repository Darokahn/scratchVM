#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include "scratch.h"
#include "letters.h"

#define FULLLCDWIDTH 320
#define FULLLCDHEIGHT 240

#define LCDWIDTH 265
#define LCDHEIGHT 208

#define SCRATCHWIDTH 480
#define SCRATCHHEIGHT 360

#define WIDTHRATIO (((float) LCDWIDTH) / SCRATCHWIDTH)
#define HEIGHTRATIO (((float) LCDHEIGHT) / SCRATCHHEIGHT)

#define COSTUMENAMESIZE 256

typedef uint16_t pixel;

struct image {
    uint8_t widthRatio;
    uint8_t heightRatio;
    uint8_t xResolution;
    uint8_t yResolution;
    char name[COSTUMENAMESIZE];
    pixel pixels[];
};

#endif
