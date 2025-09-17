#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "scratch.h"
#include "programData.h"
#include "graphics.h"

void sleepMillis(int millis) {
    struct timespec t;
    t.tv_sec = millis / 1000;
    t.tv_nsec = (millis % 1000) * 1000000;
    nanosleep(&t, NULL);
}


struct SCRATCH_sprite* sprites[SPRITEMAX];
const uint16_t* imageTable[IMAGEMAX];

uint16_t screen[LCDWIDTH][LCDHEIGHT];

int main() {
    initData(header, programData, sprites, imageTable);
    while (SCRATCH_visitAllThreads(sprites, header.spriteCount)) {
        drawSprites(sprites, 2, imageTable, screen);
    }
    machineLog(
            "size: %d, %d\nthreadCounts: %d, %d\nx: %d, %d\n",
            sprites[0]->base.size, sprites[1]->base.size, sprites[0]->base.threadCount, sprites[1]->base.threadCount, sprites[0]->base.x.i, sprites[1]->base.x.i
    );
    drawSprites(sprites, 2, imageTable, screen);
}
