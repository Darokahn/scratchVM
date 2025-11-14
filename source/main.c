#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "scratch.h"
#include "graphics.h"
#include "externFunctions.h"
#include "externGlobals.h"

unsigned long getNow();

int count = 0;
int drawRate = 2;

unsigned long interval = 1000 / FRAMESPERSEC;
extern uint8_t imageBuffer[];

int main() {
    struct SCRATCH_sprite* sprites[SPRITEMAX];
    struct image* images[IMAGEMAX];
    int spriteSetIndices[SPRITEMAX];
    struct SCRATCH_spriteContext context = {
        .sprites = sprites,
        .imageTable = images,
        .spriteSetIndices = spriteSetIndices,
    };
    unsigned long next = getNow() + interval;
    uint8_t* code;
    initData(&context, &code);
    initImages(&context, imageBuffer);
    getImage(&context, context.sprites[0]);
    startGraphics();
    updateGraphics();
    setEvent(ONFLAG, (union SCRATCH_eventInput) {0}, true);
    while (true) {
        do {
            handleInputs();
        } while (getNow() < next);
        next += interval;
        inputState[1] = 1;
        SCRATCH_visitAllThreads(&context, code);
        if (count++ % drawRate == 0) {
            drawSprites(&context);
            updateGraphics();
        }
        SCRATCH_wakeSprites(&context);
        clearEvents();
    }
}
