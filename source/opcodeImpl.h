// This file is inserted in scratch.c as the body for a switch statement. It is the right combination of size and simple layout that it seemed a good target for factoring out.

#define INTMAX16 65535
#define ERROR() machineLog("something went wrong on line %d\n", __LINE__); exit(-1);
#define INTERPRET_AS(type, value) *(type*)&(value)
#define POPFRACTION() cast(stack[--stackIndex], SCRATCH_FRACTION, NULL)
#define POPWHOLENUMBER() cast(stack[--stackIndex], SCRATCH_WHOLENUMBER, NULL)
#define POPBOOL() cast(stack[--stackIndex], SCRATCH_BOOL, NULL)
#define POPID() ((int16_t)cast(stack[--stackIndex], SCRATCH_WHOLENUMBER, NULL).data.number.i)
#define POPDEGREES() cast(stack[--stackIndex], SCRATCH_DEGREES, NULL)
#define POPTEXT(buffer) cast(stack[--stackIndex], SCRATCH_STRING, buffer)
#define POPDATA() stack[--stackIndex]
#define PUSHFRACTION(value) stack[stackIndex++] = (struct SCRATCH_data) {{.number.i=value}, .type=SCRATCH_FRACTION}
#define PUSHWHOLENUMBER(value) stack[stackIndex++] = (struct SCRATCH_data) {{.wholeNumber=(int32_t)value}, .type=SCRATCH_WHOLENUMBER}
#define PUSHID(value) stack[stackIndex++] = (struct SCRATCH_data) {{.wholeNumber=(int32_t)value}, .type=SCRATCH_WHOLENUMBER}
#define PUSHBOOL(value) stack[stackIndex++] = (struct SCRATCH_data) {{.boolean=value}, .type=SCRATCH_BOOL}
#define PUSHDATA(value) stack[stackIndex++] = value
#define PUSHSTATICTEXT(value) stack[stackIndex++] = (struct SCRATCH_data) {{.string=(value)}, .type=SCRATCH_STATICSTRING}
#define PUSHTEXT(value) stack[stackIndex++] = (struct SCRATCH_data) {{.string=NULL}, .type=SCRATCH_STRING}
#define PUSHDEGREES(value) stack[stackIndex++] = (struct SCRATCH_data) {{.degrees=value}, .type=SCRATCH_DEGREES}

#define GETARGUMENT(type) (thread->programCounter = (thread->programCounter + 3) & ~3, thread->programCounter += sizeof(type), INTERPRET_AS(type, code[thread->programCounter - sizeof(type)]))

// # TODO: shortcut fewer (if any) opcodes; let each one evaluate properly as the engine specifies
// # TODO: all operations to do with a sprite can be prepended with a special opcode that selects a sprite and hoists it to the "sprite register" for the next opcode
// # TODO: several pseudo-sprites can be selected this way (random position, mouse position)

