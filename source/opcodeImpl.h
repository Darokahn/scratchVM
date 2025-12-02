// This file is inserted in scratch.c as the body for a switch statement. It is the right combination of size and simple layout that it seemed a good target for factoring out.

#define INTMAX16 65535
#define ERROR() // Nothing for now
#define INTERPRET_AS(type, value) *(type*)&(value)
#define POPNUMBER() cast(stack[--stackIndex], SCRATCH_NUMBER, NULL)
#define POPBOOL() cast(stack[--stackIndex], SCRATCH_BOOL, NULL)
#define POPID() ((int16_t)cast(stack[--stackIndex], SCRATCH_NUMBER, NULL).data.number.i)
#define POPDEGREES() cast(stack[--stackIndex], SCRATCH_DEGREES, NULL)
#define POPTEXT(buffer) cast(stack[--stackIndex], SCRATCH_STRING, buffer)
#define POPDATA() stack[--stackIndex]
#define PUSHNUMBER(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_NUMBER, {.number.halves.high=value}};
#define PUSHFRACTION(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_NUMBER, {.number.i=value}};
#define PUSHID(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_NUMBER, {.number.i=(int32_t)value}};
#define PUSHBOOL(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_BOOL, {.boolean=value}};
#define PUSHDATA(value) stack[stackIndex++] = value;
#define PUSHSTATICTEXT(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_STATICSTRING, {.string=(value)}};
#define PUSHTEXT(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_STRING, {.string=NULL}};
#define PUSHDEGREES(value) stack[stackIndex++] = (struct SCRATCH_data) {SCRATCH_DEGREES, {.degrees=value}};

#define GETARGUMENT(type) (thread->programCounter = (thread->programCounter + 3) & ~3, thread->programCounter += sizeof(type), INTERPRET_AS(type, code[thread->programCounter - sizeof(type)]))

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
    uint16_t jumpTo = GETARGUMENT(uint16_t);
    struct SCRATCH_data toMatch = POPNUMBER();
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
    PUSHNUMBER(0);
    status = SCRATCH_continue;
    break;
}
case SENSING_MOUSEY: {
    PUSHNUMBER(0);
    status = SCRATCH_continue;
    break;
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
        PUSHNUMBER(rand() % 500 - 250);
        PUSHNUMBER(rand() % 400 - 200);
    }
    else if (value == -2) { // mouse position (not handled for this VM)
        PUSHNUMBER(0);
        PUSHNUMBER(0);
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
        PUSHNUMBER(sprite->base.costumeIndex);
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
        PUSHNUMBER(context->stage->base.costumeIndex);
    }
    else {
        char* name =getImage(context, context->stage)->name;
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
    ERROR(); // unused
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
    PUSHNUMBER(hypot);

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
    ERROR(); // unused
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
    PUSHFRACTION((op1.data.number.i * op2.data.number.i) >> 16);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_DIVIDE: {
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
    PUSHFRACTION((int64_t)(op1.data.number.i << 16) / op2.data.number.i);
    status = SCRATCH_continue;
    break;
}
case OPERATOR_RANDOM: {
    struct SCRATCH_data high = POPNUMBER();
    struct SCRATCH_data low = POPNUMBER();
    int lowValue = low.data.number.halves.high;
    int highValue = high.data.number.halves.high;
    int rangeSize = highValue - lowValue;
    uint64_t randomNumber = rand();
    PUSHNUMBER(lowValue + randomNumber * (rangeSize + 1) / ((uint64_t)RAND_MAX + 1));
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
    //char buffer[32];
    //printf("sprite: %d, var: %d, value: %s\n", spriteOperand->base.id, varIndex, cast(x, SCRATCH_STRING, buffer).data.string);
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
    //char buffer[32];
    //printf("sprite: %d, var: %d, value: %s\n", spriteOperand->base.id, varIndex, cast(x, SCRATCH_STRING, buffer).data.string);
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
        struct SCRATCH_threadHeader base = template->threads[i].base;
        SCRATCH_initThread(&newSprite->threads[i], base);
    }
    for (int i = 0; i < h.variableCount; i++) {
        if (template->variables[i].type != SCRATCH_STRING) {
            newSprite->variables[i] = template->variables[i];
        }
        // SCRATCH_STRING variables need to be handled specially because they hold allocated strings that need to be copied. TODO
    }
    SCRATCH_wakeSprite(newSprite, ONCLONE, (union SCRATCH_eventInput) {0});
    if (!SCRATCH_addSprite(context, newSprite)) {
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
    free(sprite);
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
    sprite->base.x.i += thread->operationData.glideData.stepX.i;
    sprite->base.y.i += thread->operationData.glideData.stepY.i;
    thread->operationData.glideData.remainingIterations--;
    keepInStage(context, sprite);
    if (thread->operationData.glideData.remainingIterations <= 0) {
        sprite->base.x.halves.high = thread->operationData.glideData.targetX;
        sprite->base.y.halves.high = thread->operationData.glideData.targetY;
        keepInStage(context, sprite);
        status = SCRATCH_yieldGeneric;
        break;
    }
    thread->programCounter--; // re-align program counter with this instruction so it runs again
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_MOVESTEPS: {
    struct SCRATCH_data steps = POPNUMBER();
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
    struct SCRATCH_data op2 = POPNUMBER();
    struct SCRATCH_data op1 = POPNUMBER();
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
    struct SCRATCH_data y = POPNUMBER();
    struct SCRATCH_data x = POPNUMBER();
    struct SCRATCH_data scaledSecs = POPNUMBER();
    scaledInt32 xDiff = {.i = x.data.number.i - sprite->base.x.i};
    scaledInt32 yDiff = {.i = y.data.number.i - sprite->base.y.i};
    uint16_t iterations = (scaledSecs.data.number.i * FRAMESPERSEC) >> 16;
    if (iterations == 0) {
        sprite->base.x = x.data.number;
        sprite->base.y = y.data.number;
        keepInStage(context, sprite);
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
    status = SCRATCH_continue;
    break;
}
case MOTION_POINTTOWARDS: {
    struct SCRATCH_data x = POPNUMBER();
    struct SCRATCH_data y = POPNUMBER();
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
    struct SCRATCH_data x = POPNUMBER();
    sprite->base.x.i += x.data.number.i;
    keepInStage(context, sprite);
    status = SCRATCH_continue;
    break;
}
case MOTION_SETX: {
    struct SCRATCH_data x = POPNUMBER();
    sprite->base.x = x.data.number;
    keepInStage(context, sprite);
    status = SCRATCH_continue;
    break;
}
case MOTION_CHANGEYBY: {
    struct SCRATCH_data y = POPNUMBER();
    sprite->base.y.i += y.data.number.i;
    keepInStage(context, sprite);
    status = SCRATCH_continue;
    break;
}
case MOTION_SETY: {
    struct SCRATCH_data y = POPNUMBER();
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
        MAX(0, ySpan - myRect.y - myRect.height),
        MAX(0, xSpan - myRect.x - myRect.width),
        MAX(0, ySpan + myRect.y)
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
    sprite->base.costumeIndex = index;
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
    context->stage->base.costumeIndex = index;
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
    struct SCRATCH_data sizeData = POPNUMBER();
    uint16_t size = sizeData.data.number.halves.high;
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
