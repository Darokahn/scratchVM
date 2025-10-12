#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "scratch.h"
#include "programData.h"
#include "externFunctions.h"
#include "externGlobals.h"

#define SCRATCH_implementFunction(name) static enum SCRATCH_continueStatus name(struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, struct SCRATCH_thread* thread)

#define INTERPRET_AS(type, value) *(type*)&(value)
#define PI 3.14159265358979323846264338279f

float degreeToRadian = PI / 32768.0f;
float radianToDegree = 32768.0f / PI;

extern const struct {
    bool* keypresses;
    bool* messages;
    bool* backgroundChanges;
} eventTable;

SCRATCH_implementFunction(DEBUG) {
    machineLog("DEBUG\n");
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(loopInit) {
    thread->loopCounterStack[thread->loopCounterStackIndex] = 0;
    thread->loopCounterStackIndex++;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(loopIncrement) {
    thread->loopCounterStack[thread->loopCounterStackIndex-1] += 1;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(jumpIfRepeatDone) {
    uint16_t toMatch = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof(toMatch);
    uint16_t jumpTo = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof(jumpTo);
    if (thread->loopCounterStack[thread->loopCounterStackIndex-1] >= toMatch) {
        thread->programCounter = jumpTo;
    }
    return SCRATCH_continue;
}

SCRATCH_implementFunction(loopJump) {
    uint16_t to = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter = to;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(fetch) {
    enum SCRATCH_fetchValue toFetch = INTERPRET_AS(enum SCRATCH_fetchValue, code[thread->programCounter]);
    thread->programCounter += sizeof(toFetch);
    uint16_t fetchedValue;
    switch (toFetch) {
        case SCRATCH_xPosition:
            fetchedValue = sprite->base.x.halves.high; break;
    }
    stack[*stackIndex] = (struct SCRATCH_data) {SCRATCH_NUMBER, {.number = fetchedValue}};
    (*stackIndex) += 1;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(push) {
    enum SCRATCH_fieldType type = code[thread->programCounter];
    thread->programCounter += sizeof(type);
    uint16_t field = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof(field);
    stack[*stackIndex] = (struct SCRATCH_data) {type, {.number = field}};
    (*stackIndex) += 1;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(add) {
    struct SCRATCH_data op2 = stack[*stackIndex-1];
    (*stackIndex)--;
    struct SCRATCH_data op1 = stack[*stackIndex-1];
    (*stackIndex)--;
    uint16_t result = op1.data.number + op2.data.number;
    stack[*stackIndex] = (struct SCRATCH_data) {SCRATCH_NUMBER, {.number = result}};
    (*stackIndex) += 1;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(motionGoto) {
    (*stackIndex)--;
    struct SCRATCH_data op2 = stack[*stackIndex];
    (*stackIndex)--;
    struct SCRATCH_data op1 = stack[*stackIndex];
    int x;
    int y;
    if (op1.type == SCRATCH_NUMBER) {
        x = op1.data.number;
    } else return SCRATCH_yieldGeneric;
    if (op2.type == SCRATCH_NUMBER) {
        y = op2.data.number;
    } else return SCRATCH_yieldGeneric;
    sprite->base.x.halves.high = x;
    sprite->base.y.halves.high = y;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(motionMovesteps) {
    (*stackIndex)--;
    struct SCRATCH_data steps = stack[*stackIndex];
    float rotation = sprite->base.rotation * degreeToRadian;

    int x = sin(rotation) * steps.data.number;
    int y = cos(rotation) * steps.data.number;
    sprite->base.x.halves.high += x;
    sprite->base.y.halves.high += y;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(motionTurnright) {
    (*stackIndex)--;
    struct SCRATCH_data degrees = stack[*stackIndex];
    sprite->base.rotation += degrees.data.number;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(motionTurnleft) {
    (*stackIndex)--;
    struct SCRATCH_data degrees = stack[*stackIndex];
    sprite->base.rotation -= degrees.data.number;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(motionGlideto) {
    (*stackIndex)--;
    struct SCRATCH_data scaledSecs = stack[*stackIndex]; // seconds scaled so that the larger 11 bits is seconds and the smaller 5 are seconds / 32
    (*stackIndex)--;
    struct SCRATCH_data x = stack[*stackIndex];
    (*stackIndex)--;
    struct SCRATCH_data y = stack[*stackIndex];
    int32_t xDiff = (x.data.number - sprite->base.x.halves.high) << 16;
    int32_t yDiff = (y.data.number - sprite->base.y.halves.high) << 16;
    uint16_t iterations = (scaledSecs.data.number * FRAMESPERSEC) >> 5;
    if (iterations == 0) {
        sprite->base.x.halves.high = x.data.number;
        sprite->base.y.halves.high = y.data.number;
        thread->operationData.glideData.remainingIterations = 0;
        return SCRATCH_continue;
    }
    thread->operationData.glideData = (struct SCRATCH_glideData) {
        .stepX = xDiff / iterations,
        .stepY = yDiff / iterations,
        .remainingIterations = iterations,
        .targetX = x.data.number,
        .targetY = y.data.number,
    };
    return SCRATCH_continue;
}

SCRATCH_implementFunction(_glideIteration) {
    sprite->base.x.i += thread->operationData.glideData.stepX;
    sprite->base.y.i += thread->operationData.glideData.stepY;
    thread->operationData.glideData.remainingIterations--;
    if (thread->operationData.glideData.remainingIterations <= 0) {
        sprite->base.x.halves.high = thread->operationData.glideData.targetX;
        sprite->base.y.halves.high = thread->operationData.glideData.targetY;
        sprite->base.x.halves.low = 0;
        sprite->base.y.halves.low = 0;
        return SCRATCH_yieldGeneric;
    }
    thread->programCounter--; // re-align program counter with this instruction so it runs again
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(motionPointindirection) {
    (*stackIndex)--;
    struct SCRATCH_data degrees = stack[*stackIndex];
    sprite->base.rotation = degrees.data.number;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(motionPointtowards) {
    (*stackIndex)--;
    struct SCRATCH_data x = stack[*stackIndex];
    (*stackIndex)--;
    struct SCRATCH_data y = stack[*stackIndex];
    float direction = atan2(y.data.number, x.data.number);
    direction *= radianToDegree;
    sprite->base.rotation = (uint16_t) direction;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(motionSetx) {
    (*stackIndex)--;
    struct SCRATCH_data x = stack[*stackIndex];
    sprite->base.x.halves.high = x.data.number;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(motionChangexby) {
    (*stackIndex)--;
    struct SCRATCH_data x = stack[*stackIndex];
    sprite->base.x.halves.high += x.data.number;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(motionSety) {
    (*stackIndex)--;
    struct SCRATCH_data y = stack[*stackIndex];
    sprite->base.y.halves.high = y.data.number;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(motionChangeyby) {
    (*stackIndex)--;
    struct SCRATCH_data y = stack[*stackIndex];
    sprite->base.y.halves.high += y.data.number;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(motionSetrotationstyle) {
    (*stackIndex)--;
    struct SCRATCH_data style = stack[*stackIndex];
    sprite->base.rotationStyle = style.data.boolean;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(stop) {
    thread->active = false;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(looksSay) {
}

SCRATCH_function operations[MAXOPCODE] = {
    [SCRATCH_loopInit] = loopInit,
    [SCRATCH_loopIncrement] = loopIncrement,
    [SCRATCH_jumpIfRepeatDone] = jumpIfRepeatDone,
    [SCRATCH_fetch] = fetch,
    [SCRATCH_push] = push,
    [SCRATCH_add] = add,
    [SCRATCH_motionGoto] = motionGoto,
    [SCRATCH_motionTurnright] = motionTurnright,
    [SCRATCH_motionTurnleft] = motionTurnleft,
    [SCRATCH_motionMovesteps] = motionMovesteps,
    [SCRATCH_motionGlideto] = motionGlideto,
    [SCRATCH_motion_glideIteration] = _glideIteration,
    [SCRATCH_motionPointindirection] = motionPointindirection,
    [SCRATCH_motionPointtowards] = motionPointtowards,
    [SCRATCH_motionChangexby] = motionChangexby,
    [SCRATCH_motionSetx] = motionSetx,
    [SCRATCH_motionChangeyby] = motionChangeyby,
    [SCRATCH_motionSety] = motionSety,
    [SCRATCH_loopJump] = loopJump,
    [SCRATCH_DEBUGEXPRESSION] = DEBUG,
    [SCRATCH_DEBUGSTATEMENT] = DEBUG,
    [SCRATCH_stop] = stop,
};

enum SCRATCH_continueStatus SCRATCH_processBlock(struct SCRATCH_sprite* sprite, struct SCRATCH_thread* thread) {
    struct SCRATCH_data stack[STACKMAX];
    int stackIndex = 0;
    char stringRegisterA[STRINGREGISTERMAX];
    char stringRegisterB[STRINGREGISTERMAX];
    int stringIndicesA[2] = {STRINGREGISTERMAX / 2, STRINGREGISTERMAX / 2}; // Each string register can grow on either side
    int stringIndicesB[2] = {STRINGREGISTERMAX / 2, STRINGREGISTERMAX / 2}; // for quick joining
    enum SCRATCH_opcode operation;
    while (true) {
        operation = code[thread->programCounter++];
        enum SCRATCH_continueStatus status = operations[operation](sprite, (struct SCRATCH_data*) stack, &stackIndex, thread);
        if (operation > SCRATCH_PARTITION_BEGINSTATEMENTS) return status; // A block has completed
    }
}

void SCRATCH_processThread(struct SCRATCH_sprite* sprite, struct SCRATCH_thread* thread) {
    enum SCRATCH_continueStatus status = SCRATCH_continue;
    while (status == SCRATCH_continue) {
        status = SCRATCH_processBlock(sprite, thread);
    }
}

int SCRATCH_visitAllThreads(struct SCRATCH_sprite** sprites, int spriteCount) {
    int activeThreadCount = 0;
    for (int i = 0; i < spriteCount; i++) {
        struct SCRATCH_sprite* sprite = sprites[i];
        for (int ii = 0; ii < sprite->base.threadCount; ii++) {
            if (sprite->threads[ii].active) {
                SCRATCH_processThread(sprite, &sprite->threads[ii]);
                activeThreadCount += sprite->threads[ii].active; // may have changed during this execution
            }
        }
    }
    return activeThreadCount;
}

struct SCRATCH_sprite* SCRATCH_makeNewSprite(struct SCRATCH_spriteHeader header) {
    size_t firstChunkSize =
            sizeof(struct SCRATCH_sprite) +
            header.threadCount * sizeof(struct SCRATCH_thread)
    ;
    firstChunkSize = (firstChunkSize + __alignof__(struct SCRATCH_data) - 1) & ~(__alignof(struct SCRATCH_data) - 1);
    size_t fullChunkSize = firstChunkSize + (sizeof(struct SCRATCH_data) * header.variableCount);
    struct SCRATCH_sprite* spriteChunk = malloc(fullChunkSize);
    if (spriteChunk == NULL) {
        machineLog("Failed allocation in SCRATCH_makeNewSprite\n");
        return NULL;
    }
    spriteChunk->base = header;
    spriteChunk->variables = (struct SCRATCH_data*) (firstChunkSize + (uint8_t*) spriteChunk);
    return spriteChunk;
}
