#ifndef SCRATCH_H
#define SCRATCH_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define STACKMAX (128)
#define MAXOPCODE (255)
#define LOOPNESTMAX (16) // deepest nesting for `repeat x` loops before failure
#define SPRITEMAX (8) // maximum spritecount
#define IMAGEMAX (16)

#define FRAMESPERSEC 50

#define STAGERESOLUTION 128
#define SPRITERESOLUTION 32

#define PI 3.14159265358979323846264338279f

#define halfRotation (32468.0f)
#define degreeToRadian (PI / halfRotation)
#define radianToDegree (halfRotation / PI)

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    typedef union {
        int32_t i; 
        __attribute__((packed)) struct {
            uint16_t low; 
            int16_t high;
        } halves;
    } scaledInt32;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    typedef union {
        int32_t i; 
        __attribute__((packed)) struct {
            int16_t high; 
            uint16_t low;
        } halves;
    } scaledInt32;
#else
    #error "Unknown endianness"
#endif

// using GNU typed enum extension
enum SCRATCH_fieldType : uint8_t {
    SCRATCH_BOOL,
    SCRATCH_NUMBER,
    SCRATCH_FRACTION,
    SCRATCH_STRING,
    SCRATCH_SPRITE,
    SCRATCH_COLOR,
};

struct SCRATCH_sprite;
struct SCRATCH_thread;

union SCRATCH_field {
    int number;
    float fraction;
    bool boolean;
    char* string;
    uint8_t spriteID;
    uint8_t color;
};

struct SCRATCH_data {
    enum SCRATCH_fieldType type;
    union SCRATCH_field data;
};

struct SCRATCH_list {
    struct SCRATCH_data* items;
    int itemCount;
    int itemCapacity;
};

// fields are read from the bytecode stream; inputs are popped from the stack (presumably previously pushed)
enum SCRATCH_opcode : uint8_t {
    SCRATCH_PARTITION_BEGINLOOPCONTROL, // Semantic partition.

    // Loop opcodes. 

    INNER_LOOPINIT,        // Push loop counter to loop stack
    INNER_LOOPINCREMENT,   // Increment the top of stack loop counter
    INNER_JUMPIFREPEATDONE,// Jump if the top of the loop stack has reached a value   @field loop value @field jump location

    INNER_PARTITION_BEGINEXPRESSIONS, // Semantic partition.

    // Expression opcodes. May pop from the stack; always push to the stack.

    INNER_FETCH,           // Fetch some special value such as `x position`            @field which property to get
    INNER_FETCHINPUT,      // Fetch an input property                                  @field input
    INNER_FETCHFROM,       // Like above, but fetches from an aritrary sprite.         @input which sprite @field which property
    INNER_FETCHPOSITION,   // Fetches a special position                               @field which position

    INNER_FETCHTOUCHING,
    INNER_LOADVAR,         // Load a variable                                          @field variable index
    INNER_push,            // Push argument                                            @field value

    OPERATOR_ADD,             // Add two top-of-stack values                              @input op1 @input op2
    OPERATOR_SUBTRACT,             // Subtract two top-of-stack values                              @input op1 @input op2
    OPERATOR_MULTIPLY,        // Subtract two top-of-stack values                              @input op1 @input op2
    OPERATOR_DIVIDE,          // Subtract two top-of-stack values                              @input op1 @input op2
    OPERATOR_RANDOM,
    OPERATOR_GT,             // compare top-of-stack values                              @input op1 @input op2
    OPERATOR_LT,             // compare top-of-stack values                              @input op1 @input op2
    OPERATOR_EQUALS,             // compare top-of-stack values                              @input op1 @input op2 SCRATCH_lessThan,             // compare top-of-stack values                              @input op1 @input op2 SCRATCH_lessEqual,             // compare top-of-stack values                              @input op1 @input op2
    OPERATOR_GE,             // compare top-of-stack values                              @input op1 @input op2
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
    DATA_SETVARIABLETO,          // Set a variable                                           @field variable index @input data
    DATA_CHANGEVARIABLEBY,          // increment a variable                                     @field variable index @input amount
    DATA_LOADVAR,    // Get a local variable                                     @field variable
    DATA_LOADARRAYAT,     // Load from an array                                       @field array name @input array position


    INNER_DEBUGEXPRESSION,

    INNER_PARTITION_BEGINSTATEMENTS, // Semantic partition.
    // Only statement opcodes need to mind their return value. Still, for predictability, expression opcodes should return
    // SCRATCH_continue.

    // Statement opcodes. Statements always leave the stack empty, unless there has been an error in compilation or implementation.

    INNER_LOOPJUMP,        // Signal a loop iteration to interpreter                   @field jump destination
    CONTROL_CREATE_CLONE_OF,           // Treat cloning as a privileged primitive operation        @input sprite index
    CONTROL_DELETE_THIS_CLONE,          // Delete a sprite
    INNER_JUMPIF,          // Jump if top of stack is truthy                           @input evaluand @field jump destination
    INNER_JUMPIFNOT,       // Jump if top of stack is not truthy      @input evaluand @field jump destination
    INNER_JUMP,            // Unconditional jump

    MOTION_GOTOXY,
    MOTION_GLIDETO,
    MOTION__GLIDEITERATION,
    MOTION_TURNRIGHT,
    MOTION_TURNLEFT,
    MOTION_MOVESTEPS,
    MOTION_POINTINDIRECTION,
    MOTION_POINTTOWARDS,
    MOTION_SETX,
    MOTION_CHANGEXBY,
    MOTION_SETY,
    MOTION_CHANGEYBY,
    MOTION_SETROTATIONSTYLE,

