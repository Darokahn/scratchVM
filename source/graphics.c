#include <stdio.h>
#include <stdint.h>
#include "scratch.h"
#include "graphics.h"
#include "programData.h"

void drawSprites(struct SCRATCH_sprite** sprites, int spriteCount, const uint8_t** imageTable, uint8_t screen[LCDWIDTH][LCDHEIGHT]) {
    for (int i = 0; i < spriteCount; i++) {
        struct SCRATCH_sprite* sprite = sprites[i];
        const uint8_t* image = getImage(imageTable, i, sprite->base.costumeIndex);
        int imageResolution;
        if (i == 0) imageResolution = 128;
        else imageResolution = 32;
        int baseX = sprite->base.x.halves.high * WIDTHRATIO;
        int baseY = sprite->base.y.halves.high * HEIGHTRATIO;
        int width = ((float)sprite->base.widthRatio / 255) * LCDWIDTH;
        int height = ((float)sprite->base.heightRatio / 255) * LCDHEIGHT;
        float xStride = ((float)imageResolution) / width;
        float yStride = ((float)imageResolution) / height;
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                uint8_t color = image[(int) ((y * imageResolution) * yStride + x * xStride)];
                if (color == 0) {
                    if (i == 0) color = 255;
                    else continue;
                }
                screen[y + baseY][x + baseX] = color;
            }
        }
    }
    debugImage((uint8_t*) screen, LCDWIDTH, LCDHEIGHT);
}

void debugImage(uint8_t *img, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t pixel = img[y * width + x];

            uint8_t r = (pixel >> 5) & 0x07;
            uint8_t g = (pixel >> 2) & 0x07;
            uint8_t b = pixel & 0x03;

            uint8_t R = (r * 255) / 7;
            uint8_t G = (g * 255) / 7;
            uint8_t B = (b * 255) / 3;

            printf("\x1b[48;2;%u;%u;%um  ", R, G, B);
        }
        // Reset and newline
        printf("\x1b[0m\n");
    }
}
