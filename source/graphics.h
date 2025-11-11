#ifndef GRAPHICS_H
#define GRAPHICS_H

#define FULLLCDWIDTH 320
#define FULLLCDHEIGHT 240

#define LCDWIDTH FULLLCDWIDTH
#define LCDHEIGHT FULLLCDHEIGHT

#define SCRATCHWIDTH 480
#define SCRATCHHEIGHT 360

#define WIDTHRATIO (((float) LCDWIDTH) / SCRATCHWIDTH)
#define HEIGHTRATIO (((float) LCDHEIGHT) / SCRATCHHEIGHT)

#define COSTUMENAMESIZE 256

typedef uint16_t pixel;

struct image {
    uint8_t widthRatio;
    uint8_t heightRatio;
    char name[COSTUMENAMESIZE];
    pixel pixels[];
};
#endif
