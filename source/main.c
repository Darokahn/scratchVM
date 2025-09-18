#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "scratch.h"
#include "programData.h"
#include "graphics.h"
#include "externalDefinitions.h"

struct SCRATCH_sprite* sprites[SPRITEMAX];
const uint16_t* imageTable[IMAGEMAX];

uint16_t* screen;

int main() {
    initData(header, programData, sprites, imageTable);
    screen = mallocDMA(LCDWIDTH*LCDHEIGHT*sizeof(uint16_t));
    startGraphics();
    machineLog("test\n");
    while (SCRATCH_visitAllThreads(sprites, header.spriteCount)) {
        drawSprites(sprites, 2, imageTable, screen);
        updateGraphics(screen);
    }
}
