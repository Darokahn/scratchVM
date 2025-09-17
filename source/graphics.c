#include <stdio.h>
#include <stdint.h>
#include "scratch.h"
#include "graphics.h"
#include "programData.h"

void drawSprites(struct SCRATCH_sprite** sprites, int spriteCount, const uint16_t** imageTable, uint16_t screen[LCDWIDTH][LCDHEIGHT]) {
    for (int i = 0; i < spriteCount; i++) {
        struct SCRATCH_sprite* sprite = sprites[i];
        const uint16_t* image = getImage(imageTable, i, sprite->base.costumeIndex);
        int imageResolution;
        if (i == 0) imageResolution = 128;
        else imageResolution = 32;
        int baseX = sprite->base.x.halves.high * WIDTHRATIO;
        int baseY = sprite->base.y.halves.high * HEIGHTRATIO;
        int width = ((float)sprite->base.widthRatio / 255) * LCDWIDTH;
        int height = ((float)sprite->base.heightRatio / 255) * LCDHEIGHT;
        float xStride = ((float)imageResolution) / width;
        float yStride = ((float)imageResolution) / height;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (y + baseY >= LCDHEIGHT || x + baseX >= LCDWIDTH || y + baseY < 0 || x + baseX < 0) continue;
                uint16_t color = image[(int) ((y * imageResolution) * yStride + x * xStride)];
                // transparent pixels reveal white if they are on the background; do nothing if they are on a sprite.
                if (color == 0) {
                    if (i == 0) color = 0xffff;
                    else continue;
                }
                screen[y + baseY][x + baseX] = color;
            }
        }
    }
    debugImage((uint16_t*) screen, LCDWIDTH, LCDHEIGHT);
}

void debugImage(uint16_t *img, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t pixel = img[y * width + x];

            uint8_t r = (pixel >> 11) & 31;
            uint8_t g = (pixel >> 5) & 63;
            uint8_t b = pixel & 31;

            uint8_t R = (r * 255) / 31;
            uint8_t G = (g * 255) / 63;
            uint8_t B = (b * 255) / 31;

            printf("\x1b[48;2;%u;%u;%um  ", R, G, B);
        }
        // Reset and newline
        printf("\x1b[0m\n");
    }
}
