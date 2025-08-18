#include "scratch.h"

enum SCRATCH_opcode code[] = {
    SCRATCH_loopInit,
    SCRATCH_jumpIfRepeatDone, 3, 0, 11, 0,
    SCRATCH_loopIncrement,
    SCRATCH_DEBUGEXPRESSION,
    SCRATCH_loopJump, 1, 0,
    SCRATCH_fetch, SCRATCH_xPosition,
    SCRATCH_push, SCRATCH_NUMBER, 1, 0,
    SCRATCH_add,
    SCRATCH_push, SCRATCH_NUMBER, 1, 0,
    SCRATCH_motionGoto,
    SCRATCH_DEBUGSTATEMENT,
};

struct SCRATCH_thread spriteThread = {
    .active = true,
    .programCounter = 0,
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
}
