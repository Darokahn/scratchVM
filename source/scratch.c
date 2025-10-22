#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "scratch.h"
#include "programData.h"
#include "externFunctions.h"
#include "externGlobals.h"

#define SCRATCH_implementFunction(name) static enum SCRATCH_continueStatus name(struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, struct SCRATCH_thread* thread)

/*
SCRATCH_function operations[MAXOPCODE] = {
    STOP,
    PARTITION_BEGINLOOPCONTROL,
    LOOPINIT,
    LOOPINCREMENT,
    JUMPIFREPEATDONE,
    PARTITION_BEGINEXPRESSIONS,
    ANSWER,
    MOUSEDOWN,
    MOUSEX,
    MOUSEY,
    KEYPRESSED,
    LOUDNESS,
    TIMER,
    CURRENT,
    DAYSSINCE2000,
    USERNAME,
    FETCHINPUT,
    FETCHVAR,
    XPOSITION,
    YPOSITION,
    DIRECTION,
    COSTUME,
    SIZE,
    COSTUMENUMBERNAME,
    BACKDROPNUMBERNAME,
    TOUCHINGOBJECT,
    TOUCHINGOBJECTMENU,
    TOUCHINGCOLOR,
    COLORISTOUCHINGCOLOR,
    DISTANCETO,
    DISTANCETOMENU,
    ASKANDWAIT,
    KEYOPTIONS,
    SETDRAGMODE,
    RESETTIMER,
    OF,
    OF_OBJECT_MENU,
    PUSH,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    RANDOM,
    GT,
    LT,
    EQUALS,
    GE,
    AND,
    OR,
    NOT,
    JOIN,
    LETTER_OF,
    LENGTH,
    CONTAINS,
    MOD,
    ROUND,
    MATHOP,
    DEBUGEXPRESSION,
    PARTITION_BEGINSTATEMENTS,
    SETVARIABLETO,
    CHANGEVARIABLEBY,
    SHOWVARIABLE,
    HIDEVARIABLE,
    LOOPJUMP,
    CREATE_CLONE_OF,
    WAIT,
    REPEAT,
    FOREVER,
    IF,
    IF_ELSE,
    WAIT_UNTIL,
    REPEAT_UNTIL,
    CREATE_CLONE_OF_MENU,
    DELETE_THIS_CLONE,
    JUMPIF,
    JUMPIFNOT,
    JUMP,
    _GLIDEITERATION,
    MOVESTEPS,
    TURNRIGHT,
    GOTO,
    GOTO_MENU,
    GOTOXY,
    GLIDETO,
    GLIDETO_MENU,
    GLIDESECSTOXY,
    POINTINDIRECTION,
    POINTTOWARDS,
    POINTTOWARDS_MENU,
    CHANGEXBY,
    SETX,
    CHANGEYBY,
    SETY,
    IFONEDGEBOUNCE,
    SETROTATIONSTYLE,
    SAY,
    THINKFORSECS,
    THINK,
    SWITCHCOSTUMETO,
    NEXTCOSTUME,
    SWITCHBACKDROPTO,
    BACKDROPS,
    NEXTBACKDROP,
    CHANGESIZEBY,
    SETSIZETO,
    CHANGEEFFECTBY,
    SETEFFECTTO,
    CLEARGRAPHICEFFECTS,
    SHOW,
    HIDE,
    GOTOFRONTBACK,
    GOFORWARDBACKWARDLAYERS,
    _WAITITERATION,
    DEBUGSTATEMENT,
};
*/

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
        // new proposed:
        enum SCRATCH_continueStatus status;
        switch (operation) {
            #include "opcodeImpl.inc"
        }
        if (operation > INNER_PARTITION_BEGINSTATEMENTS) return status; // A block has completed
        /* old:
        enum SCRATCH_continueStatus status = operations[operation](sprite, (struct SCRATCH_data*) stack, &stackIndex, thread);
        if (operation > SCRATCH_PARTITION_BEGINSTATEMENTS) return status; // A block has completed
        */
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
            t->programCounter = t->base.entryPoint;
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
            t->programCounter = t->base.entryPoint;
        }
    }
}
