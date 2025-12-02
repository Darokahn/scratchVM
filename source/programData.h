#pragma once
#include "scratch.h"
#include <stdio.h>
#include <stdint.h>

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
    *code = &(buffer[d->codeOffset]);
    const struct SCRATCH_spriteHeader* spriteHeaders = (struct SCRATCH_spriteHeader*) &(buffer[d->spriteOffset]);
    const struct SCRATCH_threadHeader* threadHeaders = (struct SCRATCH_threadHeader*) &(buffer[d->threadOffset]);
    context->spriteCount = d->spriteCount;
    for (int i = 0; i < d->spriteCount; i++) {
        struct SCRATCH_spriteHeader spriteHeader = spriteHeaders[i];
        context->sprites[i] = SCRATCH_makeNewSprite(spriteHeader);
        for (int ii = 0; ii < spriteHeader.threadCount; ii++) {
            struct SCRATCH_threadHeader threadHeader = *threadHeaders++;
            SCRATCH_initThread(&(context->sprites[i]->threads[ii]), threadHeader);
        }
    }
    context->stage = context->sprites[0];
    initImages(context, &(buffer[d->imageOffset]));
}
