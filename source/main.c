#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "scratch.h"
#include "programData.h"
#include "graphics.h"
#include "externFunctions.h"
#include "externGlobals.h"

unsigned long getNow();

struct SCRATCH_sprite* sprites[SPRITEMAX];
int spriteCount;
const pixel* imageTable[IMAGEMAX];

int count;
int drawRate = 2;

unsigned long interval = 1000 / FRAMESPERSEC;

enum SCRATCH_opcode insertedCode[] = {
    INNER_DEBUGEXPRESSION,
    CONTROL_STOP,
};

int main() {
    unsigned long next = getNow() + interval;
    initData(header, programData, sprites, imageTable);
    //
    code = insertedCode;
    //
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
