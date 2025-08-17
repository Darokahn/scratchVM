#include <stdbool.h>
#include "scratch.h"

#define SCRATCH_implementFunction(name) static enum SCRATCH_continueStatus name(struct SCRATCH_sprite* stage, struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, struct SCRATCH_thread* thread)

#define INTERPRET_AS(type, value) *(type*)&(value)

bool SCRATCH_requireNumber();

bool SCRATCH_requireString();

SCRATCH_implementFunction(DEBUG) {
    machineLog("DEBUG\n");
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(loopInit) {
    thread->loopCounterStack[thread->loopCounterStackIndex] = 0;
    thread->loopCounterStackIndex++;
}

SCRATCH_implementFunction(loopIncrement) {
    thread->loopCounterStack[thread->loopCounterStackIndex-1] += 1;
}

SCRATCH_implementFunction(jumpIfRepeatDone) {
    uint16_t toMatch = INTERPRET_AS(uint16_t, thread->code[thread->programCounter]);
    thread->programCounter += sizeof(toMatch);
    uint16_t jumpTo = INTERPRET_AS(uint16_t, thread->code[thread->programCounter]);
    thread->programCounter += sizeof(jumpTo);
    if (thread->loopCounterStack[thread->loopCounterStackIndex-1] >= toMatch) {
        thread->programCounter = jumpTo;
    }
    return SCRATCH_continue;
}

SCRATCH_implementFunction(loopJump) {
    uint16_t to = INTERPRET_AS(uint16_t, thread->code[thread->programCounter]);
    thread->programCounter = to;
    return SCRATCH_yieldGeneric;
}

SCRATCH_implementFunction(fetch) {
    enum SCRATCH_fetchValue toFetch = INTERPRET_AS(enum SCRATCH_fetchValue, thread->code[thread->programCounter]);
    thread->programCounter += sizeof(toFetch);
    uint16_t fetchedValue;
    switch (toFetch) {
        case SCRATCH_xPosition:
            fetchedValue = sprite->x; break;
    }
    stack[*stackIndex] = (struct SCRATCH_data) {SCRATCH_NUMBER, {.number = fetchedValue}};
    (*stackIndex) += 1;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(push) {
    enum SCRATCH_fieldType type = thread->code[thread->programCounter];
    thread->programCounter += sizeof(type);
    uint16_t field = INTERPRET_AS(uint16_t, thread->code[thread->programCounter]);
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
    machineLog("%d, %d\n", op1.data.number, op2.data.number);
    uint16_t result = op1.data.number + op2.data.number;
    machineLog("%d\n", result);
    stack[*stackIndex] = (struct SCRATCH_data) {SCRATCH_NUMBER, {.number = result}};
    (*stackIndex) += 1;
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
    sprite->x = x;
    sprite->y = y;
    return SCRATCH_yieldGeneric;
}

SCRATCH_function operations[MAXOPCODE] = {
    [SCRATCH_loopInit] = loopInit,
    [SCRATCH_loopIncrement] = loopIncrement,
    [SCRATCH_jumpIfRepeatDone] = jumpIfRepeatDone,
    [SCRATCH_fetch] = fetch,
    [SCRATCH_push] = push,
    [SCRATCH_add] = add,
    [SCRATCH_motionGoto] = motionGoto,
    [SCRATCH_loopJump] = loopJump,
    [SCRATCH_DEBUG] = DEBUG,
};

void SCRATCH_processBlock(struct SCRATCH_sprite* stage, struct SCRATCH_sprite* sprite, struct SCRATCH_thread* thread) {
    struct SCRATCH_data stack[STACKMAX];
    int stackIndex = 0;
    char stringRegisterA[STRINGREGISTERMAX];
    char stringRegisterB[STRINGREGISTERMAX];
    int stringIndicesA[2] = {STRINGREGISTERMAX / 2, STRINGREGISTERMAX / 2}; // Each string register can grow on either side
    int stringIndicesB[2] = {STRINGREGISTERMAX / 2, STRINGREGISTERMAX / 2}; // for quick joining
    enum SCRATCH_opcode operation;
    while (true) {
        operation = thread->code[thread->programCounter++];
        enum SCRATCH_continueStatus status = operations[operation](stage, sprite, (struct SCRATCH_data*) stack, &stackIndex, thread);
        thread->currentOperation = status;
        if (operation > SCRATCH_BEGINSTATEMENTS) return; // A block has completed
    }
}
