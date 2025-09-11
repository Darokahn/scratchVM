#include "scratch.h"
#include "programData.h"

#define ALIGN8(ptr) ((ptr + 7) & ~7)

uint8_t* getImage(uint8_t* table[IMAGEMAX], int spriteIndex, int costumeIndex) {
    int imageResolution;
    // The first sprite is implicitly the stage
    if (spriteIndex == 0) imageResolution = STAGERESOLUTION;
    else imageResolution = SPRITERESOLUTION;

    uint8_t* base = table[spriteIndex];
    base += costumeIndex * (imageResolution * imageResolution);
    return base;
}

void initData(struct SCRATCH_header header, uint8_t* buffer, struct SCRATCH_sprite* sprites[SPRITEMAX], uint8_t* images[IMAGEMAX]) {
    code = (enum SCRATCH_opcode*) buffer;
    buffer += header.codeLength;
    buffer = ALIGN8(buffer);
    uint8_t* imageBase = buffer;
    buffer += header.imageLength;
    buffer = ALIGN8(buffer);
}
