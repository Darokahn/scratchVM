#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

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

struct SCRATCH_rect;

void drawPixel(int x, int y, pixel color);

struct letterRow {
    char cols[3];
};

struct letter {
    struct letterRow rows[5];
};

struct letterSet {
    const struct letter* letters;
    int letterMax;
};

const extern struct letter* const letters;
const extern struct letterSet basicFont;

void drawString(char* string, struct SCRATCH_rect rect, const struct letterSet font, pixel color);
