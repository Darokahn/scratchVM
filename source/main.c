#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "scratch.h"
#include "graphics.h"
#include "externFunctions.h"
#include "externGlobals.h"

unsigned long getNow();

const pixel* imageTable[IMAGEMAX];
struct SCRATCH_sprite* sprites[SPRITEMAX];
extern int spriteCount;

int count = 0;
int drawRate = 2;

unsigned long interval = 1000 / FRAMESPERSEC;

extern const enum SCRATCH_opcode insertedCode[];
extern uint8_t imageBuffer[];

int main() {
    unsigned long next = getNow() + interval;
    initData(header, programData, sprites, imageTable);
    initImages(imageBuffer, imageTable);
    code = (enum SCRATCH_opcode*)insertedCode;
    startGraphics();
    drawSprites(sprites, spriteCount, imageTable);
    updateGraphics();
    return 0;
    setEvent(ONFLAG, (union SCRATCH_eventInput) {0}, true);
    while (true) {
        do {
            handleInputs();
            //machineLog("%d\n", getNow());
        } while (getNow() < next);
        next += interval;
        SCRATCH_visitAllThreads(sprites, spriteCount, imageTable);
        if (count++ % drawRate == 0) {
            drawSprites(sprites, spriteCount, imageTable);
            updateGraphics();
        }
        SCRATCH_wakeSprites();
        clearEvents();
    }
}
