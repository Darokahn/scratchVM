#include "scratch.h"

int THREADMAX = SPRITEMAX * THREADRATIO;

enum SCRATCH_opcode code[32] = {
        SCRATCH_push,
        SCRATCH_NUMBER,
        0xa,
        0,
        0,
        0,
        SCRATCH_push,
        SCRATCH_NUMBER,
        0xff,
        0,
        0,
        0,
        SCRATCH_motionGoto,
};

struct SCRATCH_thread spriteThread = {
    .active = true,
    .code = code,
    .programCounter = 0,
    .instructionLength = 2,
    .loopCounterStackIndex = 0,
};

struct SCRATCH_sprite sprite = {
    .threads = &spriteThread,
    .threadCount = 1,
};

struct SCRATCH_sprite stage;

int main() {
    SCRATCH_processBlock(&stage, &sprite, &spriteThread);
    machineLog("hello world!\n");
    machineLog("%d, %d\n", sprite.x, sprite.y);
}
