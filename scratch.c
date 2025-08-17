#include <stdbool.h>
#include "scratch.h"

#define SCRATCH_implementFunction(name) static enum SCRATCH_continueStatus name(struct SCRATCH_sprite* stage, struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, struct SCRATCH_thread* thread)

SCRATCH_implementFunction(push) {
    enum SCRATCH_fieldType type = thread->code[thread->programCounter];
    thread->programCounter++;
    union SCRATCH_field field = {.string = *(halfStringPointer*)&thread->code[thread->programCounter]};
    thread->programCounter += sizeof(union SCRATCH_field);
    stack[*stackIndex] = (struct SCRATCH_data) {type, field};
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
    }
    if (op2.type == SCRATCH_NUMBER) {
        y = op2.data.number;
    }
    sprite->x = x;
    sprite->y = y;
    return SCRATCH_yieldGeneric;
}

SCRATCH_function operations[MAXOPCODE] = {
    [SCRATCH_push] = push,
    [SCRATCH_motionGoto] = motionGoto,
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
