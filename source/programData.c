#include <stdint.h>

#include "programData.h"
#include "externGlobals.h"
#include "externFunctions.h"
#include "scratch.h"

int eventTypeOffsets[3] = {
    0, 5, 0
};

bool inputState[5];

struct image* getImage(const pixel* imageTable[IMAGEMAX], int spriteIndex, int costumeIndex) {
    int imageResolution;
    // The first sprite is implicitly the stage
    if (spriteIndex == 0) imageResolution = STAGERESOLUTION;
    else imageResolution = SPRITERESOLUTION;
    int imageSize = imageResolution * imageResolution * sizeof (pixel) + sizeof(struct image);

    uint8_t* base = (uint8_t*) imageTable[spriteIndex];
    base += costumeIndex * imageSize;
    return (struct image*) base;
}

bool getEvent(enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input) {
    return events[eventTypeOffsets[type] + input.i];
}

void setEvent(enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input, bool state) {
    events[eventTypeOffsets[type] + input.i] = state;
}

void initData(const struct SCRATCH_header header, const uint8_t* buffer, struct SCRATCH_sprite* sprites[SPRITEMAX], const pixel* images[IMAGEMAX]) {
    eventTypeOffsets[2] = header.messageCount + eventTypeOffsets[1];
    code = (enum SCRATCH_opcode*) buffer;
    buffer += header.codeLength;
    buffer = ALIGN8(buffer);
    const uint8_t* imageBase = (const uint8_t*) buffer;
    int imageIndex = 0;
    buffer += header.imageLength;
    buffer = ALIGN8(buffer);
    for (int i = 0; i < header.spriteCount; i++) {
        struct SCRATCH_spriteHeader h = *(struct SCRATCH_spriteHeader*) buffer;
        //
        if (i == 1) {
            h.threadCount = 1;
            h.variableCount = 2;
        }
        //
        struct SCRATCH_sprite* s = SCRATCH_makeNewSprite(h);
        sprites[i] = s;
        buffer += sizeof h;
        buffer = ALIGN8(buffer);
        //
        if (i != 1)
        //
        for (int j = 0; j < h.threadCount; j++) {
            s->threads[j].base = *(struct SCRATCH_threadHeader*) buffer;
            s->threads[j].programCounter = s->threads[j].base.entryPoint;
            buffer += sizeof(struct SCRATCH_threadHeader);
            buffer = ALIGN8(buffer);
        }
        //
        else {
            s->threads[0].base.entryPoint = 0;
            s->threads[0].programCounter = 0;
        }
        //
        int imageSize;
        if (i == 0) imageSize = STAGERESOLUTION * STAGERESOLUTION * sizeof(pixel) + sizeof (struct image);
        else imageSize = SPRITERESOLUTION * SPRITERESOLUTION * sizeof(pixel) + sizeof (struct image);
        images[imageIndex] = (pixel*) imageBase;
        imageIndex++;
        for (int j = 0; j < h.costumeMax; j++) {
            imageBase += imageSize;
        }
    }
}
