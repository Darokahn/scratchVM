#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "scratch.h"
#include "programData.h"
#include "graphics.h"
#include "externalDefinitions.h"

struct SCRATCH_sprite* sprites[SPRITEMAX];
const pixel* imageTable[IMAGEMAX];

int main() {
    initData(header, programData, sprites, imageTable);
    startGraphics();
    while (true) {
        SCRATCH_visitAllThreads(sprites, header.spriteCount);
        drawSprites(sprites, 2, imageTable);
        updateGraphics();
        puts("=============");
        debugImage(getImage(imageTable, 0, 1), 128, 128);
    }
}
