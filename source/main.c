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
    machineLog("test\n");
    while (SCRATCH_visitAllThreads(sprites, header.spriteCount)) {
        //drawSprites(sprites, 2, imageTable);
        //updateGraphics();
    }
    drawSprites(sprites, 2, imageTable);
    updateGraphics();
}
