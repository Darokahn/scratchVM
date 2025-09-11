#include <stdint.h>
#include <stdlib.h>
#include "scratch.h"
#include "programData.h"

//uint8_t* mockProgram();
//void writeMock();

struct SCRATCH_sprite* sprites[SPRITEMAX];
uint8_t* imageTable[IMAGEMAX];

int main() {
    initData(header, programData, sprites, imageTable);
    machineLog("Size: %d, %d\nthreadCounts: %d, %d\n", sprites[0]->base.size, sprites[1]->base.size, sprites[0]->base.threadCount, sprites[1]->base.threadCount);
}
