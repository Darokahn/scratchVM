#pragma once
#include "scratch.h"
#include <stdio.h>
#include <stdint.h>

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

struct dataHeader {
    uint16_t spriteCount;
    uint16_t codeLength;
    uint16_t inputCount;
    uint16_t broadcastCount;
    uint16_t backdropCount;
    uint16_t codeOffset;
    uint32_t spriteOffset;
    uint32_t threadOffset;
    uint32_t imageOffset;
};

static inline void initProgram(const uint8_t* buffer, struct SCRATCH_spriteContext* context, const uint8_t** code, int* eventCount, int eventTypeOffsets[]) {
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
