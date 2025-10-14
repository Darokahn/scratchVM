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

SCRATCH_implementFunction(jumpIf) {
    uint16_t jumpTo = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof jumpTo;
    struct SCRATCH_data evaluand = stack[*stackIndex-1];
    (*stackIndex) -= 1;
    if (evaluand.data.boolean) {
        thread->programCounter = jumpTo;
    }
    return SCRATCH_continue;
}

SCRATCH_implementFunction(jumpIfNot) {
    uint16_t jumpTo = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof jumpTo;
    struct SCRATCH_data evaluand = stack[*stackIndex-1];
    (*stackIndex) -= 1;
    if (!evaluand.data.boolean) {
        thread->programCounter = jumpTo;
    }
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

SCRATCH_implementFunction(fetchInput) {
    uint16_t toFetch = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof(toFetch);
    stack[*stackIndex] = (struct SCRATCH_data) {SCRATCH_BOOL, {.boolean = inputState[toFetch]}};
    (*stackIndex) += 1;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(loadVar) {
    uint16_t varIndex = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof varIndex;
    stack[*stackIndex] = sprite->variables[varIndex];
    (*stackIndex) += 1;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(setVar) {
    uint16_t varIndex = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof varIndex;
    struct SCRATCH_data value = stack[*stackIndex-1];
    (*stackIndex) -= 1;
    sprite->variables[varIndex] = value;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(incVar) {
    uint16_t varIndex = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof varIndex;
    struct SCRATCH_data value = stack[*stackIndex-1];
    (*stackIndex) -= 1;
    sprite->variables[varIndex].data.number += value.data.number;
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

SCRATCH_implementFunction(sub) {
    struct SCRATCH_data op2 = stack[*stackIndex-1];
    (*stackIndex)--;
    struct SCRATCH_data op1 = stack[*stackIndex-1];
    (*stackIndex)--;
    uint16_t result = op1.data.number - op2.data.number;
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

SCRATCH_implementFunction(clone) {
    int16_t field = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof field;
    struct SCRATCH_spriteHeader h;
    struct SCRATCH_sprite* template;
    if (field == -1) {
        h = sprite->base;
        template = sprite;
    }
    else {
        h = sprites[field]->base;
        template = sprites[field];
    }
    struct SCRATCH_sprite* newSprite = SCRATCH_makeNewSprite(h);
    for (int i = 0; i < h.threadCount; i++) {
        newSprite->threads[i].base = template->threads[i].base;
        newSprite->threads[i].active = false;
        newSprite->threads[i].programCounter = template->threads[i].base.entryPoint;
    }
    for (int i = 0; i < h.variableCount; i++) {
        // variables need to be handled specially because they can hold allocated strings that need to be copied. TODO
    }
    SCRATCH_wakeSprite(newSprite, SCRATCH_clone, (union SCRATCH_eventInput) {0});
    if (!SCRATCH_addSprite(newSprite)) {
        free(newSprite);
    }
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(looksSay) {
}

SCRATCH_function operations[MAXOPCODE] = {
    [SCRATCH_loopInit] = loopInit,
    [SCRATCH_loopIncrement] = loopIncrement,
    [SCRATCH_jumpIfRepeatDone] = jumpIfRepeatDone,
    [SCRATCH_jumpIf] = jumpIf,
    [SCRATCH_jumpIfNot] = jumpIfNot,
    [SCRATCH_fetch] = fetch,
    [SCRATCH_fetchInput] = fetchInput,
    [SCRATCH_push] = push,
    [SCRATCH_add] = add,
    [SCRATCH_sub] = sub,
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
    [SCRATCH_motionSetrotationstyle] = motionSetrotationstyle,
    [SCRATCH_DEBUGEXPRESSION] = DEBUG,
    [SCRATCH_DEBUGSTATEMENT] = DEBUG,
    [SCRATCH_stop] = stop,
    [SCRATCH_looksSay] = looksSay,
    [SCRATCH_loadVar] = loadVar,
    [SCRATCH_setVar] = setVar,
    [SCRATCH_incVar] = incVar,
    [SCRATCH_clone] = clone,
};

void handleInputs() {
    for (int i = 0; i < 5; i++) {
        bool prior = inputState[i];
        bool new = getInput(i); 
        inputState[i] = new;
        if (new && new != prior) {
            setEvent(ONKEY, (union SCRATCH_eventInput) {.key=i}, new);
        }
    }
}

void clearEvents() {
    for (int i = 0; i < eventCount; i++) {
        events[i] = false;
    }
}

enum SCRATCH_continueStatus SCRATCH_processBlock(struct SCRATCH_sprite* sprite, struct SCRATCH_thread* thread) {
    struct SCRATCH_data stack[STACKMAX];
    int stackIndex = 0;
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

void SCRATCH_initThread(struct SCRATCH_thread* thread, struct SCRATCH_threadHeader h) {
    thread->programCounter = h.entryPoint;
    thread->active = false;
    thread->loopCounterStackIndex = 0;
    thread->base = h;
}

bool SCRATCH_addSprite(struct SCRATCH_sprite* sprite) {
    if (spriteCount >= SPRITEMAX) return false;
    else {
        sprites[spriteCount++] = sprite;
        return true;
    }
}

bool SCRATCH_wakeSprite(struct SCRATCH_sprite* sprite, enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input) {
    bool woke = false;
    for (int i = 0; i < sprite->base.threadCount; i++) {
        struct SCRATCH_thread* t = &sprite->threads[i];
        if (t->base.startEvent == type && t->base.eventCondition.i == input.i) {
            t->active = true;
            woke = true;
        }
    }
    return woke;
}

void SCRATCH_wakeSprites() {
    for (int i = 0; i < spriteCount; i++) {
        struct SCRATCH_sprite* s = sprites[i];
        for (int j = 0; j < s->base.threadCount; j++) {
            struct SCRATCH_thread* t = &s->threads[j];
            if (t->active) {
                continue;
            }
            if (!getEvent(t->base.startEvent, t->base.eventCondition)) continue;
            t->active = true;
        }
    }
}
