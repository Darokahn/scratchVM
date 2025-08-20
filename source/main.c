#include "scratch.h"

struct SCRATCH_sprite masterSprites[16];
struct SCRATCH_sprite sprites[SPRITEMAX];
struct SCRATCH_data variableMemory[VARIABLEMAX];
struct SCRATCH_thread threadMemory[THREADMAX];

enum SCRATCH_opcode code[] = {
    SCRATCH_DEBUGSTATEMENT,
    SCRATCH_push, SCRATCH_NUMBER, 30, 0,
    SCRATCH_push, SCRATCH_NUMBER, 30, 0,
    SCRATCH_push, SCRATCH_NUMBER, 3, 0,
    SCRATCH_motionGlideto,
    SCRATCH_motion_glideIteration,
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
    SCRATCH_processThread(&stage, &sprite, &spriteThread);
    SCRATCH_processThread(&stage, &sprite, &spriteThread);
    SCRATCH_processThread(&stage, &sprite, &spriteThread);
    printf("%d, %d\n", sprite.x.halves.high, sprite.x.i >> 16);
}
