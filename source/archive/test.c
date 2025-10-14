#include "../scratch.h"
#include <stddef.h>

int main(void) {
    printf("Offsets in SCRATCH_spriteHeader:\n");
    printf("x             = %zu\n", offsetof(struct SCRATCH_spriteHeader, x));
    printf("y             = %zu\n", offsetof(struct SCRATCH_spriteHeader, y));
    printf("rotation      = %zu\n", offsetof(struct SCRATCH_spriteHeader, rotation));
    printf("visible       = %zu\n", offsetof(struct SCRATCH_spriteHeader, visible));
    printf("layer         = %zu\n", offsetof(struct SCRATCH_spriteHeader, layer));
    printf("size          = %zu\n", offsetof(struct SCRATCH_spriteHeader, size));
    printf("rotationStyle = %zu\n", offsetof(struct SCRATCH_spriteHeader, rotationStyle));
    printf("costumeIndex  = %zu\n", offsetof(struct SCRATCH_spriteHeader, costumeIndex));
    printf("costumeMax    = %zu\n", offsetof(struct SCRATCH_spriteHeader, costumeMax));
    printf("threadCount   = %zu\n", offsetof(struct SCRATCH_spriteHeader, threadCount));
    printf("variableCount = %zu\n", offsetof(struct SCRATCH_spriteHeader, variableCount));
    printf("id = %zu\n", offsetof(struct SCRATCH_spriteHeader, id));
    printf("sizeof        = %zu\n", sizeof(struct SCRATCH_spriteHeader));

    printf("\nOffsets in SCRATCH_threadHeader:\n");
    printf("eventCondition = %zu\n", offsetof(struct SCRATCH_threadHeader, eventCondition));
    printf("entryPoint     = %zu\n", offsetof(struct SCRATCH_threadHeader, entryPoint));
    printf("startEvent     = %zu\n", offsetof(struct SCRATCH_threadHeader, startEvent));
    printf("sizeof         = %zu\n", sizeof(struct SCRATCH_threadHeader));

    return 0;
}