case INNER_PARTITION_BEGINEXPRESSIONS: {
    // Not to be implemented
    ERROR();
    break;
}
case INNER_PUSHFRAME: {
    uint16_t size = GETARGUMENT(uint16_t);
    SCRATCH_vectorExtend(&thread->threadLocals, size);
    status = SCRATCH_continue;
    break;
}
case INNER_POPFRAME: {
    uint16_t size = GETARGUMENT(uint16_t);
    SCRATCH_vectorRetract(&thread->threadLocals, size);
    status = SCRATCH_continue;
    break;
}
case INNER_GETFRAMEVAR: {
    uint16_t index = GETARGUMENT(uint16_t);
    PUSHDATA(*SCRATCH_vectorFromTop(&thread->threadLocals, index));
    status = SCRATCH_continue;
    break;
}
case INNER_SETFRAMEVAR: {
    uint16_t index = GETARGUMENT(uint16_t);
    struct SCRATCH_data data = POPDATA();
    *SCRATCH_vectorFromTop(&thread->threadLocals, index) = data;
    status = SCRATCH_continue;
    break;
}
case SENSING_ANSWER: {
    static char* nullString = "";
    PUSHSTATICTEXT(nullString);
    status = SCRATCH_continue;
    break;
}
case SENSING_MOUSEDOWN: {
    PUSHBOOL(false);
    status = SCRATCH_continue;
    break;
}
case SENSING_MOUSEX: {
    PUSHWHOLENUMBER(0);
    status = SCRATCH_continue;
    break;
}
case SENSING_MOUSEY: {
    PUSHWHOLENUMBER(0);
    status = SCRATCH_continue;
    break;
}
// TODO handle string
case SENSING_KEYPRESSED: {
    struct SCRATCH_data keyIndex = POPWHOLENUMBER();
    status = SCRATCH_continue;
    if (-1 == keyIndex.data.wholeNumber) {
        PUSHBOOL(false);
        break;
    }
    PUSHBOOL(inputState[keyIndex.data.wholeNumber]);
    break;
}
case SENSING_LOUDNESS: {
    PUSHFRACTION(0);
    status = SCRATCH_continue;
    break;
}
case SENSING_TIMER: {
    uint32_t millisSinceLastReset = getNow() - context->timerLastReset;
    millisSinceLastReset <<= 6;
    millisSinceLastReset /= 1000;
    millisSinceLastReset <<= 10;
    PUSHFRACTION(millisSinceLastReset);
    status = SCRATCH_continue;
    break;
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
    int16_t toFetch = GETARGUMENT(int16_t);
    PUSHBOOL(inputState[toFetch]);
    status = SCRATCH_continue;
    break;
}
// TODO get argument from stack and treat it more stringlike
case INNER_FETCHPOSITION: {
    int16_t value = GETARGUMENT(int16_t);
    if (value == -1) { // random position
        PUSHWHOLENUMBER(rand() % 500 - 250);
        PUSHWHOLENUMBER(rand() % 400 - 200);
    }
    else if (value == -2) { // mouse position (not handled for this VM)
        PUSHWHOLENUMBER(0);
        PUSHWHOLENUMBER(0);
    }
    else { // a sprite's position
        struct SCRATCH_sprite* s = sprites[value];
        PUSHFRACTION(s->base.x.i);
        PUSHFRACTION(s->base.y.i);
    }
    status = SCRATCH_continue;
    break;
}
// TODO get sprite operand from stack and treat it more stringlike
case INNER_FETCHVAR: {
    int16_t spriteOperandIndex = GETARGUMENT(int16_t);
    int16_t varIndex = GETARGUMENT(int16_t);
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
// TODO get sprite operand from stack and treat it more stringlike
case MOTION_XPOSITION: {
    int16_t spriteOperandIndex = GETARGUMENT(int16_t);
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
// TODO get sprite operand from stack and treat it more stringlike
case MOTION_YPOSITION: {
    int16_t spriteOperandIndex = GETARGUMENT(int16_t);
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
// TODO get sprite operand from stack and treat it more stringlike
case MOTION_DIRECTION: {
    int16_t spriteOperandIndex = GETARGUMENT(int16_t);
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
    break;
}
case LOOKS_SIZE: {
    // TODO
    // push the size of the sprite specified by (uint16_6) code
}
case LOOKS_COSTUMENUMBERNAME: {
    int16_t option = GETARGUMENT(int16_t);
    // number
    if (option == 0) {
        PUSHWHOLENUMBER(sprite->base.costumeIndex + 1);
    }
    else {
        char* name = getImage(context, sprite)->name;
        PUSHSTATICTEXT(name);
    }
    status = SCRATCH_continue;
    break;
}
case LOOKS_BACKDROPNUMBERNAME: {
    int16_t option = GETARGUMENT(int16_t);
    // number
    if (option == 0) {
        PUSHWHOLENUMBER(context->stage->base.costumeIndex + 1);
    }
    else {
        char* name = getImage(context, context->stage)->name;
        PUSHSTATICTEXT(name);
    }
    status = SCRATCH_continue;
    break;
}
case SENSING_TOUCHINGOBJECT: {
    int16_t target = GETARGUMENT(int16_t);
    // mouse pointer (unused)
    if (target == -1) {
        PUSHBOOL(false);
    }
    // edge
    else if (target == -2) {
        int left = -(SCRATCHWIDTH / 2);
        int right = (SCRATCHWIDTH / 2);
        int top = (SCRATCHHEIGHT / 2);
        int bottom = -(SCRATCHHEIGHT / 2);
        struct SCRATCH_rect myRect = getRect(context, NULL);
        bool touchingEdge = false;
        if      (myRect.x <= left) touchingEdge = true;
        else if (myRect.x + myRect.width >= right) touchingEdge = true;
        else if (myRect.y <= bottom) touchingEdge =  true;
        else if (myRect.y + myRect.height >= top) touchingEdge = true;
        PUSHBOOL(touchingEdge);
    }
    else {
        if (!sprite->base.visible) {
            PUSHBOOL(false);
            status = SCRATCH_continue;
            break;
        }
        struct SCRATCH_sprite* spriteOperand;
        struct SCRATCH_rect myRect = getRect(context, NULL);
        bool colliding = false;
        for (int i = 0; i < context->spriteCount; i++) {
            spriteOperand = context->sprites[i];
            if (spriteOperand->base.id != target) continue;
            if (!spriteOperand->base.visible) continue;
            struct SCRATCH_rect otherRect = getRect(context, spriteOperand);
            colliding |= rectsCollide(myRect, otherRect);
        }
        PUSHBOOL(colliding);
    }
    status = SCRATCH_continue;
    break;
}
case SENSING_TOUCHINGOBJECTMENU: {
    ERROR(); // unused
    break;
}
case SENSING_TOUCHINGCOLOR: {
    PUSHBOOL(false);
    status = SCRATCH_continue;
    break;
}
case SENSING_COLORISTOUCHINGCOLOR: {
    ERROR(); // unused
    break;
}
case SENSING_DISTANCETO: {
    int id = POPID();
    struct SCRATCH_sprite* spriteArgument = context->sprites[id];

    // don't bother with subpixel precision
    int x1 = sprite->base.x.halves.high;
    int y1 = sprite->base.y.halves.high;

    int x2 = spriteArgument->base.x.halves.high;
    int y2 = spriteArgument->base.y.halves.high;

    int xDist = x1 - x2;
    int yDist = y1 - y2;

    int hypot = sqrt(xDist * xDist + yDist * yDist);
    PUSHFRACTION(hypot);

    status = SCRATCH_continue;
    break;
}

case SENSING_DISTANCETOMENU: {
    ERROR(); // unused
    break;
}
case SENSING_ASKANDWAIT: {
    // This block is largely unused
    status = SCRATCH_continue;
    break;
}
case SENSING_KEYOPTIONS: {
    ERROR(); // unused
    break;
}
case SENSING_SETDRAGMODE: {
    ERROR(); // unused
    break;
}
case SENSING_RESETTIMER: {
    context->timerLastReset = getNow();
    status = SCRATCH_continue;
    break;
}
case SENSING_OF: {
    int16_t id = GETARGUMENT(int16_t);
    struct SCRATCH_sprite* spriteOperand = context->sprites[POPID()];
    switch (id) {
        case -1: // costume number
            PUSHWHOLENUMBER(spriteOperand->base.costumeIndex);
            break;
        case -2: // costume name
            char* name = getImage(context, spriteOperand)->name;
            PUSHSTATICTEXT(name);
            break;
        case -3: // volume
            PUSHWHOLENUMBER(0);
            break;
        case -4: // x
            PUSHFRACTION(spriteOperand->base.x.i);
            break;
        case -5: // y
            PUSHFRACTION(spriteOperand->base.y.i);
            break;
        case -6: // direction
            PUSHDEGREES(spriteOperand->base.rotation);
            break;
        case -7: // size
            PUSHFRACTION((spriteOperand->base.size << 16) / SIZERATIO);
            break;
        default: // variable
            PUSHDATA(spriteOperand->variables[id - 6]);
            
    }
    status = SCRATCH_continue;
    break;
}
case SENSING_OF_OBJECT_MENU: {
    ERROR(); // unused
    break;
}
case INNER_PUSHNUMBER: {
    scaledInt32 field = GETARGUMENT(scaledInt32);
    PUSHFRACTION(field.i);
    status = SCRATCH_continue;
    break;
}
case INNER_PUSHDEGREES: {
    uint32_t field = GETARGUMENT(uint32_t);
    PUSHDEGREES(field);
    status = SCRATCH_continue;
    break;
}
case INNER_PUSHTEXT: {
    char* ptr = (char*)&(code[thread->programCounter]);
    PUSHSTATICTEXT(ptr);
    while (code[thread->programCounter++] != 0);
    status = SCRATCH_continue;
    break;
}
case INNER_PUSHID: {
    int32_t field = GETARGUMENT(int16_t);
    PUSHID(field);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_ADD: {
    struct SCRATCH_data op2 = POPFRACTION();
    struct SCRATCH_data op1 = POPFRACTION();
    scaledInt32 result = {.i =op1.data.number.i + op2.data.number.i};
    PUSHFRACTION(result.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_SUBTRACT: {
    struct SCRATCH_data op2 = POPFRACTION();
    struct SCRATCH_data op1 = POPFRACTION();
    scaledInt32 result = {.i = op1.data.number.i - op2.data.number.i};
    PUSHFRACTION(result.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_MULTIPLY: {
    struct SCRATCH_data op2 = POPFRACTION();
    struct SCRATCH_data op1 = POPFRACTION();
    PUSHFRACTION(((int64_t)op1.data.number.i * op2.data.number.i) >> 16);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_DIVIDE: {
    struct SCRATCH_data op2 = POPFRACTION();
    struct SCRATCH_data op1 = POPFRACTION();
    PUSHFRACTION(((int64_t)op1.data.number.i << 16) / op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_RANDOM: {
    struct SCRATCH_data high = POPFRACTION();
    struct SCRATCH_data low = POPFRACTION();
    int32_t lowValue = low.data.number.i;
    int32_t highValue = high.data.number.i;
    int rangeSize = highValue - lowValue;
    uint64_t randomNumber = rand();
    PUSHFRACTION(lowValue + randomNumber * (rangeSize + 1) / ((uint64_t)RAND_MAX + 1));
    status = SCRATCH_continue;
    break;
}
case OPERATOR_GT: {
    struct SCRATCH_data op2 = POPFRACTION();
    struct SCRATCH_data op1 = POPFRACTION();
    PUSHBOOL((int32_t)op1.data.number.i > (int32_t)op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_LT: {
    struct SCRATCH_data op2 = POPFRACTION();
    struct SCRATCH_data op1 = POPFRACTION();
    PUSHBOOL((int32_t)op1.data.number.i < (int32_t)op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_EQUALS: {
    struct SCRATCH_data op2 = POPDATA();
    struct SCRATCH_data op1 = POPDATA();
    PUSHBOOL(equal(op1, op2));
    status = SCRATCH_continue;
    break;
}
case INNER_GE: {
    struct SCRATCH_data op2 = POPFRACTION();
    struct SCRATCH_data op1 = POPFRACTION();
    PUSHBOOL(op1.data.number.i >= op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case INNER_LE: {
    struct SCRATCH_data op2 = POPFRACTION();
    struct SCRATCH_data op1 = POPFRACTION();
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
    int16_t operationEnum = GETARGUMENT(int16_t);
    struct SCRATCH_data operand = POPFRACTION();
    
    // Convert scaledInt32 to float
    float floatValue = (float)operand.data.number.i / 65536.0f;
    float result = 0.0f;
    
    // Perform the math operation based on enum value
    switch (operationEnum) {
        case 0: // abs
            result = fabsf(floatValue);
            break;
        case 1: // floor
            result = floorf(floatValue);
            break;
        case 2: // ceil
            result = ceilf(floatValue);
            break;
        case 3: // sqrt
            result = sqrtf(floatValue >= 0.0f ? floatValue : 0.0f);
            break;
        case 4: // sin
            result = sinf(floatValue);
            break;
        case 5: // cos
            result = cosf(floatValue);
            break;
        case 6: // tan
            result = tanf(floatValue);
            break;
        case 7: // asin
            result = asinf(floatValue);
            break;
        case 8: // acos
            result = acosf(floatValue);
            break;
        case 9: // atan
            result = atanf(floatValue);
            break;
        case 10: // ln (natural log)
            result = logf(floatValue > 0.0f ? floatValue : 1.0f);
            break;
        case 11: // log (base 10)
            result = log10f(floatValue > 0.0f ? floatValue : 1.0f);
            break;
        case 12: // e^ (exp)
            result = expf(floatValue);
            break;
        case 13: // 10^
            result = powf(10.0f, floatValue);
            break;
        default:
            result = floatValue; // Unknown operation, return as-is
            break;
    }
    
    // Convert float back to scaledInt32
    int32_t scaledResult = (int32_t)(result * 65536.0f);
    PUSHFRACTION(scaledResult);
    status = SCRATCH_continue;
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
    int16_t spriteOperandIndex = GETARGUMENT(int16_t);
    int16_t varIndex = GETARGUMENT(int16_t);
    struct SCRATCH_sprite* spriteOperand;
    if (spriteOperandIndex == -1) {
        spriteOperand = sprite;
    }
    else {
        spriteOperand = sprites[spriteOperandIndex];
    }
    struct SCRATCH_data x = POPDATA();
    char debug[300];
    cast(x, SCRATCH_STRING, debug);
    spriteOperand->variables[varIndex] = x;
    status = SCRATCH_continue;
    break;
}
case DATA_CHANGEVARIABLEBY: {
    int16_t spriteOperandIndex = GETARGUMENT(int16_t);
    int16_t varIndex = GETARGUMENT(int16_t);
    struct SCRATCH_sprite* spriteOperand;
    if (spriteOperandIndex == -1) {
        spriteOperand = sprite;
    }
    else {
        spriteOperand = sprites[spriteOperandIndex];
    }
    struct SCRATCH_data x = POPDATA();
    spriteOperand->variables[varIndex].data.number.i += x.data.number.i;
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
case EVENT_BROADCAST: {
    int broadcastId = POPID();
    setEvent(ONMESSAGE, (union SCRATCH_eventInput){.message = broadcastId}, true);
    break;
}
case INNER_LOOPJUMP: {
    uint16_t to = GETARGUMENT(uint16_t);
    thread->programCounter = to;
    status = SCRATCH_yieldLogic;
    break;
}
case INNER_LOOPREPEATINIT: {
    uint16_t framePos = GETARGUMENT(uint16_t);
    struct SCRATCH_data target = POPWHOLENUMBER();
    uint16_t targetPos = framePos;
    uint16_t iterPos = framePos + 1;
    struct SCRATCH_data iter = (struct SCRATCH_data) {.data.wholeNumber=0, .type=SCRATCH_WHOLENUMBER};
    *SCRATCH_vectorFromTop(&thread->threadLocals, targetPos) = target;
    *SCRATCH_vectorFromTop(&thread->threadLocals, iterPos) = iter;
    status = SCRATCH_continue;
    break;
}
case INNER_LOOPREPEAT: {
    status = SCRATCH_continue;
    uint16_t framePos = GETARGUMENT(uint16_t);
    uint16_t breakTo = GETARGUMENT(uint16_t);
    uint16_t targetPos = framePos;
    uint16_t iterPos = framePos + 1;
    int32_t* iterations = &(SCRATCH_vectorFromTop(&thread->threadLocals, iterPos)->data.wholeNumber);
    int32_t* target = &(SCRATCH_vectorFromTop(&thread->threadLocals, targetPos)->data.wholeNumber);
    if (*iterations == *target) {
        thread->programCounter = breakTo;
        break;
    }
    ++*iterations;
    break;
}
case CONTROL_CREATE_CLONE_OF: {
    status = SCRATCH_continue;
    if (context->spriteCount >= SPRITEMAX) break;
    int field = POPID();
    struct SCRATCH_sprite* template;
    if (field == -1) {
        template = sprite;
    }
    else {
        template = sprites[field];
    }
    struct SCRATCH_sprite* newSprite = SCRATCH_cloneSprite(template);
    SCRATCH_wakeSprite(newSprite, ONCLONE, (union SCRATCH_eventInput) {0});
    if (!SCRATCH_addSprite(context, newSprite)) {
        SCRATCH_freeSprite(newSprite);
    }
    break;
}
case CONTROL_WAIT: {
    struct SCRATCH_data scaledSecs = POPFRACTION();
    uint16_t framePos = GETARGUMENT(uint16_t);
    uint16_t iterations = ((scaledSecs.data.number.i * FRAMESPERSEC) >> 16);
    *SCRATCH_vectorFromTop(&thread->threadLocals, framePos) = (struct SCRATCH_data){{.wholeNumber=iterations}, SCRATCH_WHOLENUMBER};
    status = SCRATCH_yieldGeneric;
    break;
}
case CONTROL_WAIT_UNTIL: {
    ERROR(); // unused
    break;
}
case CONTROL_CREATE_CLONE_OF_MENU: {
    ERROR();
    break;
}
case CONTROL_DELETE_THIS_CLONE: {
    status = SCRATCH_killSprite;
    if (sprite == sprites[sprite->base.id]) {
        break;
    }
    context->sprites[context->currentIndex] = context->sprites[context->spriteCount - 1];
    context->spriteCount -= 1;
    SCRATCH_freeSprite(sprite);
    sprite = NULL;
    break;
}
case CONTROL_STOP: {
    int id = POPID();
    if (id == 0) {
        thread->active = false;
        status = SCRATCH_killThread;
    }
    else if (id == 1) {
        thread->active = false;
        status = SCRATCH_killAllThreads;
    }
    else if (id == 2) {
        status = SCRATCH_killOtherThreads;
    }
    break;
}
case INNER_JUMPIF: {
    uint16_t jumpTo = GETARGUMENT(uint16_t);
    struct SCRATCH_data evaluand = POPBOOL();
    if (evaluand.data.boolean) {
        thread->programCounter = jumpTo;
    }
    status = SCRATCH_continue;
    break;
}
case INNER_JUMPIFNOT: {
    uint16_t jumpTo = GETARGUMENT(uint16_t);
    struct SCRATCH_data evaluand = POPBOOL();
    if (!evaluand.data.boolean) {
        thread->programCounter = jumpTo;
    }
    status = SCRATCH_continue;
    break;
}
case INNER_JUMP: {
    uint16_t jumpTo = GETARGUMENT(uint16_t);
    thread->programCounter = jumpTo;
    status = SCRATCH_continue;
    break;
}
case INNER_JUMPINDIRECT: {
    uint16_t jumpTo = POPID();
    thread->programCounter = jumpTo;
    status = SCRATCH_continue;
    break;
}
case INNER__GLIDEITERATION: {
    uint16_t programCounter = thread->programCounter;
    uint16_t framePos = GETARGUMENT(uint16_t);
    struct SCRATCH_data* iterationsRemaining = SCRATCH_vectorFromTop(&thread->threadLocals, framePos + 0);
    struct SCRATCH_data* xTarget = SCRATCH_vectorFromTop(&thread->threadLocals, framePos + 1);
    struct SCRATCH_data* yTarget = SCRATCH_vectorFromTop(&thread->threadLocals, framePos + 2);
    struct SCRATCH_data* xStep = SCRATCH_vectorFromTop(&thread->threadLocals, framePos + 3);
    struct SCRATCH_data* yStep = SCRATCH_vectorFromTop(&thread->threadLocals, framePos + 4);
    if (iterationsRemaining->data.wholeNumber <= 0) {
        sprite->base.x = xTarget->data.number;
        sprite->base.y = yTarget->data.number;
        keepInStage(context, sprite);
        status = SCRATCH_yieldGeneric;
        break;
    }
    sprite->base.x.i += xStep->data.number.i;
    sprite->base.y.i += yStep->data.number.i;
    iterationsRemaining->data.wholeNumber--;
    thread->programCounter = programCounter - 1; // re-align program counter with this instruction so it runs again
    keepInStage(context, sprite);
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_MOVESTEPS: {
    struct SCRATCH_data steps = POPFRACTION();
    float rotation = sprite->base.rotation * degreeToRadian;

    int x = sin(rotation) * steps.data.number.i;
    int y = cos(rotation) * steps.data.number.i;
    sprite->base.x.i += x;
    sprite->base.y.i += y;
    keepInStage(context, sprite);
    status = SCRATCH_continue;
    break;
}
case MOTION_TURNRIGHT: {
    struct SCRATCH_data degrees = POPDEGREES();
    sprite->base.rotation += degrees.data.degrees;
    status = SCRATCH_continue;
    break;
}
case MOTION_TURNLEFT: {
    struct SCRATCH_data degrees = POPDEGREES();
    sprite->base.rotation -= degrees.data.degrees;
    status = SCRATCH_continue;
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
    struct SCRATCH_data op2 = POPFRACTION();
    struct SCRATCH_data op1 = POPFRACTION();
    sprite->base.x = op1.data.number;
    sprite->base.y = op2.data.number;
    keepInStage(context, sprite);
    status = SCRATCH_continue;
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
    uint16_t framePos = GETARGUMENT(uint16_t);
    struct SCRATCH_data y = POPFRACTION();
    struct SCRATCH_data x = POPFRACTION();
    struct SCRATCH_data scaledSecs = POPFRACTION();
    scaledInt32 xDiff = {.i = x.data.number.i - sprite->base.x.i};
    scaledInt32 yDiff = {.i = y.data.number.i - sprite->base.y.i};
    uint16_t iterations = (scaledSecs.data.number.i * FRAMESPERSEC) >> 16;
    if (iterations == 0) {
        sprite->base.x = x.data.number;
        sprite->base.y = y.data.number;
        keepInStage(context, sprite);
        return SCRATCH_continue;
    }
    *SCRATCH_vectorFromTop(&thread->threadLocals, framePos + 0) = (struct SCRATCH_data) {{.wholeNumber=iterations}, SCRATCH_WHOLENUMBER};
    *SCRATCH_vectorFromTop(&thread->threadLocals, framePos + 1) = x;
    *SCRATCH_vectorFromTop(&thread->threadLocals, framePos + 2) = y;
    *SCRATCH_vectorFromTop(&thread->threadLocals, framePos + 3) = (struct SCRATCH_data) {{xDiff.i / iterations}, SCRATCH_FRACTION};
    *SCRATCH_vectorFromTop(&thread->threadLocals, framePos + 4) = (struct SCRATCH_data) {{yDiff.i / iterations}, SCRATCH_FRACTION};
    status = SCRATCH_continue;
    break;
}
case MOTION_POINTINDIRECTION: {
    struct SCRATCH_data degrees = POPDEGREES();
    sprite->base.rotation = degrees.data.degrees;
    status = SCRATCH_continue;
    break;
}
case MOTION_POINTTOWARDS: {
    struct SCRATCH_data y = POPFRACTION();
    struct SCRATCH_data x = POPFRACTION();
    float xDist = sprite->base.x.halves.high - x.data.number.halves.high;
    float yDist = sprite->base.y.halves.high - y.data.number.halves.high;
    float direction = atan2(yDist, -xDist);
    direction *= radianToDegree;
    uint32_t intDirection = (uint32_t) direction;

    intDirection += (uint32_t)quarterRotation;

    sprite->base.rotation = intDirection;
    status = SCRATCH_continue;
    break;
}
case MOTION_POINTTOWARDS_MENU: {
    ERROR(); // unused
    break;
}
case MOTION_CHANGEXBY: {
    struct SCRATCH_data x = POPFRACTION();
    sprite->base.x.i += x.data.number.i;
    keepInStage(context, sprite);
    status = SCRATCH_continue;
    break;
}
case MOTION_SETX: {
    struct SCRATCH_data x = POPFRACTION();
    sprite->base.x = x.data.number;
    keepInStage(context, sprite);
    status = SCRATCH_continue;
    break;
}
case MOTION_CHANGEYBY: {
    struct SCRATCH_data y = POPFRACTION();
    sprite->base.y.i += y.data.number.i;
    keepInStage(context, sprite);
    status = SCRATCH_continue;
    break;
}
case MOTION_SETY: {
    struct SCRATCH_data y = POPFRACTION();
    sprite->base.y = y.data.number;
    keepInStage(context, sprite);
    status = SCRATCH_continue;
    break;
}
case MOTION_IFONEDGEBOUNCE: {
    int xSpan = (SCRATCHWIDTH / 2);
    int ySpan = (SCRATCHHEIGHT / 2);
    struct SCRATCH_rect myRect = getRect(context, NULL);
    const int distances[4] = {
        MAX(0, xSpan + myRect.x),
        MAX(0, ySpan + myRect.y),
        MAX(0, xSpan - (myRect.x + myRect.width)),
        MAX(0, ySpan - (myRect.y + myRect.height))
    };

    int minDistIndex = 0;
    for (int i = 0; i < 4; i++) {
        if (distances[i] < distances[minDistIndex]) minDistIndex = i;
    }
    if (distances[minDistIndex] > 0) {
        status = SCRATCH_continue;
        break;
    }

    float radians = degreeToRadian * (quarterRotation - sprite->base.rotation);
    float dx = cos(radians);
    float dy = -sin(radians);
    float* arg = &dy;
    if (minDistIndex % 2 == 0) {
        arg = &dx;
    }
    float multiplier = 1;
    if (minDistIndex > 1) {
        multiplier = -1;
    }
    *arg = MAX(0.2f, fabs(*arg)) * multiplier;

    sprite->base.rotation = (atan2(dy, dx) * radianToDegree) + quarterRotation;
    printf("unscaled: %u, degrees: %lu\n", sprite->base.rotation, (uint64_t)sprite->base.rotation * 360 / ((uint64_t)UINT32_MAX + 1));

    keepInFrame(context, sprite);
    status = SCRATCH_continue;
    break;
}
case MOTION_SETROTATIONSTYLE: {
    uint16_t style = GETARGUMENT(int16_t);
    sprite->base.rotationStyle = style;
    status = SCRATCH_continue;
    break;
}
case LOOKS_SAY: {
    // TODO: copy speech strings into heap memory and handle freeing
    static char buffer[300];
    struct SCRATCH_data string = POPTEXT(buffer);
    sprite->talkingString = string.data.string;
    break;
}
case LOOKS_THINKFORSECS: {
    ERROR(); // unused
    break;
}
case LOOKS_SAYFORSECS: {
    ERROR(); // unused
    break;
}
case LOOKS_THINK: {
    // TODO: copy speech strings into heap memory and handle freeing
    static char buffer[300];
    struct SCRATCH_data string = POPTEXT(buffer);
    sprite->talkingString = string.data.string;
    break;
}
case LOOKS_SWITCHCOSTUMETO: {
    uint16_t index = POPID();
    sprite->base.costumeIndex = index - 1;
    status = SCRATCH_continue;
    break;
}
case LOOKS_NEXTCOSTUME: {
    sprite->base.costumeIndex += 1;
    sprite->base.costumeIndex %= sprite->base.costumeMax;
    status = SCRATCH_continue;
    break;
}
case LOOKS_SWITCHBACKDROPTO: {
    uint16_t index = POPID();
    context->stage->base.costumeIndex = index - 1;
    status = SCRATCH_continue;
    break;
}
case LOOKS_BACKDROPS: {
    ERROR();
    break;
}
case LOOKS_NEXTBACKDROP: {
    context->stage->base.costumeIndex += 1;
    context->stage->base.costumeIndex %= sprite->base.costumeMax;
    status = SCRATCH_continue;
    break;
}
case LOOKS_CHANGESIZEBY: {
    // TODO
    break;
}
case LOOKS_SETSIZETO: {
    struct SCRATCH_data sizeData = POPWHOLENUMBER();
    uint32_t size = sizeData.data.wholeNumber;
    size *= SIZERATIO;
    size /= 100;
    sprite->base.size = size;
    status = SCRATCH_continue;
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
    status = SCRATCH_continue;
    break;
}
case LOOKS_HIDE: {
    sprite->base.visible = false;
    status = SCRATCH_continue;
    break;
}
case LOOKS_GOTOFRONTBACK: {
    //ERROR(); // unused
    break;
}
case LOOKS_GOFORWARDBACKWARDLAYERS: {
    ERROR(); // unused
    break;
}
case INNER__WAITITERATION: {
    uint16_t programCounter = thread->programCounter;
    uint16_t framePos = GETARGUMENT(uint16_t);
    struct SCRATCH_data* iterations = SCRATCH_vectorFromTop(&thread->threadLocals, framePos);
    if (iterations->data.wholeNumber > 0) {
        iterations->data.wholeNumber--;
        thread->programCounter = programCounter - 1;
    }
    status = SCRATCH_yieldGeneric;
    break;
}
case INNER_NOP: {
    status = SCRATCH_yieldGeneric;
    break;
}
case INNER_DEBUGSTATEMENT: {
    machineLog("DEBUG\n");
    status = SCRATCH_yieldGeneric;
    break;
}
