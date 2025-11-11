// This file is inserted in scratch.c as the body for a switch statement. It is the right combination of size and simple layout that it seemed a good target for factoring out.

#define ERROR() // Nothing for now
#define INTERPRET_AS(type, value) *(type*)&(value)
#define POPNUMBER() stack[--stackIndex]
#define POPBOOL() stack[--stackIndex]
#define POPID() stack[--stackIndex]
#define POPDEGREES() stack[--stackIndex]
#define POPDATA() stack[--stackIndex]
#define PUSHNUMBER(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_NUMBER, {.number.halves.high=value}};
#define PUSHFRACTION(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_NUMBER, {.number.i=value}};
#define PUSHID(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_ID, {.number.i=value}};
#define PUSHBOOL(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_BOOL, {.boolean=value}};
#define PUSHDATA(value) stack[stackIndex++] = value;
#define PUSHTEXT(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_STRING, {.string=NULL}};
#define PUSHDEGREES(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_DEGREES, {.degrees=value}};

case INNER_PARTITION_BEGINLOOPCONTROL: {
    ERROR();
    break;
}
case INNER_LOOPINIT: {
    thread->loopCounterStack[thread->loopCounterStackIndex] = 0;
    thread->loopCounterStackIndex++;
    status = SCRATCH_continue;
    break;
}
case INNER_LOOPINCREMENT: {
    thread->loopCounterStack[thread->loopCounterStackIndex-1] += 1;
    status = SCRATCH_continue;
    break;
}
case INNER_JUMPIFREPEATDONE: {
    uint16_t jumpTo = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof(jumpTo);
    struct SCRATCH_data toMatch = POPNUMBER();
    machineLog("stack index: %d\n", thread->loopCounterStackIndex);
    machineLog("stack value: %d\n", thread->loopCounterStack[thread->loopCounterStackIndex-1]);
    if (thread->loopCounterStack[thread->loopCounterStackIndex-1] >= toMatch.data.number.halves.high) {
        thread->loopCounterStackIndex--;
        thread->programCounter = jumpTo;
    }
    status = SCRATCH_continue;
    break;
}
case INNER_PARTITION_BEGINEXPRESSIONS: {
    // Not to be implemented
    ERROR();
    break;
}
case SENSING_ANSWER: {
    // TODO
    // push a string to the stack
    status = SCRATCH_continue;
    break;
}
case SENSING_MOUSEDOWN: {
    PUSHBOOL(false);
    status = SCRATCH_continue;
    break;
}
case SENSING_MOUSEX: {
    PUSHNUMBER(0);
    status = SCRATCH_continue;
    break;
}
case SENSING_MOUSEY: {
    PUSHNUMBER(0);
    status = SCRATCH_continue;
}
case SENSING_KEYPRESSED: {
    struct SCRATCH_data keyIndex = POPNUMBER();
    PUSHBOOL(inputState[keyIndex.data.number.halves.high]);
    status = SCRATCH_continue;
    break;
}
case SENSING_LOUDNESS: {
    PUSHNUMBER(0);
    status = SCRATCH_continue;
}
case SENSING_TIMER: {
    // TODO
    // push the value of the global timer
}
case SENSING_CURRENT: {
    // TODO
    // Get enumerated request type from (uint16_t) code; push 2025 for year, (whichever day that was) for day, and 0 for rest
}
case SENSING_DAYSSINCE2000: {
    // TODO
    // push however many days it was from jan 1 2000 to jan 1 2025
}
case SENSING_USERNAME: {
    // TODO
    // push global username string
}
case INNER_FETCHINPUT: {
    uint16_t toFetch = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof(toFetch);
    PUSHBOOL(inputState[toFetch]);
    status = SCRATCH_continue;
    break;
}
case INNER_FETCHPOSITION: {
    int16_t value = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof value;
    if (value == -1) { // random position
        PUSHNUMBER(rand() % 500 - 250);
        PUSHNUMBER(rand() % 400 - 200);
    }
    else if (value == -2) { // mouse position (not handled for this VM)
        PUSHNUMBER(0);
        PUSHNUMBER(0);
    }
    else { // a sprite's position
        struct SCRATCH_sprite* s = sprites[value];
        PUSHFRACTION(s->base.y.i);
        PUSHFRACTION(s->base.x.i);
    }
    status = SCRATCH_continue;
    break;
}
case INNER_FETCHVAR: {
    int16_t spriteOperandIndex = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof spriteOperandIndex;
    int16_t varIndex = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof spriteOperandIndex;
    struct SCRATCH_sprite* spriteOperand;
    if (spriteOperandIndex == -1) {
        spriteOperand = sprite;
    }
    else {
        spriteOperand = sprites[spriteOperandIndex];
    }
    PUSHDATA(spriteOperand->variables[varIndex]);
    status = SCRATCH_continue;
    break;
}
case MOTION_XPOSITION: {
    int16_t spriteOperandIndex = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof spriteOperandIndex;
    struct SCRATCH_sprite* spriteOperand;
    if (spriteOperandIndex == -1) {
        spriteOperand = sprite;
    }
    else {
        spriteOperand = sprites[spriteOperandIndex];
    }
    PUSHFRACTION(spriteOperand->base.x.i);
    status = SCRATCH_continue;
    break;
}
case MOTION_YPOSITION: {
    int16_t spriteOperandIndex = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof spriteOperandIndex;
    struct SCRATCH_sprite* spriteOperand;
    if (spriteOperandIndex == -1) {
        spriteOperand = sprite;
    }
    else {
        spriteOperand = sprites[spriteOperandIndex];
    }
    PUSHFRACTION(spriteOperand->base.y.i);
    status = SCRATCH_continue;
    break;
}
case MOTION_DIRECTION: {
    int16_t spriteOperandIndex = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof spriteOperandIndex;
    struct SCRATCH_sprite* spriteOperand;
    if (spriteOperandIndex == -1) {
        spriteOperand = sprite;
    }
    else {
        spriteOperand = sprites[spriteOperandIndex];
    }
    PUSHDEGREES(spriteOperand->base.rotation);
    status = SCRATCH_continue;
    break;
}
case LOOKS_COSTUME: {
    int16_t spriteOperandIndex = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof spriteOperandIndex;
    struct SCRATCH_sprite* spriteOperand;
    if (spriteOperandIndex == -1) {
        spriteOperand = sprite;
    }
    else {
        spriteOperand = sprites[spriteOperandIndex];
    }
    PUSHID(spriteOperand->base.costumeIndex);
    status = SCRATCH_continue;
    break;
}
case LOOKS_SIZE: {
    // TODO
    // push the size of the sprite specified by (uint16_6) code
}
case LOOKS_COSTUMENUMBERNAME: {
    int16_t option = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof option;
    // number
    if (option == 0) {
        PUSHNUMBER(sprite->base.costumeIndex);
    }
    else {
        PUSHTEXT(getImage(imageTable, sprite->base.id, sprite->base.costumeIndex)->name);
    }
    status = SCRATCH_continue;
    break;
}
case LOOKS_BACKDROPNUMBERNAME: {
    // TODO
    break; // I need to look into this one
}
case SENSING_TOUCHINGOBJECT: {
    // TODO
    // push whether the current sprite is touching the sprite specified by (uint16_t) code
}
case SENSING_TOUCHINGOBJECTMENU: {
    ERROR(); // unused
    break;
}
case SENSING_TOUCHINGCOLOR: {
    ERROR(); // unused
    break;
}
case SENSING_COLORISTOUCHINGCOLOR: {
    ERROR() // unused
    break;
}
case SENSING_DISTANCETO: {
    // TODO
    // push the distance to the sprite specified by (uint16_6) code
}
case SENSING_DISTANCETOMENU: {
    ERROR(); // unused
    break;
}
case SENSING_ASKANDWAIT: {
    // TODO
    // set the talking string (variable 0) of the current sprite to the string specified by the top of stack
    // immediately push empty string
}
case SENSING_KEYOPTIONS: {
    // TODO
    // I have to look into this one
}
case SENSING_SETDRAGMODE: {
    ERROR(); // unused
    break;
}
case SENSING_RESETTIMER: {
    // TODO
    // set the value of the global timer to 0
}
case SENSING_OF: {
    ERROR(); // unused
    break;
}
case SENSING_OF_OBJECT_MENU: {
    ERROR(); // unused
    break;
}
case INNER_PUSHNUMBER: {
    scaledInt32 field = INTERPRET_AS(scaledInt32, code[thread->programCounter]);
    thread->programCounter += sizeof(field);
    PUSHFRACTION(field.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_ADD: {
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
    scaledInt32 result = {.i =op1.data.number.i + op2.data.number.i};
    PUSHFRACTION(result.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_SUBTRACT: {
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
    scaledInt32 result = {.i = op1.data.number.i - op2.data.number.i};
    PUSHFRACTION(result.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_MULTIPLY: {
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
    PUSHFRACTION(op1.data.number.i * op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_DIVIDE: {
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
    PUSHFRACTION(op1.data.number.i / op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_RANDOM: {
    struct SCRATCH_data high = POPNUMBER();
    struct SCRATCH_data low = POPNUMBER();
    scaledInt32 lowValue = low.data.number;
    scaledInt32 highValue = high.data.number;
    PUSHNUMBER(rand() % highValue.halves.high + lowValue.halves.high);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_GT: {
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
    PUSHBOOL(op1.data.number.i > op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_LT: {
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
    PUSHBOOL(op1.data.number.i < op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_EQUALS: {
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
    PUSHBOOL(op1.data.number.i == op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case INNER_GE: {
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
    PUSHBOOL(op1.data.number.i >= op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case INNER_LE: {
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
    PUSHBOOL(op1.data.number.i <= op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_AND: {
    struct SCRATCH_data op2 = POPBOOL();
    struct SCRATCH_data op1 = POPBOOL();
    PUSHBOOL(op1.data.boolean && op2.data.boolean);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_OR: {
    struct SCRATCH_data op2 = POPBOOL();
    struct SCRATCH_data op1 = POPBOOL();
    PUSHBOOL(op1.data.boolean || op2.data.boolean);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_NOT: {
    struct SCRATCH_data op1 = POPBOOL();
    PUSHBOOL(!op1.data.boolean);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_JOIN: {
    // TODO
    // pop two strings and join them
}
case OPERATOR_LETTER_OF: {
    // TODO
    // pop string, pop index, push string[index]
}
case OPERATOR_LENGTH: {
    // TODO
    // pop string, push length
}
case OPERATOR_CONTAINS: {
    // TODO
    // pop string, pop substring, push `substring in string`
}
case OPERATOR_MOD: {
    // TODO
    break; // I need to look into this (default number in scratch is float, so I need to figure out what the behavior is)
}
case OPERATOR_ROUND: {
    // TODO
    // pop number, push rounded (not banker's rounding)
}
case OPERATOR_MATHOP: {
    // TODO
    break;
}
case INNER_DEBUGEXPRESSION: {
    machineLog("DEBUG\n");
    status = SCRATCH_yieldGeneric;
    break;
}
case INNER_PARTITION_BEGINSTATEMENTS: {
    ERROR(); // unused
    break;
}
case DATA_SETVARIABLETO: {
    int16_t spriteOperandIndex = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof spriteOperandIndex;
    int16_t varIndex = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof spriteOperandIndex;
    struct SCRATCH_sprite* spriteOperand;
    if (spriteOperandIndex == -1) {
        spriteOperand = sprite;
    }
    else {
        spriteOperand = sprites[spriteOperandIndex];
    }
    struct SCRATCH_data x = POPDATA();
    sprite->variables[varIndex] = x;
    status = SCRATCH_continue;
    break;
}
case DATA_CHANGEVARIABLEBY: {
    int16_t spriteOperandIndex = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof spriteOperandIndex;
    int16_t varIndex = INTERPRET_AS(int16_t, code[thread->programCounter]);
    thread->programCounter += sizeof spriteOperandIndex;
    struct SCRATCH_sprite* spriteOperand;
    if (spriteOperandIndex == -1) {
        spriteOperand = sprite;
    }
    else {
        spriteOperand = sprites[spriteOperandIndex];
    }
    struct SCRATCH_data x = POPDATA();
    sprite->variables[varIndex].data.number.i = x.data.number.i;
    status = SCRATCH_continue;
    break;
}
case INNER_CHANGEVARIABLEBYLOCAL: {
    uint16_t varIndex = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof varIndex;
    struct SCRATCH_data value = POPNUMBER();
    sprite->variables[varIndex].data.number.i += value.data.number.i;
    status = SCRATCH_continue;
    break;
}
case DATA_SHOWVARIABLE: {
    // TODO
    break;
}
case DATA_HIDEVARIABLE: {
    // TODO
    break;
}
case INNER_LOOPJUMP: {
    uint16_t to = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter = to;
    status = SCRATCH_yieldGeneric;
    break;
}
case CONTROL_CREATE_CLONE_OF: {
    uint16_t field = INTERPRET_AS(int16_t, code[thread->programCounter]);
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
    SCRATCH_wakeSprite(newSprite, 0, (union SCRATCH_eventInput) {0});
    if (!SCRATCH_addSprite(newSprite)) {
        free(newSprite);
    }
    status = SCRATCH_yieldGeneric;
    break;
}
case CONTROL_WAIT: {
    struct SCRATCH_data scaledSecs = POPNUMBER();
    uint16_t iterations = ((scaledSecs.data.number.i * FRAMESPERSEC) >> 16);
    thread->operationData.waitData = (struct SCRATCH_waitData) {
        .remainingIterations = iterations
    };
    status = SCRATCH_yieldGeneric;
    break;
}
case CONTROL_REPEAT: {
    // TODO
    break;
}
case CONTROL_FOREVER: {
    // TODO
    break;
}
case CONTROL_IF: {
    // TODO
    break;
}
case CONTROL_IF_ELSE: {
    // TODO
    break;
}
case CONTROL_WAIT_UNTIL: {
    // TODO
    break;
}
case CONTROL_REPEAT_UNTIL: {
    // TODO
    break;
}
case CONTROL_CREATE_CLONE_OF_MENU: {
    // TODO
    break;
}
case CONTROL_DELETE_THIS_CLONE: {
    // TODO
    status = SCRATCH_yieldGeneric;
    if (sprite == sprites[sprite->base.id]) {
        // This is not a clone, it is the original. Scratch does nothing in this case.
        break;
    }
    break;
}
case CONTROL_STOP: {
    thread->active = false;
    status = SCRATCH_yieldGeneric;
    break;
}
case INNER_JUMPIF: {
    uint16_t jumpTo = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof jumpTo;
    struct SCRATCH_data evaluand = POPBOOL();
    if (evaluand.data.boolean) {
        thread->programCounter = jumpTo;
    }
    status = SCRATCH_continue;
    break;
}
case INNER_JUMPIFNOT: {
    uint16_t jumpTo = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof jumpTo;
    struct SCRATCH_data evaluand = POPBOOL();
    if (!evaluand.data.boolean) {
        thread->programCounter = jumpTo;
    }
    status = SCRATCH_continue;
    break;
}
case INNER_JUMP: {
    uint16_t jumpTo = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof jumpTo;
    thread->programCounter = jumpTo;
    break;
}
case INNER__GLIDEITERATION: {
    sprite->base.x.i += thread->operationData.glideData.stepX.i;
    sprite->base.y.i += thread->operationData.glideData.stepY.i;
    thread->operationData.glideData.remainingIterations--;
    if (thread->operationData.glideData.remainingIterations <= 0) {
        sprite->base.x.halves.high = thread->operationData.glideData.targetX;
        sprite->base.y.halves.high = thread->operationData.glideData.targetY;
        return SCRATCH_yieldGeneric;
    }
    thread->programCounter--; // re-align program counter with this instruction so it runs again
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_MOVESTEPS: {
    struct SCRATCH_data steps = POPNUMBER();
    float rotation = sprite->base.rotation * degreeToRadian;

    int x = sin(rotation) * steps.data.number.halves.high;
    int y = cos(rotation) * steps.data.number.halves.high;
    sprite->base.x.halves.high += x;
    sprite->base.y.halves.high += y;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_TURNRIGHT: {
    struct SCRATCH_data degrees = POPDEGREES();
    sprite->base.rotation += degrees.data.degrees;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_TURNLEFT: {
    struct SCRATCH_data degrees = POPDEGREES();
    sprite->base.rotation -= degrees.data.degrees;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_GOTO: {
    ERROR(); // unused
    break;
}
case MOTION_GOTO_MENU: {
    ERROR(); // unused
    break;
}
case MOTION_GOTOXY: {
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
    sprite->base.x = op1.data.number;
    sprite->base.y = op2.data.number;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_GLIDETO: {
    ERROR(); // unused
    break;
}
case MOTION_GLIDETO_MENU: {
    ERROR(); // unused
    break;
}
case MOTION_GLIDESECSTOXY: {
    struct SCRATCH_data y = POPNUMBER();
    struct SCRATCH_data x = POPNUMBER();
    struct SCRATCH_data scaledSecs = POPNUMBER();
    scaledInt32 xDiff = {.i = x.data.number.i - sprite->base.x.i};
    scaledInt32 yDiff = {.i = y.data.number.i - sprite->base.y.i};
    uint16_t iterations = (scaledSecs.data.number.i * FRAMESPERSEC) >> 16;
    if (iterations == 0) {
        sprite->base.x = x.data.number;
        sprite->base.y = y.data.number;
        thread->operationData.glideData.remainingIterations = 0;
        return SCRATCH_continue;
    }
    thread->operationData.glideData = (struct SCRATCH_glideData) {
        .stepX = {.i = xDiff.i / iterations},
        .stepY = {.i = yDiff.i / iterations},
        .remainingIterations = iterations,
        .targetX = x.data.number.halves.high,
        .targetY = y.data.number.halves.high,
    };
    status = SCRATCH_continue;
    break;
}
case MOTION_POINTINDIRECTION: {
    struct SCRATCH_data degrees = POPDEGREES();
    sprite->base.rotation = degrees.data.degrees;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_POINTTOWARDS: {
    struct SCRATCH_data x = POPNUMBER();
    struct SCRATCH_data y = POPNUMBER();
    float direction = atan2(y.data.number.halves.high, x.data.number.halves.high);
    direction *= radianToDegree;
    sprite->base.rotation = (uint16_t) direction;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_POINTTOWARDS_MENU: {
    ERROR(); // unused
    break;
}
case MOTION_CHANGEXBY: {
    struct SCRATCH_data x = POPNUMBER();
    sprite->base.x.i += x.data.number.i;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_SETX: {
    struct SCRATCH_data x = POPNUMBER();
    sprite->base.x = x.data.number;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_CHANGEYBY: {
    struct SCRATCH_data y = POPNUMBER();
    sprite->base.y.i += y.data.number.i;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_SETY: {
    struct SCRATCH_data y = POPNUMBER();
    sprite->base.y = y.data.number;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_IFONEDGEBOUNCE: {
    // TODO
    // detect whether any amount of sprite is off edge, then if so flip the sprite's angle
}
case MOTION_SETROTATIONSTYLE: {
    uint16_t style = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof style;
    sprite->base.rotationStyle = style;
    status = SCRATCH_yieldGeneric;
    break;
}
case LOOKS_SAY: {
    // TODO
    // set the sprite's first variable to the string popped from the stack
}
case LOOKS_THINKFORSECS: {
    ERROR(); // unused
    break;
}
case LOOKS_THINK: {
    // TODO
    // set the sprite's first variable to the string popped from the stack
}
case LOOKS_SWITCHCOSTUMETO: {
    int16_t index = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof index;
    sprite->base.costumeIndex = index;
    status = SCRATCH_yieldGeneric;
    break;
}
case LOOKS_NEXTCOSTUME: {
    sprite->base.costumeIndex += 1;
    sprite->base.costumeIndex %= sprite->base.costumeMax;
    status = SCRATCH_yieldGeneric;
    break;
}
case LOOKS_SWITCHBACKDROPTO: {
    // TODO
    break;
}
case LOOKS_BACKDROPS: {
    // TODO
    break;
}
case LOOKS_NEXTBACKDROP: {
    // TODO
    break;
}
case LOOKS_CHANGESIZEBY: {
    // TODO
    break;
}
case LOOKS_SETSIZETO: {
    struct SCRATCH_data size = POPNUMBER();
    sprite->base.size = size.data.number.halves.high;
    status = SCRATCH_yieldGeneric;
    break;
}
case LOOKS_CHANGEEFFECTBY: {
    ERROR(); // unused
    break;
}
case LOOKS_SETEFFECTTO: {
    ERROR(); // unused
    break;
}
case LOOKS_CLEARGRAPHICEFFECTS: {
    ERROR(); // unused
    break;
}
case LOOKS_SHOW: {
    sprite->base.visible = true;
    status = SCRATCH_yieldGeneric;
    break;
}
case LOOKS_HIDE: {
    sprite->base.visible = false;
    status = SCRATCH_yieldGeneric;
    break;
}
case LOOKS_GOTOFRONTBACK: {
    ERROR(); // unused
    break;
}
case LOOKS_GOFORWARDBACKWARDLAYERS: {
    ERROR(); // unused
    break;
}
case INNER__WAITITERATION: {
    if (thread->operationData.waitData.remainingIterations <= 0) {
        return SCRATCH_yieldGeneric;
    }
    thread->operationData.waitData.remainingIterations--;
    thread->programCounter--;
    status = SCRATCH_yieldGeneric;
    break;
}
case INNER_DEBUGSTATEMENT: {
    machineLog("DEBUG\n");
    status = SCRATCH_yieldGeneric;
    break;
}