    SCRATCH_looksSay,
    SCRATCH_hide,
    SCRATCH_show,
    SCRATCH_setSize,
    SCRATCH_setCostume,
    SCRATCH_nextCostume,

    CONTROL_WAIT,
    INNER__WAITITERATION,

    INNER_DEBUGSTATEMENT,

    CONTROL_stop,
};

// values indicating which dynamic value to fetch
enum SCRATCH_fetchValue : uint8_t {
    SCRATCH_xPosition,
    SCRATCH_yPosition,
    SCRATCH_direction,
    SCRATCH_costumeNumber,
    SCRATCH_backdropNumber,
    SCRATCH_size,
    SCRATCH_volume, // ignored
    SCRATCH_answer, // ignored
    SCRATCH_mouseX, // ignored
    SCRATCH_mouseY, // ignored
    SCRATCH_loudness, // ignored
    SCRATCH_timer,
    SCRATCH_year,
    SCRATCH_daysSince2000,
    SCRATCH_username, // ignored
};
// Signals a block gives the interpreter about how to continue the sequence
enum SCRATCH_continueStatus {
    SCRATCH_continue,
    SCRATCH_yieldGeneric,
};

typedef bool (*SCRATCH_functionIterator)(struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, int iteration);
typedef enum SCRATCH_continueStatus (*SCRATCH_function)(struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, struct SCRATCH_thread* thread);
// SCRATCH_functionIterator is used for animated blocks. A SCRATCH_function that returns non-null signals to the thread
// that this block should yield and use the returned enumeration to set a new current task.


enum SCRATCH_EVENTTYPE : uint8_t {
    ONKEY,
    ONMESSAGE,
    ONBACKDROP,
    ONCLONE,
    ONFLAG,
    ONCLICK, // ignored
    ONLOUDNESS, // ignored
    __EVENTTYPECOUNT
};

union SCRATCH_eventInput { // redundant union; meant for semantic labeling
    uint16_t key;
    uint16_t backdrop;
    uint16_t loudness;
    uint16_t message;
    uint16_t i;
};

struct SCRATCH_event {
    enum SCRATCH_EVENTTYPE type;
    union SCRATCH_eventInput input;
};

union SCRATCH_stepSize {
    uint16_t value;
    uint8_t bytes[2];
};

struct SCRATCH_glideData {
    int32_t stepX;
    int32_t stepY;
    uint16_t remainingIterations; // how many times to iterate
    uint16_t targetX; // what to set position to when done (assume slight error due to rounding)
    uint16_t targetY;
};

struct SCRATCH_waitData {
    uint32_t remainingIterations;
};

struct SCRATCH_header {
    uint32_t codeLength;
    uint32_t imageLength;
    uint32_t spriteCount;
    uint32_t messageCount;
    uint32_t backdropCount;
};

struct SCRATCH_spriteHeader {
    scaledInt32 x; // Scaled int split into a whole and fractional part. whole part is a Number that, according to my testing, can go from -1000 to 1000.
    scaledInt32 y;
    uint16_t rotation; // Rotation maps (0 -> 360) to the entire range of a 16-bit integer
    bool visible;
    int8_t layer;
    uint16_t size; // number representing percent of original size
    bool rotationStyle;
    uint8_t costumeIndex;
    uint8_t costumeMax;
    uint8_t threadCount;
    uint8_t variableCount;
    uint8_t id;
};

struct SCRATCH_threadHeader {
    union SCRATCH_eventInput eventCondition;
    uint16_t entryPoint;
    enum SCRATCH_EVENTTYPE startEvent;
};

struct SCRATCH_thread {
    struct SCRATCH_threadHeader base;
    uint16_t programCounter;
    bool active;
    uint16_t loopCounterStack[LOOPNESTMAX];
    uint8_t loopCounterStackIndex;
    union {
        struct SCRATCH_glideData glideData;
        struct SCRATCH_waitData waitData;
    } operationData;
};

// a SCRATCH_sprite is designed to sit in a single heap allocation. `variables` should point to the next SCRATCH_data aligned
// address after the end of `threads`.
struct SCRATCH_sprite {
    struct SCRATCH_spriteHeader base;
    struct SCRATCH_data* variables; // Variable 0 is always the sprite's message (what it might be `say`ing at any moment)
    struct SCRATCH_thread threads[];
};

enum SCRATCH_continueStatus SCRATCH_processBlock(struct SCRATCH_sprite* sprite, struct SCRATCH_thread* thread);
void SCRATCH_processThread(struct SCRATCH_sprite* sprite, struct SCRATCH_thread* thread);
int SCRATCH_visitAllThreads(struct SCRATCH_sprite** sprites, int spriteCount);
struct SCRATCH_sprite* SCRATCH_makeNewSprite(struct SCRATCH_spriteHeader header);
void SCRATCH_initThread(struct SCRATCH_thread*, struct SCRATCH_threadHeader);
void handleInputs();
void clearEvents();
bool SCRATCH_addSprite(struct SCRATCH_sprite* sprite);
bool SCRATCH_wakeSprite(struct SCRATCH_sprite* sprite, enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input);
void SCRATCH_wakeSprites();


#endif
