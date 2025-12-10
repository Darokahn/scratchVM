#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "scratch.h"
#include "ioFuncs.h"
#include "globals.h"
#include "programData.h"

int eventTypeOffsets[__EVENTTYPECOUNT];
bool inputState[128];
bool events[256];
int eventCount;

unsigned long getNow();

int count = 0;
int drawRate = 1;
const uint8_t* code;

char appName[12] = "app";

unsigned long interval = 1000 / FRAMESPERSEC;

int runApp(app_t* app) {
    machineLog("Running app\n\r");
    uint8_t* programData = app->programData;
    struct SCRATCH_sprite* sprites[SPRITEMAX];
    struct image* images[IMAGEMAX];
    uint16_t spriteSetIndices[SPRITEMAX];
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
            int continueStatus = updateIO(app);
            if (continueStatus != 0) {
                SCRATCH_freeSprites(&context);
                return continueStatus;
            }
        }
        SCRATCH_wakeSprites(&context);
        clearEvents(eventCount);
    }
}

int main() {
    startIO();
    app_t app;
    selectApp(&app, "app");
    while (true) {
        runApp(&app);
        closeApp(&app, 0);
    }
}
