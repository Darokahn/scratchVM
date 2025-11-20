#include <stdint.h>
#include <string.h>
#include "graphics.h"
#include "scratch.h"
#include "externFunctions.h"
#include "externGlobals.h"
#include "letters.h"
#include "drawing.h"

void drawSprites(struct SCRATCH_spriteContext* context) {
    for (int i = 0; i < context->spriteCount; i++) {
        struct SCRATCH_sprite* sprite = context->sprites[i];
        if (!sprite->base.visible) continue;
        struct image* image = getImage(context, sprite);
        int imageResolution;
        int baseX;
        int baseY;
        int width;
        int height;
        imageResolution = image->xResolution;
        if (i == 0) {
            baseX = 0;
            baseY = 0;
            width = LCDWIDTH;
            height = LCDHEIGHT;
        }
        else {
            baseX = (sprite->base.x.halves.high + (SCRATCHWIDTH / 2)) * WIDTHRATIO;
            baseY = (-sprite->base.y.halves.high + (SCRATCHHEIGHT / 2)) * HEIGHTRATIO;
            width = (((float)image->widthRatio / 255) * LCDWIDTH) * sprite->base.size / 100;
            height = (((float)image->heightRatio/ 255) * LCDHEIGHT) * sprite->base.size / 100;
            baseX -= (width / 2);
            baseY -= (height / 2);
        }
        // convert from inner scratch-centric coordinates to real screen coordinates
        float xStride = ((float)imageResolution) / width;
        float yStride = ((float)imageResolution) / height;
        int scanX;
        int scanStep;
        int scanStart;
        if (sprite->base.rotation < halfRotation) {
            scanStart = 0;
            scanStep = 1;
        }
        else {
            scanStart = width;
            scanStep = -1;
        }
        int x;
        int y;
        if (sprite->talkingString != NULL) {
            drawString(sprite->talkingString, (struct SCRATCH_rect) {.x=baseX + width, .y=baseY, .width=100, .height=50}, basicFont, 0);
        }
        for (y = 0; y < height; y++) {
            scanX = scanStart;
            for ((x = 0, scanX = scanStart); x < width; (x++, scanX += scanStep)) {
                if (y + baseY >= LCDHEIGHT || x + baseX >= LCDWIDTH || y + baseY < 0 || x + baseX < 0) continue;
                int row = (y * yStride);
                int index = ((row * imageResolution) + (scanX * xStride));
                if (index >= image->width * image->height) continue;
                pixel color = image->pixels[index];

                // transparent pixels reveal white if they are on the background; do nothing if they are on a sprite.
                if (color == 0) {
                    if (i == 0) color = (pixel) 0xffff;
                    else continue;
                }
                drawPixel(x + baseX, y + baseY, color);
            }
        }
    }
}

bool getLetterPixel(struct letterSet letterSet, int letter, int row, int col) {
    letter -= 'A';
    if (letter >= letterSet.letterMax || letter < 0) return 0;
    return letterSet.letters[letter].rows[row].cols[col] - '0';
}

void drawString(char* string, struct SCRATCH_rect rect, const struct letterSet letterSet, pixel color) {
    int charWidth = 3;
    int charHeight = 5;
    int stringLength = strlen(string);
    for (int i = 0; i < stringLength; i++) {
        char c = string[i];
        int baseX = rect.x + (charWidth + 1) * i;
        int baseY = rect.y;
        for (int y = 0; y < charHeight + 1; y++) {
            for (int x = 0; x < charWidth + 1; x++) {
                bool draw = getLetterPixel(letterSet, c, y, x);
                pixel thisColor = color;
                if (!draw || x == charWidth || y == charHeight) thisColor = 0xffff;
                drawPixel(x + baseX, y + baseY, thisColor);
            }
        }
    }
}
