#include <stdint.h>

#include "externGlobals.h"
#include "externFunctions.h"
#include "scratch.h"

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

void initImages(struct SCRATCH_spriteContext* context, uint8_t* buffer) {
    struct SCRATCH_sprite* s;
    int imageCount = 0;
    for (int i = 0; i < context->spriteCount; i++) {
        context->spriteSetIndices[i] = imageCount;
        s = context->sprites[i];
        for (int costumeCount = 0; costumeCount < s->base.costumeMax; costumeCount++) {
            struct image* thisImage = (void*) buffer;
            context->imageTable[imageCount++] = thisImage;
            int imageSize = sizeof (*thisImage) + (thisImage->xResolution * thisImage->yResolution * sizeof(pixel));
            buffer += imageSize;
        }
    }
}
