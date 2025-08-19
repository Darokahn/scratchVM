#include "scratch.h"

struct SCRATCH_sprite masterSprites[16];
struct SCRATCH_sprite sprites[SPRITEMAX];
struct SCRATCH_data variableMemory[VARIABLEMAX];
struct SCRATCH_thread threadMemory[THREADMAX];

enum SCRATCH_opcode code[] = {
    SCRATCH_DEBUGSTATEMENT,
    SCRATCH_push, SCRATCH_NUMBER, 0, 32,
    SCRATCH_motionTurnright,
    SCRATCH_push, SCRATCH_NUMBER, 255, 0,
    SCRATCH_motionMovesteps,
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
    machineLog("%d, %d\n", sprite.x, sprite.y);
}
