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

#define SIZERATIO 1024

typedef uint16_t pixel;

struct image {
    uint16_t widthRatio;
    uint16_t heightRatio;
    uint16_t xResolution;
    uint16_t yResolution;
    int16_t xRotationCenter;
    int16_t yRotationCenter;
    char name[COSTUMENAMESIZE];
    pixel pixels[];
};

#endif
