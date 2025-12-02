#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "scratch.h"
#include "graphics.h"
#include "drawing.h"
#include "letters.h"
#include "externFunctions.h"
#include "externGlobals.h"
#include "programData.h"

int eventTypeOffsets[__EVENTTYPECOUNT];
bool inputState[128];
bool events[256];
int eventCount;

unsigned long getNow();

int count = 0;
int drawRate = 2;
const uint8_t* code;

unsigned long interval = 1000 / FRAMESPERSEC;

int runApp(app_t* app) {
    uint8_t* programData = app->programData;
    struct SCRATCH_sprite* sprites[SPRITEMAX];
    struct image* images[IMAGEMAX];
    int spriteSetIndices[SPRITEMAX];
    for (int i = 0; i < SPRITEMAX; i++) {
        sprites[i] = NULL;
    }
    struct SCRATCH_spriteContext context = {
        .sprites = sprites,
        .imageTable = images,
        .spriteSetIndices = spriteSetIndices,
        .currentIndex = 0,
    };
    unsigned long next = getNow() + interval;
    initProgram(programData, &context, &code, &eventCount, eventTypeOffsets);
    setEvent(ONFLAG, (union SCRATCH_eventInput) {0}, true);
    while (true) {
        do {
            handleInputs();
        } while (getNow() < next);
        next += interval;
        SCRATCH_visitAllThreads(&context, (uint8_t*) code);
        if (count++ % drawRate == 0) {
            drawSprites(&context);
            updateIO();
        }
        SCRATCH_wakeSprites(&context);
        clearEvents(eventCount);
    }
}

int main() {
    startIO();
    while (true) {
        app_t currentApp;
        pollApp(&currentApp);
        runApp(&currentApp);
    }
}
