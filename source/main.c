#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "scratch.h"
#include "programData.h"
#include "graphics.h"
#include "externFunctions.h"
#include "externGlobals.h"

unsigned long getNow();

const pixel* imageTable[IMAGEMAX];
struct SCRATCH_sprite* sprites[SPRITEMAX];
int spriteCount;

int count = 0;
int drawRate = 2;

unsigned long interval = 1000 / FRAMESPERSEC;

extern const enum SCRATCH_opcode insertedCode[];

int main() {
    unsigned long next = getNow() + interval;
    initData(header, programData, sprites, imageTable);
    code = insertedCode;
    startGraphics();
    drawSprites(sprites, header.spriteCount, imageTable);
    updateGraphics();
    setEvent(ONFLAG, (union SCRATCH_eventInput) {0}, true);
    while (true) {
        do handleInputs(); while (getNow() < next);
        next += interval;
        SCRATCH_visitAllThreads(sprites, header.spriteCount);
        if (count++ % drawRate == 0) {
            drawSprites(sprites, 2, imageTable);
            updateGraphics();
        }
        SCRATCH_wakeSprites();
        clearEvents();
    }
}
