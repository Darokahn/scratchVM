#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "scratch.h"
#include "programData.h"
#include "graphics.h"
#include "externalDefinitions.h"

unsigned long getNow();

struct SCRATCH_sprite* sprites[SPRITEMAX];
const pixel* imageTable[IMAGEMAX];

int count;
int drawRate = 2;

unsigned long interval = 1000 / FRAMESPERSEC;

int main() {
    unsigned long next = getNow() + interval;
    initData(header, programData, sprites, imageTable);
    startGraphics();
    while (true) {
        SCRATCH_visitAllThreads(sprites, header.spriteCount);
        if (count++ % drawRate == 0) {
            drawSprites(sprites, header.spriteCount, imageTable);
            updateGraphics();
        }
        while (getNow() < next) {
        }
        next += interval;
    }
}
