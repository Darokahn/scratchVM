// This file is inserted in scratch.c as the body for a switch statement. It is the right combination of size and simple layout that it seemed a good target for factoring out.

#define INTMAX16 65535
#define ERROR() // Nothing for now
#define INTERPRET_AS(type, value) *(type*)&(value)
#define POPFRACTION() cast(stack[--stackIndex], SCRATCH_FRACTION, NULL)
#define POPWHOLENUMBER() cast(stack[--stackIndex], SCRATCH_WHOLENUMBER, NULL)
#define POPBOOL() cast(stack[--stackIndex], SCRATCH_BOOL, NULL)
#define POPID() ((int16_t)cast(stack[--stackIndex], SCRATCH_WHOLENUMBER, NULL).data.number.i)
#define POPDEGREES() cast(stack[--stackIndex], SCRATCH_DEGREES, NULL)
#define POPTEXT(buffer) cast(stack[--stackIndex], SCRATCH_STRING, buffer)
#define POPDATA() stack[--stackIndex]
#define PUSHFRACTION(value) stack[stackIndex++] = (struct SCRATCH_data) {{.number.i=value}, .type=SCRATCH_FRACTION};
#define PUSHWHOLENUMBER(value) stack[stackIndex++] = (struct SCRATCH_data) {{.wholeNumber=(int32_t)value}, .type=SCRATCH_WHOLENUMBER};
#define PUSHID(value) stack[stackIndex++] = (struct SCRATCH_data) {{.wholeNumber=(int32_t)value}, .type=SCRATCH_WHOLENUMBER};
#define PUSHBOOL(value) stack[stackIndex++] = (struct SCRATCH_data) {{.boolean=value}, .type=SCRATCH_BOOL};
#define PUSHDATA(value) stack[stackIndex++] = value;
#define PUSHSTATICTEXT(value) stack[stackIndex++] = (struct SCRATCH_data) {{.string=(value)}, .type=SCRATCH_STATICSTRING};
#define PUSHTEXT(value) stack[stackIndex++] = (struct SCRATCH_data) {{.string=NULL}, .type=SCRATCH_STRING};
#define PUSHDEGREES(value) stack[stackIndex++] = (struct SCRATCH_data) {{.degrees=value}, .type=SCRATCH_DEGREES};

#define GETARGUMENT(type) (thread->programCounter = (thread->programCounter + 3) & ~3, thread->programCounter += sizeof(type), INTERPRET_AS(type, code[thread->programCounter - sizeof(type)]))

case INNER_PARTITION_BEGINEXPRESSIONS: {
    // Not to be implemented
    ERROR();
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
case SENSING_KEYPRESSED: {
    struct SCRATCH_data keyIndex = POPWHOLENUMBER();
    PUSHBOOL(inputState[keyIndex.data.wholeNumber]);
    status = SCRATCH_continue;
    break;
}
case SENSING_LOUDNESS: {
    PUSHFRACTION(0);
    status = SCRATCH_continue;
    break;
}
case SENSING_TIMER: {
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
    uint16_t toFetch = GETARGUMENT(uint16_t);
    PUSHBOOL(inputState[toFetch]);
    status = SCRATCH_continue;
    break;
}
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
    /*
    char buffer[32];
    machineLog("sprite: %d, var: %d\n\r", spriteOperand->base.id, varIndex);
    if (spriteOperand->variables[varIndex].type == SCRATCH_UNINIT) {
        machineLog("WARNING: scratch variable uninitialized. Not printing.\n\r");
        fflush(stdout);
    }
    else machineLog("value: %s\n\r", cast(spriteOperand->variables[varIndex], SCRATCH_STRING, buffer).data.string);
    */
    PUSHDATA(spriteOperand->variables[varIndex]);
    status = SCRATCH_continue;
    break;
}
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
    ERROR() // unused
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
    PUSHWHOLENUMBER(hypot);

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
    // TODO
    // set the value of the global timer to 0
}
case SENSING_OF: {
    int16_t id = GETARGUMENT(int16_t);
    struct SCRATCH_sprite* spriteOperand = context->sprites[POPID()];
    switch (id) {
        case 0: // costume number
            PUSHWHOLENUMBER(spriteOperand->base.costumeIndex);
            break;
        case 1: // costume name
            char* name = getImage(context, spriteOperand)->name;
            PUSHSTATICTEXT(name);
            break;
        case 2: // volume
            PUSHWHOLENUMBER(0);
            break;
        case 3: // x
            PUSHFRACTION(spriteOperand->base.x.i);
            break;
        case 4: // y
            PUSHFRACTION(spriteOperand->base.y.i);
            break;
        case 5: // direction
            PUSHDEGREES(spriteOperand->base.rotation);
            break;
        case 6: // size
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
    struct SCRATCH_data op2 = POPFRACTION();
    struct SCRATCH_data op1 = POPFRACTION();
    PUSHBOOL(op1.data.number.i == op2.data.number.i);
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
case CONTROL_CREATE_CLONE_OF: {
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
        free(newSprite);
    }
    status = SCRATCH_yieldGeneric;
    break;
}
case CONTROL_WAIT: {
    struct SCRATCH_data scaledSecs = POPFRACTION();
    uint16_t iterations = ((scaledSecs.data.number.i * FRAMESPERSEC) >> 16);
    SCRATCH_vectorPush(&thread->threadLocals, (struct SCRATCH_data){{.wholeNumber=iterations}, SCRATCH_WHOLENUMBER});
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
    break;
}
case INNER__GLIDEITERATION: {
    keepInStage(context, sprite);
    thread->programCounter--; // re-align program counter with this instruction so it runs again
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
    struct SCRATCH_data y = POPFRACTION();
    struct SCRATCH_data x = POPFRACTION();
    struct SCRATCH_data scaledSecs = POPFRACTION();
    scaledInt32 xDiff = {.i = x.data.number.i - sprite->base.x.i};
    scaledInt32 yDiff = {.i = y.data.number.i - sprite->base.y.i};
    (void) xDiff;
    (void) yDiff;
    uint16_t iterations = (scaledSecs.data.number.i * FRAMESPERSEC) >> 16;
    if (iterations == 0) {
        sprite->base.x = x.data.number;
        sprite->base.y = y.data.number;
        keepInStage(context, sprite);
        return SCRATCH_continue;
    }
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
    struct SCRATCH_data x = POPFRACTION();
    struct SCRATCH_data y = POPFRACTION();
    float direction = atan2(y.data.number.halves.high, x.data.number.halves.high);
    direction *= radianToDegree;
    sprite->base.rotation = (uint32_t) direction;
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
    uint16_t style = GETARGUMENT(uint16_t);
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
    int16_t index = GETARGUMENT(uint16_t);
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
    int16_t index = GETARGUMENT(uint16_t);
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
    ERROR(); // unused
    break;
}
case LOOKS_GOFORWARDBACKWARDLAYERS: {
    ERROR(); // unused
    break;
}
case INNER__WAITITERATION: {
    struct SCRATCH_data* iterations = SCRATCH_vectorTop(&thread->threadLocals);
    if (iterations->data.wholeNumber > 0) {
        iterations->data.wholeNumber--;
        thread->programCounter--;
    }
    else {
        SCRATCH_vectorPop(&thread->threadLocals);
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
