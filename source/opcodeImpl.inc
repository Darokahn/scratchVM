// This file is inserted in scratch.c as the body for a switch statement. It is the right combination of size and simple layout that it seemed a good target for factoring out.

#define ERROR() // Nothing for now
#define INTERPRET_AS(type, value) *(type*)&(value)
#define POP() stack[--stackIndex]
#define PUSH(value) stack[stackIndex++] = value;

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
    uint16_t toMatch = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof(toMatch);
    uint16_t jumpTo = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof(jumpTo);
    if (thread->loopCounterStack[thread->loopCounterStackIndex-1] >= toMatch) {
        thread->programCounter = jumpTo;
    }
    status = SCRATCH_continue;
    break;
}
case INNER_PARTITION_BEGINEXPRESSIONS: {
    ERROR();
    break;
}
case SENSING_ANSWER: {
    // push a string to the stack
    status = SCRATCH_continue;
    break;
}
case SENSING_MOUSEDOWN: {
    break;
}
case SENSING_MOUSEX: {
    break;
}
case SENSING_MOUSEY: {
    break;
}
case SENSING_KEYPRESSED: {
    break;
}
case SENSING_LOUDNESS: {
    break;
}
case SENSING_TIMER: {
    break;
}
case SENSING_CURRENT: {
    break;
}
case SENSING_DAYSSINCE2000: {
    break;
}
case SENSING_USERNAME: {
    break;
}
case INNER_FETCHINPUT: {
    uint16_t toFetch = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof(toFetch);
    PUSH( ((struct SCRATCH_data) {SCRATCH_BOOL, {.boolean = inputState[toFetch]}}));
    status = SCRATCH_continue;
    break;
}
case INNER_FETCHPOSITION: {
    uint16_t value = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof value;
    if (value == -1) { // random position
        stack[stackIndex] = (struct SCRATCH_data) {
            .type = SCRATCH_NUMBER,
            .data = (union SCRATCH_field) {.number = (rand() % 500) - 250},
        };
        (stackIndex)++;
        stack[stackIndex] = (struct SCRATCH_data) {
            .type = SCRATCH_NUMBER,
            .data = (union SCRATCH_field) {.number = (rand() % 400) - 200},
        };
        (stackIndex)++;
    }
    else if (value == -2) { // mouse position (not handled for this VM)
        stack[stackIndex] = (struct SCRATCH_data) {
            .type = SCRATCH_NUMBER,
            .data = (union SCRATCH_field) {.number = 0},
        };
        (stackIndex)++;
        stack[stackIndex] = (struct SCRATCH_data) {
            .type = SCRATCH_NUMBER,
            .data = (union SCRATCH_field) {.number = 0},
        };
        (stackIndex)++;
    }
    else { // a sprite's position
        struct SCRATCH_sprite* s = sprites[value];
        stack[stackIndex] = (struct SCRATCH_data) {
            .type = SCRATCH_NUMBER,
            .data = (union SCRATCH_field) {.number = s->base.y.halves.high},
        };
        (stackIndex)++;
        stack[stackIndex] = (struct SCRATCH_data) {
            .type = SCRATCH_NUMBER,
            .data = (union SCRATCH_field) {.number = s->base.x.halves.high},
        };
        (stackIndex)++;
    }
    status = SCRATCH_continue;
    break;
}
case INNER_FETCHVAR: {
    break;
}
case MOTION_XPOSITION: {
    break;
}
case MOTION_YPOSITION: {
    break;
}
case MOTION_DIRECTION: {
    break;
}
case LOOKS_COSTUME: {
    break;
}
case LOOKS_SIZE: {
    break;
}
case LOOKS_COSTUMENUMBERNAME: {
    break;
}
case LOOKS_BACKDROPNUMBERNAME: {
    break;
}
case SENSING_TOUCHINGOBJECT: {
    break;
}
case SENSING_TOUCHINGOBJECTMENU: {
    break;
}
case SENSING_TOUCHINGCOLOR: {
    break;
}
case SENSING_COLORISTOUCHINGCOLOR: {
    break;
}
case SENSING_DISTANCETO: {
    break;
}
case SENSING_DISTANCETOMENU: {
    break;
}
case SENSING_ASKANDWAIT: {
    break;
}
case SENSING_KEYOPTIONS: {
    break;
}
case SENSING_SETDRAGMODE: {
    break;
}
case SENSING_RESETTIMER: {
    break;
}
case SENSING_OF: {
    break;
}
case SENSING_OF_OBJECT_MENU: {
    break;
}
case INNER_PUSHNUMBER: {
    enum SCRATCH_fieldType type = code[thread->programCounter];
    thread->programCounter += sizeof(type);
    uint16_t field = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof(field);
    stack[stackIndex] = (struct SCRATCH_data) {type, {.number = field}};
    (stackIndex) += 1;
    status = SCRATCH_continue;
    break;
}
case OPERATOR_ADD: {
    struct SCRATCH_data op2 = stack[stackIndex-1];
    (stackIndex)--;
    struct SCRATCH_data op1 = stack[stackIndex-1];
    (stackIndex)--;
    int16_t result = op1.data.number + op2.data.number;
    stack[stackIndex] = (struct SCRATCH_data) {SCRATCH_NUMBER, {.number = result}};
    (stackIndex) += 1;
    status = SCRATCH_continue;
    break;
}
case OPERATOR_SUBTRACT: {
    struct SCRATCH_data op2 = stack[stackIndex-1];
    (stackIndex)--;
    struct SCRATCH_data op1 = stack[stackIndex-1];
    (stackIndex)--;
    int16_t result = op1.data.number - op2.data.number;
    stack[stackIndex] = (struct SCRATCH_data) {SCRATCH_NUMBER, {.number = result}};
    (stackIndex) += 1;
    status = SCRATCH_continue;
    break;
}
case OPERATOR_MULTIPLY: {
    (stackIndex) -= 1;
    struct SCRATCH_data op1 = stack[stackIndex];
    (stackIndex) -= 1;
    struct SCRATCH_data op2 = stack[stackIndex];
    stack[stackIndex] = (struct SCRATCH_data) {
        .type = SCRATCH_NUMBER,
        .data = (union SCRATCH_field) {.number = op1.data.number * op2.data.number}
    };
    (stackIndex) += 1;
    status = SCRATCH_continue;
    break;
}
case OPERATOR_DIVIDE: {
    (stackIndex) -= 1;
    struct SCRATCH_data op1 = stack[stackIndex];
    (stackIndex) -= 1;
    struct SCRATCH_data op2 = stack[stackIndex];
    stack[stackIndex] = (struct SCRATCH_data) {
        .type = SCRATCH_NUMBER,
        .data = (union SCRATCH_field) {.number = op1.data.number / op2.data.number}
    };
    (stackIndex) += 1;
    status = SCRATCH_continue;
    break;
}
case OPERATOR_RANDOM: {
    (stackIndex) -= 1;
    struct SCRATCH_data low = stack[stackIndex];
    (stackIndex) -= 1;
    struct SCRATCH_data high = stack[stackIndex];
    int lowValue = low.data.number;
    int highValue = high.data.number;
    stack[stackIndex] = (struct SCRATCH_data) {
        .type = SCRATCH_NUMBER,
        .data = (union SCRATCH_field) {.number = (rand() % (highValue - lowValue)) + lowValue}
    };
    (stackIndex) += 1;
    status = SCRATCH_continue;
    break;
}
case OPERATOR_GT: {
    break;
}
case OPERATOR_LT: {
    (stackIndex) -= 1;
    struct SCRATCH_data op1 = stack[stackIndex];
    (stackIndex) -= 1;
    struct SCRATCH_data op2 = stack[stackIndex];
    stack[stackIndex] = (struct SCRATCH_data) {
        .type = SCRATCH_NUMBER,
        .data = (union SCRATCH_field) {.boolean = op1.data.number < op2.data.number}
    };
    (stackIndex) += 1;
    status = SCRATCH_continue;
    break;
}
case OPERATOR_EQUALS: {
    (stackIndex) -= 1;
    struct SCRATCH_data op1 = stack[stackIndex];
    (stackIndex) -= 1;
    struct SCRATCH_data op2 = stack[stackIndex];
    stack[stackIndex] = (struct SCRATCH_data) {
        .type = SCRATCH_NUMBER,
        .data = (union SCRATCH_field) {.boolean = op1.data.number == op2.data.number}
    };
    (stackIndex) += 1;
    status = SCRATCH_continue;
    break;
}
case OPERATOR_GE: {
    (stackIndex) -= 1;
    struct SCRATCH_data op1 = stack[stackIndex];
    (stackIndex) -= 1;
    struct SCRATCH_data op2 = stack[stackIndex];
    stack[stackIndex] = (struct SCRATCH_data) {
        .type = SCRATCH_NUMBER,
        .data = (union SCRATCH_field) {.boolean = op1.data.number > op2.data.number}
    };
    (stackIndex) += 1;
    status = SCRATCH_continue;
    break;
}
case OPERATOR_AND: {
    break;
}
case OPERATOR_OR: {
    (stackIndex) -= 1;
    struct SCRATCH_data op1 = stack[stackIndex];
    (stackIndex) -= 1;
    struct SCRATCH_data op2 = stack[stackIndex];
    stack[stackIndex] = (struct SCRATCH_data) {
        .type = SCRATCH_NUMBER,
        .data = (union SCRATCH_field) {.boolean = op1.data.number || op2.data.number}
    };
    (stackIndex) += 1;
    status = SCRATCH_continue;
    break;
}
case OPERATOR_NOT: {
    break;
}
case OPERATOR_JOIN: {
    break;
}
case OPERATOR_LETTER_OF: {
    break;
}
case OPERATOR_LENGTH: {
    break;
}
case OPERATOR_CONTAINS: {
    break;
}
case OPERATOR_MOD: {
    break;
}
case OPERATOR_ROUND: {
    break;
}
case OPERATOR_MATHOP: {
    break;
}
case INNER_DEBUGEXPRESSION: {
    machineLog("DEBUG\n");
    status = SCRATCH_yieldGeneric;
    break;
}
case INNER_PARTITION_BEGINSTATEMENTS: {
    break;
}
case DATA_SETVARIABLETO: {
    break;
}
case DATA_CHANGEVARIABLEBY: {
    /*
    uint16_t varIndex = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof varIndex;
    uint16_t targetSprite = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof varIndex;
    struct SCRATCH_data value = stack[stackIndex-1];
    (stackIndex) -= 1;
    sprites[targetSprite.data.number]->variables[varIndex].data.number += value.data.number;
    status = SCRATCH_continue;
    */
    break;
}
case INNER_CHANGEVARIABLEBYLOCAL: {
    uint16_t varIndex = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof varIndex;
    struct SCRATCH_data value = stack[stackIndex-1];
    (stackIndex) -= 1;
    sprite->variables[varIndex].data.number += value.data.number;
    status = SCRATCH_continue;
    break;
}
case DATA_SHOWVARIABLE: {
    break;
}
case DATA_HIDEVARIABLE: {
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
    (stackIndex)--;
    struct SCRATCH_data scaledSecs = stack[stackIndex]; // seconds scaled so that the larger 11 bits is seconds and the smaller 5 are seconds / 32
    uint16_t iterations = (scaledSecs.data.number * FRAMESPERSEC) >> 5;
    thread->operationData.waitData = (struct SCRATCH_waitData) {
        .remainingIterations = iterations
    };
    status = SCRATCH_yieldGeneric;

    break;
}
case CONTROL_REPEAT: {
    break;
}
case CONTROL_FOREVER: {
    break;
}
case CONTROL_IF: {
    break;
}
case CONTROL_IF_ELSE: {
    break;
}
case CONTROL_WAIT_UNTIL: {
    break;
}
case CONTROL_REPEAT_UNTIL: {
    break;
}
case CONTROL_CREATE_CLONE_OF_MENU: {
    break;
}
case CONTROL_DELETE_THIS_CLONE: {
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
    struct SCRATCH_data evaluand = stack[stackIndex-1];
    (stackIndex) -= 1;
    if (evaluand.data.boolean) {
        thread->programCounter = jumpTo;
    }
    status = SCRATCH_continue;
    break;
}
case INNER_JUMPIFNOT: {
    uint16_t jumpTo = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof jumpTo;
    struct SCRATCH_data evaluand = stack[stackIndex-1];
    stackIndex -= 1;
    if (!evaluand.data.boolean) {
        thread->programCounter = jumpTo;
    }
    status = SCRATCH_continue;
    break;
}
case INNER_JUMP: {
    break;
}
case INNER__GLIDEITERATION: {
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
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_MOVESTEPS: {
    (stackIndex)--;
    struct SCRATCH_data steps = stack[stackIndex];
    float rotation = sprite->base.rotation * degreeToRadian;

    int x = sin(rotation) * steps.data.number;
    int y = cos(rotation) * steps.data.number;
    sprite->base.x.halves.high += x;
    sprite->base.y.halves.high += y;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_TURNRIGHT: {
    (stackIndex)--;
    struct SCRATCH_data degrees = stack[stackIndex];
    sprite->base.rotation += degrees.data.number;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_TURNLEFT: {
    (stackIndex)--;
    struct SCRATCH_data degrees = stack[stackIndex];
    sprite->base.rotation -= degrees.data.number;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_GOTO: {
    break;
}
case MOTION_GOTO_MENU: {
    break;
}
case MOTION_GOTOXY: {
    (stackIndex)--;
    struct SCRATCH_data op2 = stack[stackIndex];
    (stackIndex)--;
    struct SCRATCH_data op1 = stack[stackIndex];
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
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_GLIDETO: {
    (stackIndex)--;
    struct SCRATCH_data scaledSecs = stack[stackIndex]; // seconds scaled so that the larger 11 bits is seconds and the smaller 5 are seconds / 32
    (stackIndex)--;
    struct SCRATCH_data x = stack[stackIndex];
    (stackIndex)--;
    struct SCRATCH_data y = stack[stackIndex];
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
    status = SCRATCH_continue;
    break;
}
case MOTION_GLIDETO_MENU: {
    break;
}
case MOTION_GLIDESECSTOXY: {
    break;
}
case MOTION_POINTINDIRECTION: {
    (stackIndex)--;
    struct SCRATCH_data degrees = stack[stackIndex];
    sprite->base.rotation = degrees.data.number;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_POINTTOWARDS: {
    (stackIndex)--;
    struct SCRATCH_data x = stack[stackIndex];
    (stackIndex)--;
    struct SCRATCH_data y = stack[stackIndex];
    float direction = atan2(y.data.number, x.data.number);
    direction *= radianToDegree;
    sprite->base.rotation = (uint16_t) direction;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_POINTTOWARDS_MENU: {
    break;
}
case MOTION_CHANGEXBY: {
    (stackIndex)--;
    struct SCRATCH_data x = stack[stackIndex];
    sprite->base.x.halves.high += x.data.number;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_SETX: {
    (stackIndex)--;
    struct SCRATCH_data x = stack[stackIndex];
    sprite->base.x.halves.high = x.data.number;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_CHANGEYBY: {
    (stackIndex)--;
    struct SCRATCH_data y = stack[stackIndex];
    sprite->base.y.halves.high += y.data.number;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_SETY: {
    (stackIndex)--;
    struct SCRATCH_data y = stack[stackIndex];
    sprite->base.y.halves.high = y.data.number;
    status = SCRATCH_yieldGeneric;
    break;
}
case MOTION_IFONEDGEBOUNCE: {
    break;
}
case MOTION_SETROTATIONSTYLE: {
    (stackIndex)--;
    struct SCRATCH_data style = stack[stackIndex];
    sprite->base.rotationStyle = style.data.boolean;
    status = SCRATCH_yieldGeneric;
    break;
}
case LOOKS_SAY: {
    break;
}
case LOOKS_THINKFORSECS: {
    break;
}
case LOOKS_THINK: {
    break;
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
    break;
}
case LOOKS_BACKDROPS: {
    break;
}
case LOOKS_NEXTBACKDROP: {
    break;
}
case LOOKS_CHANGESIZEBY: {
    break;
}
case LOOKS_SETSIZETO: {
    (stackIndex)--;
    struct SCRATCH_data size = stack[stackIndex];
    sprite->base.size = size.data.number;
    status = SCRATCH_yieldGeneric;
    break;
}
case LOOKS_CHANGEEFFECTBY: {
    break;
}
case LOOKS_SETEFFECTTO: {
    break;
}
case LOOKS_CLEARGRAPHICEFFECTS: {
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
    break;
}
case LOOKS_GOFORWARDBACKWARDLAYERS: {
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


/*
SCRATCH_implementFunction(loadVar) {
    uint16_t varIndex = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof varIndex;
    stack[stackIndex] = sprites[0]->variables[varIndex];
    (stackIndex) += 1;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(setVar) {
    uint16_t varIndex = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof varIndex;
    struct SCRATCH_data value = stack[stackIndex-1];
    (stackIndex) -= 1;
    sprites[0]->variables[varIndex] = value;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(loadVarLocal) {
    uint16_t varIndex = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof varIndex;
    stack[stackIndex] = sprite->variables[varIndex];
    (stackIndex) += 1;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(setVarLocal) {
    uint16_t varIndex = INTERPRET_AS(uint16_t, code[thread->programCounter]);
    thread->programCounter += sizeof varIndex;
    struct SCRATCH_data value = stack[stackIndex-1];
    (stackIndex) -= 1;
    sprite->variables[varIndex] = value;
    return SCRATCH_continue;
}

SCRATCH_implementFunction(looksSay) {
}

*/
#undef ERROR
#undef INTERPRET_AS
