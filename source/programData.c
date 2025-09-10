#include "scratch.h"
#include "programData.h"

uint8_t* getImage(uint8_t* table[IMAGEMAX], int spriteIndex, int costumeIndex) {
    int imageResolution;
    // The first sprite is implicitly the stage
    if (spriteIndex == 0) imageResolution = STAGERESOLUTION;
    else imageResolution = SPRITERESOLUTION;

    uint8_t* base = table[spriteIndex];
    base += costumeIndex * (imageResolution * imageResolution);
    return base;
}
