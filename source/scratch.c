#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <sys/param.h>
#include <stdio.h>
#include "scratch.h"
#include "ioFuncs.h"
#include "globals.h"
#include "programData.h"

const char* SCRATCH_opcode_names[INNER_DEBUGSTATEMENT + 1] = {
    [EVENT_BROADCAST] = "EVENT_BROADCAST",
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
    [INNER_PUSHID] = "INNER_PUSHID",

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
    [DATA_SHOWVARIABLE] = "DATA_SHOWVARIABLE",
    [DATA_HIDEVARIABLE] = "DATA_HIDEVARIABLE",

    [INNER_LOOPJUMP] = "INNER_LOOPJUMP",
    [CONTROL_CREATE_CLONE_OF] = "CONTROL_CREATE_CLONE_OF",
    [CONTROL_WAIT] = "CONTROL_WAIT",
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
    [LOOKS_SAYFORSECS] = "LOOKS_SAYFORSECS",
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

void clearEvents(int eventCount) {
    for (int i = 0; i < eventCount; i++) {
        events[i] = false;
    }
}

// scratch's rule is to keep sprites in frame by a fixed number of pixels in each direction.
void keepInFrame(struct SCRATCH_spriteContext* context, struct SCRATCH_sprite* sprite) {
    struct image* i = getImage(context, sprite);
    int pixelWidth = (i->widthRatio * SCRATCHWIDTH * sprite->base.size) / (SIZERATIO * 255);
    int pixelHeight = (i->heightRatio * SCRATCHHEIGHT * sprite->base.size) / (SIZERATIO * 255);
    int allowedOffscreenX = -(pixelWidth / 2) - 1;
    int allowedOffscreenY = -(pixelHeight / 2) - 1;
    int left = -(SCRATCHWIDTH / 2) - allowedOffscreenX;
    int right = (SCRATCHWIDTH / 2) + allowedOffscreenX;
    int bottom = -(SCRATCHHEIGHT / 2) - allowedOffscreenY;
    int top = (SCRATCHHEIGHT / 2) + allowedOffscreenY;
    if (sprite->base.x.halves.high < left) sprite->base.x.halves.high = left;
    if (sprite->base.x.halves.high > right) sprite->base.x.halves.high = right;
    if (sprite->base.y.halves.high < bottom) sprite->base.y.halves.high = bottom;
    if (sprite->base.y.halves.high > top) sprite->base.y.halves.high = top;
}

void keepInStage(struct SCRATCH_spriteContext* context, struct SCRATCH_sprite* sprite) {
    const int pixelMargin = 10;
    struct image* i = getImage(context, sprite);
    int pixelWidth = (i->widthRatio * SCRATCHWIDTH * sprite->base.size) / (SIZERATIO * 255);
    int pixelHeight = (i->heightRatio * SCRATCHHEIGHT * sprite->base.size) / (SIZERATIO * 255);
    int allowedOffscreenX = (pixelWidth / 2) - pixelMargin;
    int allowedOffscreenY = (pixelHeight / 2) - pixelMargin;
    int left = -(SCRATCHWIDTH / 2) - allowedOffscreenX;
    int right = (SCRATCHWIDTH / 2) + allowedOffscreenX;
    int bottom = -(SCRATCHHEIGHT / 2) - allowedOffscreenY;
    int top = (SCRATCHHEIGHT / 2) + allowedOffscreenY;
    if (sprite->base.x.halves.high < left) sprite->base.x.halves.high = left;
    if (sprite->base.x.halves.high > right) sprite->base.x.halves.high = right;
    if (sprite->base.y.halves.high < bottom) sprite->base.y.halves.high = bottom;
    if (sprite->base.y.halves.high > top) sprite->base.y.halves.high = top;
}

const char* hatTable[] = {
    "when key pressed",
    "when I receive message",
    "when backdrop changes to",
    "when I start as clone",
    "when flag clicked",
    "when this sprite clicked",
    "when loudness >"
};

enum SCRATCH_continueStatus SCRATCH_processBlock(struct SCRATCH_spriteContext* context, struct SCRATCH_thread* thread, uint8_t* code) {
    struct SCRATCH_data stack[STACKMAX];
    int stackIndex = 0;
    enum SCRATCH_opcode operation;
    struct SCRATCH_sprite* sprite = context->sprites[context->currentIndex];
    struct SCRATCH_sprite** sprites = context->sprites;
    enum SCRATCH_opcode watchValue = -1;
    while (true) {
        operation = code[thread->programCounter++];
        enum SCRATCH_continueStatus status;
        const char* opcodeName = SCRATCH_opcode_names[operation];
        bool loggingCondition = sprite->base.id == 1 && (thread -sprite->threads) == 4;
            if (loggingCondition) {
            printf("%d, %d\n", operation, watchValue);
            machineLog("id: %d, index: %d, thread: %d, hat: %s, ", sprite->base.id, context->currentIndex, thread - (sprite->threads), hatTable[thread->base.startEvent]);
            if (opcodeName == NULL) machineLog("opcode: %d\n\r", operation);
            else machineLog("opcode: %s\n\r", SCRATCH_opcode_names[operation]);
            if (operation == watchValue) {
                raise(SIGTRAP);
            }
        }
        switch (operation) {
            #include "opcodeImpl.h"
        }
        if (status == SCRATCH_killSprite || operation > INNER_PARTITION_BEGINSTATEMENTS) {
            return status;
        }
    }
}

enum SCRATCH_continueStatus SCRATCH_processThread(struct SCRATCH_spriteContext* context, struct SCRATCH_thread* thread, uint8_t* code) {
    bool executedRealBlock = false;
    enum SCRATCH_continueStatus status = SCRATCH_continue;
    while (status == SCRATCH_continue || executedRealBlock == false) {
        status = SCRATCH_processBlock(context, thread, code);
        if (status != SCRATCH_yieldLogic) executedRealBlock = true;
    }
    return status;
}

int SCRATCH_visitAllThreads(struct SCRATCH_spriteContext* context, uint8_t* code) {
    int activeThreadCount = 0;
    for (int i = 0; i < context->spriteCount; i++) {
        context->currentIndex = i;
        for (int ii = 0; ii < context->sprites[i]->base.threadCount; ii++) {
            if (!context->sprites[i]->threads[ii].active) continue;
            enum SCRATCH_continueStatus status = SCRATCH_processThread(context, &context->sprites[i]->threads[ii], code);
            if (status == SCRATCH_continue || status == SCRATCH_yieldGeneric || status == SCRATCH_yieldLogic) {
                activeThreadCount++;
            }
            else if (status == SCRATCH_killOtherThreads || status == SCRATCH_killAllThreads) {
                for (int iii = 0; iii < context->sprites[i]->base.threadCount; iii++) {
                    if (iii == ii && status == SCRATCH_killOtherThreads) continue;
                    context->sprites[i]->threads[iii].active = false;
                }
                goto nextSprite;
            }
            if (status == SCRATCH_killSprite) {
                goto nextSprite;
            }
        }
nextSprite:
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
    for (int i = 0; i < header.variableCount; i++) {
        spriteChunk->variables[i].type = SCRATCH_NUMBER;
        spriteChunk->variables[i].data.number.i = 0;
    }
    spriteChunk->talkingString = NULL;
    return spriteChunk;
}

void SCRATCH_freeSprites(struct SCRATCH_spriteContext* context) {
    for (int i = 0; i < context->spriteCount; i++) {
        free(context->sprites[i]);
    }
}

void SCRATCH_initThread(struct SCRATCH_thread* thread, struct SCRATCH_threadHeader h) {
    thread->programCounter = h.entryPoint;
    thread->active = false;
    thread->loopCounterStackIndex = 0;
    thread->base = h;
}

bool SCRATCH_addSprite(struct SCRATCH_spriteContext* context, struct SCRATCH_sprite* sprite) {
    if (context->spriteCount >= SPRITEMAX) return false;
    else {
        context->sprites[context->spriteCount++] = sprite;
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

void SCRATCH_wakeSprites(struct SCRATCH_spriteContext* context) {
    for (int i = 0; i < context->spriteCount; i++) {
        struct SCRATCH_sprite* s = context->sprites[i];
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

struct SCRATCH_rect getRect(struct SCRATCH_spriteContext* context, struct SCRATCH_sprite* operand) {
    struct SCRATCH_rect rect;
    struct SCRATCH_sprite* s;
    if (operand == NULL) s = context->sprites[context->currentIndex];
    else s = operand;
    rect.x = s->base.x.halves.high;
    rect.y = -s->base.y.halves.high;
    struct image* i = getImage(context, s);
    rect.width = i->widthRatio * SCRATCHWIDTH / 255;
    rect.height = i->heightRatio * SCRATCHHEIGHT / 255;
    rect.width = (rect.width * s->base.size) / SIZERATIO;
    rect.height = (rect.height * s->base.size) / SIZERATIO;
    rect.x -= i->xRotationCenter * s->base.size / SIZERATIO;
    rect.y -= i->yRotationCenter * s->base.size / SIZERATIO;
    return rect;
}

struct SCRATCH_rect getIntersection(struct SCRATCH_rect r1, struct SCRATCH_rect r2) {
    int left = MAX(r1.x, r2.x);
    int right = MIN(r1.x + r1.width, r2.x + r2.width);
    int top = MAX(r1.y, r2.y);
    int bottom = MIN(r1.y + r1.height, r2.y + r2.height);
    struct SCRATCH_rect intersection = {
        left,
        top,
        right - left,
        bottom - top
    };
    return intersection;
}

bool rectsCollide(struct SCRATCH_rect r1, struct SCRATCH_rect r2) {
    struct SCRATCH_rect intersection = getIntersection(r1, r2);
    return (intersection.width > 0 && intersection.height > 0);
}

/*
 * NUMBER
 * DEGREES
 * BOOL
 * STRING
 * STATICSTRING
 */

#define pair(x, y) (x << 3 | y)
// TODO: Thorough V8 engine audit to get exactly-accurate conversions. For now: only %d and %d.%d conversions.
struct SCRATCH_data cast(struct SCRATCH_data d, enum SCRATCH_fieldType type, char* stringBuffer) {
    static char* boolStrings[] = {
        "false",
        "true"
    };
    if (d.type == SCRATCH_UNINIT) {
        machineLog("WARNING: scratch data value uninitialized. Likely a variable that was never set.\n");
        fflush(stdout);
        return d;
    }
    if (d.type == type) return d;
    int combination = pair(type, d.type);
    switch (combination) {
        case pair(SCRATCH_STRING, SCRATCH_NUMBER):
            sprintf(stringBuffer, "%f", (float)d.data.number.halves.high + d.data.number.halves.low / (float)(UINT16_MAX + 1));
            d.data.string = stringBuffer;
            d.type = SCRATCH_STRING;
            return d;
        case pair(SCRATCH_STRING, SCRATCH_DEGREES):
            d = cast(d, SCRATCH_NUMBER, stringBuffer);
            d = cast(d, SCRATCH_STRING, stringBuffer);
            return d;
        case pair(SCRATCH_STRING, SCRATCH_BOOL):
            d.data.string = boolStrings[d.data.boolean];
            d.type = SCRATCH_STATICSTRING;
            return d;
        case pair(SCRATCH_NUMBER, SCRATCH_DEGREES):
            d.data.number.i = (uint64_t)d.data.degrees * 360 >> 16;
            d.type = SCRATCH_NUMBER;
            return d;
        case pair(SCRATCH_NUMBER, SCRATCH_BOOL):
            d.data.number.i = d.data.boolean;
            d.type = SCRATCH_NUMBER;
            return d;
        case pair(SCRATCH_NUMBER, SCRATCH_STRING):
            float f = 0;
            sscanf(d.data.string, "%f", &f);
            d.data.number.i = (f * (float)(UINT16_MAX + 1));
            d.type = SCRATCH_NUMBER;
            return d;
        case pair(SCRATCH_DEGREES, SCRATCH_NUMBER):
            uint64_t newDegrees = (uint64_t)d.data.number.i << 16;
            newDegrees /= 360;
            // no-op. Good style? I don't know.
            d.data.degrees = newDegrees;
            d.type = SCRATCH_DEGREES;
            return d;
        case pair(SCRATCH_DEGREES, SCRATCH_BOOL):
            d.data.degrees = d.data.boolean;
            d.type = SCRATCH_DEGREES;
            return d;
        case pair(SCRATCH_DEGREES, SCRATCH_STRING):
            d = cast(d, SCRATCH_NUMBER, NULL);
            d = cast(d, SCRATCH_DEGREES, NULL);
            return d;
        default:
            return d;
        // Scratch has no truthiness; All booleans are derived from an expression producing one. There is no cast from something to bool.
    }
}
