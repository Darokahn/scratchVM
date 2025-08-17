#include "scratch.h"

int THREADMAX = SPRITEMAX * THREADRATIO;

enum SCRATCH_opcode code[64] = {
    SCRATCH_loopInit,
    SCRATCH_jumpIfRepeatDone, 3, 0, 10, 0,
    SCRATCH_DEBUG,
    SCRATCH_loopIncrement,
    SCRATCH_loopJump, 1, 0,
    SCRATCH_fetch, SCRATCH_xPosition,
    SCRATCH_push, SCRATCH_NUMBER, 1, 0,
    SCRATCH_add,
    SCRATCH_push, SCRATCH_NUMBER, 1, 0,
    SCRATCH_motionGoto,
    SCRATCH_DEBUG,
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
    .x = 1,
};

struct SCRATCH_sprite stage;

int main() {
    SCRATCH_processBlock(&stage, &sprite, &spriteThread);
    SCRATCH_processBlock(&stage, &sprite, &spriteThread);
    SCRATCH_processBlock(&stage, &sprite, &spriteThread);
    SCRATCH_processBlock(&stage, &sprite, &spriteThread);
    SCRATCH_processBlock(&stage, &sprite, &spriteThread);
    SCRATCH_processBlock(&stage, &sprite, &spriteThread);
    SCRATCH_processBlock(&stage, &sprite, &spriteThread);
}
