#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include "scratch.h"
#include "ioFuncs.h"
#include "globals.h"
#include "programData.h"

struct image* getImage(struct SCRATCH_spriteContext* context, struct SCRATCH_sprite* operand) {
    struct SCRATCH_sprite* s = operand;
    if (s == NULL) s = context->sprites[context->currentIndex];
    int index = context->spriteSetIndices[s->base.id] + s->base.costumeIndex;
    struct image* image = context->imageTable[index];
    return image;
}

bool getEvent(enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input) {
    int index = eventTypeOffsets[type] + input.i;
    if (index == -1) return false;
    return events[index];
}

void setEvent(enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input, bool state) {
    events[eventTypeOffsets[type] + input.i] = state;
}

void initImages(struct SCRATCH_spriteContext* context, const uint8_t* buffer) {
    struct SCRATCH_sprite* s;
    int imageCount = 0;
    for (int i = 0; i < context->spriteCount; i++) {
        context->spriteSetIndices[i] = imageCount;
        s = context->sprites[i];
        for (int costumeCount = 0; costumeCount < s->base.costumeMax; costumeCount++) {
            struct image* thisImage = (void*) buffer;
            context->imageTable[imageCount++] = thisImage;
            int imageSize = sizeof (*thisImage) + (thisImage->xResolution * thisImage->yResolution * sizeof(pixel));
            machineLog("Loaded image %s, pixel size %dx%d, logical size %d%%x%d%%\n\r", thisImage->name, thisImage->xResolution, thisImage->yResolution, thisImage->widthRatio * 100 / 255, thisImage->heightRatio * 100 / 255);
            buffer += imageSize;
        }
    }
}

void debugSprite(struct SCRATCH_sprite* sprite) {
    static bool hasPrintedGeneral = false;
    struct SCRATCH_spriteHeader base = sprite->base;
    if (!hasPrintedGeneral) {
        machineLog("General thread information: sizeof header = %d, alignof = %d\n\r", sizeof(struct SCRATCH_threadHeader), __alignof(struct SCRATCH_threadHeader));
        machineLog("General sprite information: sizeof header = %d, alignof = %d\n\r", sizeof(base), __alignof(base));
        machineLog("Fields are printed as-is in their native format, which may be misleading. Several fields are scaled integers.\n\r");
        hasPrintedGeneral = true;
    }
    machineLog("This sprite:\n\r\tx: %d, y: %d, rotation: %d, size: %d, visible: %d, layer: %d, rotationStyle: %d, costumeIndex: %d, costumeMax: %d, threadCount: %d, variableCount: %d, id: %d\n\r", base.x.i, base.y.i, base.rotation, base.size, base.visible, base.layer, base.rotationStyle, base.costumeIndex, base.costumeMax, base.threadCount, base.variableCount, base.id);
    machineLog("Threads for this sprite:\n\r");
    for (int i = 0; i < base.threadCount; i++) {
        struct SCRATCH_threadHeader tHeader = sprite->threads[i].base;
        machineLog("\tcondition: %d, entry: %d, start: %d\n\r", tHeader.eventCondition, tHeader.entryPoint, tHeader.startEvent);
    }
}

void initProgram(const uint8_t* buffer, struct SCRATCH_spriteContext* context, const uint8_t** code, int* eventCount, int eventTypeOffsets[]) {
    struct dataHeader* d = (struct dataHeader*)buffer;
    int offsetTotal = 0;
    eventTypeOffsets[ONKEY] = offsetTotal;
    offsetTotal += d->inputCount;
    eventTypeOffsets[ONMESSAGE] = offsetTotal;
    offsetTotal += d->broadcastCount;
    eventTypeOffsets[ONBACKDROP] = offsetTotal;
    offsetTotal += d->backdropCount;;
    eventTypeOffsets[ONCLONE] = -1; // ONCLONE is an event, but it is not triggered globally so takes no space in the events array
    offsetTotal += 0;
    eventTypeOffsets[ONFLAG] = offsetTotal;
    offsetTotal += 1;
    eventTypeOffsets[ONCLICK] = -1;
    offsetTotal += 0;
    eventTypeOffsets[ONLOUDNESS] = -1;
    offsetTotal += 0;
    *eventCount = offsetTotal;
    machineLog("\n\r\n\r");
    machineLog("%d %d %d %d %d %d %d %d %d\n\r", d->spriteCount, d->codeLength, d->inputCount, d->broadcastCount, d->backdropCount, d->codeOffset, d->spriteOffset, d->threadOffset, d->imageOffset);
    *code = &(buffer[d->codeOffset]);
    const struct SCRATCH_spriteHeader* spriteHeaders = (struct SCRATCH_spriteHeader*) &(buffer[d->spriteOffset]);
    const struct SCRATCH_threadHeader* threadHeaders = (struct SCRATCH_threadHeader*) &(buffer[d->threadOffset]);
    context->spriteCount = d->spriteCount;
    for (int i = 0; i < d->spriteCount; i++) {
        struct SCRATCH_spriteHeader spriteHeader = spriteHeaders[i];
        if (context->sprites[i] != NULL) free(context->sprites[i]);
        context->sprites[i] = SCRATCH_makeNewSprite(spriteHeader);
        for (int ii = 0; ii < spriteHeader.threadCount; ii++) {
            struct SCRATCH_threadHeader threadHeader = *threadHeaders++;
            SCRATCH_initThread(&(context->sprites[i]->threads[ii]), threadHeader);
        }
        debugSprite(context->sprites[i]);
    }
    context->stage = context->sprites[0];
    initImages(context, &(buffer[d->imageOffset]));
}
