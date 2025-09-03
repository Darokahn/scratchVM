#include "scratch.h"
#include <stdlib.h>

struct SCRATCH_sprite* sprites;
uint8_t spriteCount;
struct SCRATCH_sprite stage;

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

int main() {
    struct SCRATCH_sprite* sprite = SCRATCH_makeNewSprite(1, 1);
    sprite->threads[0] = (struct SCRATCH_thread) {.active = 1, .programCounter = 0, .loopCounterStackIndex = 0};
    struct SCRATCH_sprite* sprites[1] = {sprite};
    int activeThreadCount = 1;
    while(SCRATCH_visitAllThreads(&stage, sprites, 1));
    free(sprite);
}
