#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "scratch.h"
#include "programData.h"
#include "graphics.h"
#include "graphicsOutput.h"

struct SCRATCH_sprite* sprites[SPRITEMAX];
const uint16_t* imageTable[IMAGEMAX];

uint16_t screen[LCDHEIGHT][LCDWIDTH];

int main() {
    initData(header, programData, sprites, imageTable);
    startGraphics();
    while (SCRATCH_visitAllThreads(sprites, header.spriteCount)) {
        drawSprites(sprites, 2, imageTable, screen);
        updateGraphics((uint16_t*) screen);
    }
    drawSprites(sprites, 2, imageTable, screen);
}
