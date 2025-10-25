enum SCRATCH_opcode : uint8_t {
    INNER_PARTITION_BEGINLOOPCONTROL, // Semantic partition.

    // Loop opcodes. 

    INNER_LOOPINIT,                 // Push loop counter to loop stack
    INNER_LOOPINCREMENT,            // Increment the top of stack loop counter
    INNER_JUMPIFREPEATDONE,         // Jump if the top of the loop stack has reached a value   @field loop value @field jump location

    INNER_PARTITION_BEGINEXPRESSIONS, // Semantic partition.

    // Expression opcodes. May pop from the stack; always push to the stack.

    // Fetches. All fetches take the ID of the sprite whose property should be fetched as a field, perform no computation, and return a variable.
    // If opcode space is depleted, these can potentially be merged into one fetch that also takes the type of fetch as a field.

    SENSING_ANSWER,                 // RETURNS EMPTY STRING
    SENSING_MOUSEDOWN,              // RETURNS FALSE
    SENSING_MOUSEX,                 // RETURNS 0
    SENSING_MOUSEY,                 // RETURNS 0
    SENSING_KEYPRESSED,
    SENSING_LOUDNESS,               // UNUSED
    SENSING_TIMER,
    SENSING_CURRENT,                // ALWAYS RETURNS AS IF JAN 1ST 2025 00:00
    SENSING_DAYSSINCE2000,          // RETURNS FOR JAN 1ST 2025
    SENSING_USERNAME,
    INNER_FETCHINPUT,
    INNER_FETCHPOSITION,
    INNER_FETCHVAR,
    MOTION_XPOSITION,
    MOTION_YPOSITION,
    MOTION_DIRECTION,
    LOOKS_COSTUME,
    LOOKS_SIZE,
    LOOKS_COSTUMENUMBERNAME,
    LOOKS_BACKDROPNUMBERNAME,

    SENSING_TOUCHINGOBJECT,         // fields: object1, object2
    SENSING_TOUCHINGOBJECTMENU,     // IMPLEMENTED VIA FETCHPOSITION
    SENSING_TOUCHINGCOLOR,          // UNUSED
    SENSING_COLORISTOUCHINGCOLOR,   // UNUSED
    SENSING_DISTANCETO,             // fields: object1, object2
    SENSING_DISTANCETOMENU,         // IMPLEMENTED VIA FETCHPOSITION
    SENSING_ASKANDWAIT,             // fields: stringInput1
    SENSING_KEYOPTIONS,             // UNUSED
    SENSING_SETDRAGMODE,            // UNUSED
    SENSING_RESETTIMER,
    SENSING_OF,                     // IMPLEMENTED VIA FETCH SERIES OF OPCODES
    SENSING_OF_OBJECT_MENU,         // UNUSED

    INNER_PUSHNUMBER,

    OPERATOR_ADD,
    OPERATOR_SUBTRACT,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_RANDOM,
    OPERATOR_GT,
    OPERATOR_LT,
    OPERATOR_EQUALS,
    OPERATOR_GE,
    OPERATOR_AND,
    OPERATOR_OR,
    OPERATOR_NOT,
    OPERATOR_JOIN,
    OPERATOR_LETTER_OF,
    OPERATOR_LENGTH,
    OPERATOR_CONTAINS,
    OPERATOR_MOD,
    OPERATOR_ROUND,
    OPERATOR_MATHOP,

    INNER_DEBUGEXPRESSION,

    INNER_PARTITION_BEGINSTATEMENTS, // Semantic partition.
    // Only statement opcodes need to mind their return value. Still, for predictability, expression opcodes should return

    // Statement opcodes. Statements always leave the stack empty, unless there has been an error in compilation or implementation.
    DATA_SETVARIABLETO,          // Set a variable                                           @field variable index @input data
    DATA_CHANGEVARIABLEBY,          // increment a variable                                     @field variable index @input amount
    INNER_CHANGEVARIABLEBYLOCAL,          // increment a variable                                     @field variable index @input amount
    DATA_SHOWVARIABLE,
    DATA_HIDEVARIABLE,

    INNER_LOOPJUMP,        // Signal a loop iteration to interpreter                   @field jump destination
    CONTROL_CREATE_CLONE_OF,           // Treat cloning as a privileged primitive operation        @input sprite index
    CONTROL_WAIT,
    CONTROL_REPEAT,
    CONTROL_FOREVER,
    CONTROL_IF,
    CONTROL_IF_ELSE,
    CONTROL_WAIT_UNTIL,
    CONTROL_REPEAT_UNTIL,
    CONTROL_CREATE_CLONE_OF_MENU,
    CONTROL_DELETE_THIS_CLONE,          // Delete a sprite
    CONTROL_STOP,
    INNER_JUMPIF,          // Jump if top of stack is truthy                           @input evaluand @field jump destination
    INNER_JUMPIFNOT,       // Jump if top of stack is not truthy      @input evaluand @field jump destination
    INNER_JUMP,            // Unconditional jump

    INNER__GLIDEITERATION,

    MOTION_MOVESTEPS,
    MOTION_TURNRIGHT,
    MOTION_TURNLEFT,
    MOTION_GOTO,
    MOTION_GOTO_MENU,
    MOTION_GOTOXY,
    MOTION_GLIDETO,
    MOTION_GLIDETO_MENU,
    MOTION_GLIDESECSTOXY,
    MOTION_POINTINDIRECTION,
    MOTION_POINTTOWARDS,
    MOTION_POINTTOWARDS_MENU,
    MOTION_CHANGEXBY,
    MOTION_SETX,
    MOTION_CHANGEYBY,
    MOTION_SETY,
    MOTION_IFONEDGEBOUNCE,
    MOTION_SETROTATIONSTYLE,

    LOOKS_SAY,
    LOOKS_THINKFORSECS,
    LOOKS_THINK,
    LOOKS_SWITCHCOSTUMETO,
    LOOKS_NEXTCOSTUME,
    LOOKS_SWITCHBACKDROPTO,
    LOOKS_BACKDROPS,
    LOOKS_NEXTBACKDROP,
    LOOKS_CHANGESIZEBY,
    LOOKS_SETSIZETO,
    LOOKS_CHANGEEFFECTBY,
    LOOKS_SETEFFECTTO,
    LOOKS_CLEARGRAPHICEFFECTS,
    LOOKS_SHOW,
    LOOKS_HIDE,
    LOOKS_GOTOFRONTBACK,
    LOOKS_GOFORWARDBACKWARDLAYERS,

    INNER__WAITITERATION,

    INNER_DEBUGSTATEMENT,
};
