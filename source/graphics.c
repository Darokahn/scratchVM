#include <stdio.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include "scratch.h"
#include "graphics.h"
#include "programData.h"
#include "letters.h"
#include "externalDefinitions.h"

void drawSprites(struct SCRATCH_sprite** sprites, int spriteCount, const uint16_t** imageTable, uint16_t* screen) {
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
                int row = (y * yStride);
                int index = ((row * imageResolution) + (x * xStride));
                uint16_t color = image[index];
                // transparent pixels reveal white if they are on the background; do nothing if they are on a sprite.
                if (color == 0) {
                    if (i == 0) color = 0xffff;
                    else continue;
                }
                screen[(y + baseY ) * LCDWIDTH + x + baseX] = color;
            }
        }
    }
}

void debugImage(uint16_t *img, int width, int height) {
    uint8_t pixels[(width * height * 21) + (5 * 128) + 1];
    char* pixelPointer = pixels;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t pixel = img[y * width + x];

            uint8_t r = (pixel >> 11) & 31;
            uint8_t g = (pixel >> 5) & 63;
            uint8_t b = pixel & 31;

            uint8_t R = (r * 255) / 31;
            uint8_t G = (g * 255) / 63;
            uint8_t B = (b * 255) / 31;

            pixelPointer += sprintf(pixelPointer, "\x1b[48;2;%u;%u;%um  ", R, G, B);
        }
        // Reset and newline
        pixelPointer += sprintf(pixelPointer, "\x1b[0m\n");
    }
    puts(pixels);
}
