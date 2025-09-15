#include <stdint.h>
#include <stdlib.h>
#include "scratch.h"
#include "programData.h"
#include "graphics.h"

struct SCRATCH_sprite* sprites[SPRITEMAX];
const uint8_t* imageTable[IMAGEMAX];

uint8_t screen[LCDWIDTH][LCDHEIGHT];

int main() {
    initData(header, programData, sprites, imageTable);
    //while (SCRATCH_visitAllThreads(sprites, header.spriteCount));
    machineLog(
            "size: %d, %d\nthreadCounts: %d, %d\nx: %d, %d\n",
            sprites[0]->base.size, sprites[1]->base.size, sprites[0]->base.threadCount, sprites[1]->base.threadCount, sprites[0]->base.x.i, sprites[1]->base.x.i
    );
    drawSprites(sprites, 2, imageTable, screen);
}
