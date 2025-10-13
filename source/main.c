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
    SCRATCH_push, 1, 0x41, 0xff,
    SCRATCH_push, 1, 0x0, 0x0,
    SCRATCH_motionGoto,
    SCRATCH_push, 1, 0xa, 0x0,
    SCRATCH_setVar, 0x0, 0x0,
    SCRATCH_push, 0x1, 0x0, 0x0,
    SCRATCH_setVar, 0x1, 0x0,
    // 1:
    SCRATCH_fetchInput, 0x0, 0x0,
    SCRATCH_jumpIfNot, 35, 0x0,
    SCRATCH_loadVar, 0x0, 0x0,
    SCRATCH_incVar, 0x1, 0x0,
    // 2:
    SCRATCH_fetchInput, 0x2, 0x0,
    SCRATCH_jumpIfNot, 52, 0x0,
    SCRATCH_push, 1, 0x0, 0x0,
    SCRATCH_loadVar, 0x0, 0x0,
    SCRATCH_sub,
    SCRATCH_incVar, 0x1, 0x0,
    // 3:
    SCRATCH_push, 1, 0x41, 0xff,
    SCRATCH_loadVar, 0x1, 0x0,
    SCRATCH_motionGoto,
    SCRATCH_loopJump, 23, 0x0,
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
    while (true) {
        do handleInputs(); while (getNow() < next);
        next += interval;
        SCRATCH_visitAllThreads(sprites, header.spriteCount);
        if (count++ % drawRate == 0) {
            drawSprites(sprites, header.spriteCount, imageTable);
            updateGraphics();
        }
        SCRATCH_wakeSprites();
        clearEvents();
    }
}
