#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "scratch.h"
#include "externFunctions.h"
#include "externGlobals.h"

#define SCRATCH_implementFunction(name) static enum SCRATCH_continueStatus name(struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, struct SCRATCH_thread* thread)

const char* SCRATCH_opcode_names[INNER_DEBUGSTATEMENT + 1] = {
    [INNER_PARTITION_BEGINLOOPCONTROL] = "INNER_PARTITION_BEGINLOOPCONTROL",
    [INNER_LOOPINIT] = "INNER_LOOPINIT",
    [INNER_LOOPINCREMENT] = "INNER_LOOPINCREMENT",
    [INNER_JUMPIFREPEATDONE] = "INNER_JUMPIFREPEATDONE",

    [INNER_PARTITION_BEGINEXPRESSIONS] = "INNER_PARTITION_BEGINEXPRESSIONS",

    [SENSING_ANSWER] = "SENSING_ANSWER",
    [SENSING_MOUSEDOWN] = "SENSING_MOUSEDOWN",
    [SENSING_MOUSEX] = "SENSING_MOUSEX",
    [SENSING_MOUSEY] = "SENSING_MOUSEY",
    [SENSING_KEYPRESSED] = "SENSING_KEYPRESSED",
    [SENSING_LOUDNESS] = "SENSING_LOUDNESS",
    [SENSING_TIMER] = "SENSING_TIMER",
    [SENSING_CURRENT] = "SENSING_CURRENT",
    [SENSING_DAYSSINCE2000] = "SENSING_DAYSSINCE2000",
    [SENSING_USERNAME] = "SENSING_USERNAME",
    [INNER_FETCHINPUT] = "INNER_FETCHINPUT",
    [INNER_FETCHPOSITION] = "INNER_FETCHPOSITION",
    [INNER_FETCHVAR] = "INNER_FETCHVAR",
    [MOTION_XPOSITION] = "MOTION_XPOSITION",
    [MOTION_YPOSITION] = "MOTION_YPOSITION",
    [MOTION_DIRECTION] = "MOTION_DIRECTION",
    [LOOKS_COSTUME] = "LOOKS_COSTUME",
    [LOOKS_SIZE] = "LOOKS_SIZE",
    [LOOKS_COSTUMENUMBERNAME] = "LOOKS_COSTUMENUMBERNAME",
    [LOOKS_BACKDROPNUMBERNAME] = "LOOKS_BACKDROPNUMBERNAME",

    [SENSING_TOUCHINGOBJECT] = "SENSING_TOUCHINGOBJECT",
    [SENSING_TOUCHINGOBJECTMENU] = "SENSING_TOUCHINGOBJECTMENU",
    [SENSING_TOUCHINGCOLOR] = "SENSING_TOUCHINGCOLOR",
    [SENSING_COLORISTOUCHINGCOLOR] = "SENSING_COLORISTOUCHINGCOLOR",
    [SENSING_DISTANCETO] = "SENSING_DISTANCETO",
    [SENSING_DISTANCETOMENU] = "SENSING_DISTANCETOMENU",
    [SENSING_ASKANDWAIT] = "SENSING_ASKANDWAIT",
    [SENSING_KEYOPTIONS] = "SENSING_KEYOPTIONS",
    [SENSING_SETDRAGMODE] = "SENSING_SETDRAGMODE",
    [SENSING_RESETTIMER] = "SENSING_RESETTIMER",
    [SENSING_OF] = "SENSING_OF",
    [SENSING_OF_OBJECT_MENU] = "SENSING_OF_OBJECT_MENU",

    [INNER_PUSHNUMBER] = "INNER_PUSHNUMBER",
    [INNER_PUSHTEXT] = "INNER_PUSHTEXT",
    [INNER_PUSHDEGREES] = "INNER_PUSHDEGREES",

    [OPERATOR_ADD] = "OPERATOR_ADD",
    [OPERATOR_SUBTRACT] = "OPERATOR_SUBTRACT",
    [OPERATOR_MULTIPLY] = "OPERATOR_MULTIPLY",
    [OPERATOR_DIVIDE] = "OPERATOR_DIVIDE",
    [OPERATOR_RANDOM] = "OPERATOR_RANDOM",
    [OPERATOR_GT] = "OPERATOR_GT",
    [OPERATOR_LT] = "OPERATOR_LT",
    [OPERATOR_EQUALS] = "OPERATOR_EQUALS",
    [INNER_LE] = "INNER_LE",
    [INNER_GE] = "INNER_GE",
    [OPERATOR_AND] = "OPERATOR_AND",
    [OPERATOR_OR] = "OPERATOR_OR",
    [OPERATOR_NOT] = "OPERATOR_NOT",
    [OPERATOR_JOIN] = "OPERATOR_JOIN",
    [OPERATOR_LETTER_OF] = "OPERATOR_LETTER_OF",
    [OPERATOR_LENGTH] = "OPERATOR_LENGTH",
    [OPERATOR_CONTAINS] = "OPERATOR_CONTAINS",
    [OPERATOR_MOD] = "OPERATOR_MOD",
    [OPERATOR_ROUND] = "OPERATOR_ROUND",
    [OPERATOR_MATHOP] = "OPERATOR_MATHOP",

    [INNER_DEBUGEXPRESSION] = "INNER_DEBUGEXPRESSION",

    [INNER_PARTITION_BEGINSTATEMENTS] = "INNER_PARTITION_BEGINSTATEMENTS",

    [DATA_SETVARIABLETO] = "DATA_SETVARIABLETO",
    [DATA_CHANGEVARIABLEBY] = "DATA_CHANGEVARIABLEBY",
    [INNER_CHANGEVARIABLEBYLOCAL] = "INNER_CHANGEVARIABLEBYLOCAL",
    [DATA_SHOWVARIABLE] = "DATA_SHOWVARIABLE",
    [DATA_HIDEVARIABLE] = "DATA_HIDEVARIABLE",

    [INNER_LOOPJUMP] = "INNER_LOOPJUMP",
    [CONTROL_CREATE_CLONE_OF] = "CONTROL_CREATE_CLONE_OF",
    [CONTROL_WAIT] = "CONTROL_WAIT",
    [CONTROL_REPEAT] = "CONTROL_REPEAT",
    [CONTROL_FOREVER] = "CONTROL_FOREVER",
    [CONTROL_IF] = "CONTROL_IF",
    [CONTROL_IF_ELSE] = "CONTROL_IF_ELSE",
    [CONTROL_WAIT_UNTIL] = "CONTROL_WAIT_UNTIL",
    [CONTROL_REPEAT_UNTIL] = "CONTROL_REPEAT_UNTIL",
    [CONTROL_CREATE_CLONE_OF_MENU] = "CONTROL_CREATE_CLONE_OF_MENU",
    [CONTROL_DELETE_THIS_CLONE] = "CONTROL_DELETE_THIS_CLONE",
    [CONTROL_STOP] = "CONTROL_STOP",
    [INNER_JUMPIF] = "INNER_JUMPIF",
    [INNER_JUMPIFNOT] = "INNER_JUMPIFNOT",
    [INNER_JUMP] = "INNER_JUMP",

    [INNER__GLIDEITERATION] = "INNER__GLIDEITERATION",

    [MOTION_MOVESTEPS] = "MOTION_MOVESTEPS",
    [MOTION_TURNRIGHT] = "MOTION_TURNRIGHT",
    [MOTION_TURNLEFT] = "MOTION_TURNLEFT",
    [MOTION_GOTO] = "MOTION_GOTO",
    [MOTION_GOTO_MENU] = "MOTION_GOTO_MENU",
    [MOTION_GOTOXY] = "MOTION_GOTOXY",
    [MOTION_GLIDETO] = "MOTION_GLIDETO",
    [MOTION_GLIDETO_MENU] = "MOTION_GLIDETO_MENU",
    [MOTION_GLIDESECSTOXY] = "MOTION_GLIDESECSTOXY",
    [MOTION_POINTINDIRECTION] = "MOTION_POINTINDIRECTION",
    [MOTION_POINTTOWARDS] = "MOTION_POINTTOWARDS",
    [MOTION_POINTTOWARDS_MENU] = "MOTION_POINTTOWARDS_MENU",
    [MOTION_CHANGEXBY] = "MOTION_CHANGEXBY",
    [MOTION_SETX] = "MOTION_SETX",
    [MOTION_CHANGEYBY] = "MOTION_CHANGEYBY",
    [MOTION_SETY] = "MOTION_SETY",
    [MOTION_IFONEDGEBOUNCE] = "MOTION_IFONEDGEBOUNCE",
    [MOTION_SETROTATIONSTYLE] = "MOTION_SETROTATIONSTYLE",

    [LOOKS_SAY] = "LOOKS_SAY",
    [LOOKS_THINKFORSECS] = "LOOKS_THINKFORSECS",
    [LOOKS_THINK] = "LOOKS_THINK",
    [LOOKS_SWITCHCOSTUMETO] = "LOOKS_SWITCHCOSTUMETO",
    [LOOKS_NEXTCOSTUME] = "LOOKS_NEXTCOSTUME",
    [LOOKS_SWITCHBACKDROPTO] = "LOOKS_SWITCHBACKDROPTO",
    [LOOKS_BACKDROPS] = "LOOKS_BACKDROPS",
    [LOOKS_NEXTBACKDROP] = "LOOKS_NEXTBACKDROP",
    [LOOKS_CHANGESIZEBY] = "LOOKS_CHANGESIZEBY",
    [LOOKS_SETSIZETO] = "LOOKS_SETSIZETO",
    [LOOKS_CHANGEEFFECTBY] = "LOOKS_CHANGEEFFECTBY",
    [LOOKS_SETEFFECTTO] = "LOOKS_SETEFFECTTO",
    [LOOKS_CLEARGRAPHICEFFECTS] = "LOOKS_CLEARGRAPHICEFFECTS",
    [LOOKS_SHOW] = "LOOKS_SHOW",
    [LOOKS_HIDE] = "LOOKS_HIDE",
    [LOOKS_GOTOFRONTBACK] = "LOOKS_GOTOFRONTBACK",
    [LOOKS_GOFORWARDBACKWARDLAYERS] = "LOOKS_GOFORWARDBACKWARDLAYERS",

    [INNER__WAITITERATION] = "INNER__WAITITERATION",

    [INNER_DEBUGSTATEMENT] = "INNER_DEBUGSTATEMENT",
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

enum SCRATCH_continueStatus SCRATCH_processBlock(struct SCRATCH_sprite* sprite, struct SCRATCH_thread* thread, const pixel* imageTable[]) {
    struct SCRATCH_data stack[STACKMAX];
    int stackIndex = 0;
    enum SCRATCH_opcode operation;
    while (true) {
        operation = code[thread->programCounter++];
        enum SCRATCH_continueStatus status;
        machineLog("%s\n", SCRATCH_opcode_names[operation]);
        switch (operation) {
            #include "opcodeImpl.h"
        }
        if (operation > INNER_PARTITION_BEGINSTATEMENTS) return status; // A block has completed
    }
}

void SCRATCH_processThread(struct SCRATCH_sprite* sprite, struct SCRATCH_thread* thread, const pixel* imageTable[]) {
    enum SCRATCH_continueStatus status = SCRATCH_continue;
    while (status == SCRATCH_continue) {
        status = SCRATCH_processBlock(sprite, thread, imageTable);
    }
}

int SCRATCH_visitAllThreads(struct SCRATCH_sprite** sprites, int spriteCount, const pixel* imageTable[]) {
    int activeThreadCount = 0;
    for (int i = 0; i < spriteCount; i++) {
        struct SCRATCH_sprite* sprite = sprites[i];
        for (int ii = 0; ii < sprite->base.threadCount; ii++) {
            if (sprite->threads[ii].active) {
                SCRATCH_processThread(sprite, &sprite->threads[ii], imageTable);
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

struct SCRATCH_rect getRect(struct SCRATCH_sprite* s, const pixel* imageTable[]) {
    struct SCRATCH_rect rect;
    rect.x = s->base.x.halves.high;
    rect.y = s->base.y.halves.high;
    struct image* i = getImage(imageTable, s->base.id, s->base.costumeIndex);
    rect.width = i->widthRatio * SCRATCHWIDTH / 255;
    rect.height = i->heightRatio * SCRATCHWIDTH / 255;
    rect.x -= rect.width / 2;
    rect.y -= rect.height / 2;
    return rect;
}

bool rectsCollide(struct SCRATCH_rect r1, struct SCRATCH_rect r2) {
    bool collides = false;
    for (int i = 0; i < 4; i++) {
        int x = r1.x;
        int y = r1.y;
        if (i & 1) x += r1.width;
        if (i & 2) y += r1.height;
        collides |= (x > r2.x && x < r2.x + r2.width) && (y > r2.y && y < r2.y + r2.height);
    }
    return collides;
}
