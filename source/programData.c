#include <stdint.h>

#include "programData.h"
#include "scratch.h"

const uint16_t* getImage(const uint16_t* imageTable[IMAGEMAX], int spriteIndex, int costumeIndex) {
    int imageResolution;
    // The first sprite is implicitly the stage
    if (spriteIndex == 0) imageResolution = STAGERESOLUTION;
    else imageResolution = SPRITERESOLUTION;

    const uint16_t* base = imageTable[spriteIndex];
    base += costumeIndex * (imageResolution * imageResolution);
    return base;
}

void initData(const struct SCRATCH_header header, const uint8_t* buffer, struct SCRATCH_sprite* sprites[SPRITEMAX], const uint16_t* images[IMAGEMAX]) {
    code = (enum SCRATCH_opcode*) buffer;
    buffer += header.codeLength;
    buffer = ALIGN8(buffer);
    const uint16_t* imageBase = (const uint16_t*) buffer;
    int imageIndex = 0;
    buffer += header.imageLength;
    buffer = ALIGN8(buffer);
    for (int i = 0; i < header.spriteCount; i++) {
        struct SCRATCH_spriteHeader h = *(struct SCRATCH_spriteHeader*) buffer;
        struct SCRATCH_sprite* s = SCRATCH_makeNewSprite(h);
        sprites[i] = s;
        buffer += sizeof h;
        buffer = ALIGN8(buffer);
        for (int j = 0; j < h.threadCount; j++) {
            s->threads[j].base = *(struct SCRATCH_threadHeader*) buffer;
            s->threads[j].programCounter = s->threads[j].base.entryPoint;
            buffer += sizeof(struct SCRATCH_threadHeader);
            buffer = ALIGN8(buffer);
        }
        int imageSize;
        if (i == 0) imageSize = STAGERESOLUTION * STAGERESOLUTION;
        else imageSize = SPRITERESOLUTION * SPRITERESOLUTION;
        for (int j = 0; j < h.costumeMax; j++) {
            images[imageIndex] = imageBase;
            imageIndex++;
            imageBase += imageSize;
        }
    }
}
