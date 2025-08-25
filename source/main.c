#include "scratch.h"
#include <stdlib.h>

struct SCRATCH_sprite masterSprites[16];
struct SCRATCH_sprite sprites[SPRITEMAX];
struct SCRATCH_data variableMemory[VARIABLEMAX];
struct SCRATCH_thread threadMemory[THREADMAX];

const enum SCRATCH_opcode code[] = {
    SCRATCH_DEBUGSTATEMENT,
    SCRATCH_push, SCRATCH_NUMBER, 30, 0,
    SCRATCH_push, SCRATCH_NUMBER, 30, 0,
    SCRATCH_push, SCRATCH_NUMBER, 3, 0,
    SCRATCH_motionGlideto,
    SCRATCH_motion_glideIteration,
    SCRATCH_DEBUGSTATEMENT,
    SCRATCH_stop,

    SCRATCH_DEBUGSTATEMENT,
    SCRATCH_stop,
};

struct SCRATCH_sprite stage;

int main() {
    struct SCRATCH_sprite* sprite = SCRATCH_makeNewSprite(1);
    sprite->threads[0] = (struct SCRATCH_thread) {.active = 1, .programCounter = 0, .loopCounterStackIndex = 0};
    struct SCRATCH_sprite* sprites[1] = {sprite};
    int activeThreadCount = 1;
    while(SCRATCH_visitAllThreads(&stage, sprites, 1));
    free(sprite);
}
