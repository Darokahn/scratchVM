#ifndef SCRATCH_H
#define SCRATCH_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define STACKMAX (128)
#define STRINGREGISTERMAX (4096)
#define MAXOPCODE (255)
#define LOOPNESTMAX (8) // deepest nesting for `repeat x` loops before failure
#define SPRITEMAX (8) // maximum spritecount
#define IMAGEMAX (16)

#define FRAMESPERSEC 30

#define STAGERESOLUTION 128
#define SPRITERESOLUTION 32

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    typedef union {
        int32_t i; 
        __attribute__((packed)) struct {
            int16_t low; 
            int16_t high;
        } halves;
    } scaledInt32;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    typedef union {
        int32_t i; 
        __attribute__((packed)) struct {
            int16_t high; 
            int16_t low;
        } halves;
    } scaledInt32;
#else
    #error "Unknown endianness"
#endif

typedef uint16_t halfStringPointer; // used to represent the bottom half of a pointer; safe to use with some static buffers.
extern int machineLog(const char* fmt, ...);

// using GNU typed enum extension
enum SCRATCH_fieldType : uint8_t {
    SCRATCH_BOOL,
    SCRATCH_NUMBER,
    SCRATCH_STRING,
    SCRATCH_SPRITE,
    SCRATCH_COLOR,
};

struct SCRATCH_sprite;
struct SCRATCH_thread;

union SCRATCH_field {
    bool boolean;
    uint16_t number;
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

    SCRATCH_loopInit,        // Push loop counter to loop stack
    SCRATCH_loopIncrement,   // Increment the top of stack loop counter
    SCRATCH_jumpIfRepeatDone,// Jump if the top of the loop stack has reached a value   @field loop value @field jump location

    SCRATCH_PARTITION_BEGINEXPRESSIONS, // Semantic partition.

    // Expression opcodes. May pop from the stack; always push to the stack.

    SCRATCH_fetch,           // Fetch some special value such as `x position`            @field which property to get
    SCRATCH_fetchFrom,       // Like above, but fetches from an aritrary sprite.         @input which sprite @field which property
    SCRATCH_loadVar,         // Load a variable                                          @field variable index
    SCRATCH_setVar,          // Set a variable                                           @field variable index @input data
    SCRATCH_loadVarFrom,     // Like above, but from an arbitrary sprite.                @input which sprite @field variable index
    SCRATCH_loadArrayAt,     // Load from an array                                       @field array name @input array position
    SCRATCH_push,            // Push argument                                            @field value

    SCRATCH_add,             // Add two top-of-stack values                              @input op1 @input op2


    SCRATCH_DEBUGEXPRESSION,

    SCRATCH_PARTITION_BEGINSTATEMENTS, // Semantic partition.
    // Only statement opcodes need to mind their return value. Still, for predictability, expression opcodes should return
    // SCRATCH_continue.

    // Statement opcodes. Statements always leave the stack empty, unless there has been an error in compilation or implementation.

    SCRATCH_loopJump,        // Signal a loop iteration to interpreter                   @field jump destination
    SCRATCH_joinString,      // For string join operations                               @input string1 @input string2
    SCRATCH_clone,           // Treat cloning as a privileged primitive operation        @input sprite index
    SCRATCH_jumpIf,          // Jump if top of stack is truthy                           @input evaluand @field jump destination
    SCRATCH_jump,            // Unconditional jump

    SCRATCH_motionGoto,
    SCRATCH_motionGlideto,
    SCRATCH_motion_glideIteration,
    SCRATCH_motionTurnright,
    SCRATCH_motionTurnleft,
    SCRATCH_motionMovesteps,
    SCRATCH_motionPointindirection,
    SCRATCH_motionPointtowards,
    SCRATCH_motionSetx,
    SCRATCH_motionChangexby,
    SCRATCH_motionSety,
    SCRATCH_motionChangeyby,
    SCRATCH_DEBUGSTATEMENT,

    SCRATCH_stop,
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


struct SCRATCH_stringRegister {
    char* base;
    int length;
    int capacity;
};

enum SCRATCH_EVENTTYPE : uint8_t {
    ONFLAG,
    ONKEY,
    ONCLICK, // ignored
    ONBACKDROP,
    ONLOUDNESS, // ignored
    ONMESSAGE,
};

union SCRATCH_eventInput { // redundant union; meant for semantic labeling
    uint16_t key;
    uint16_t backdrop;
    uint16_t loudness;
    uint16_t message;
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
};

struct SCRATCH_spriteHeader {
    scaledInt32 x;
    scaledInt32 y;
    uint16_t rotation; // Rotation maps (0 -> 360) to the entire range of a 16-bit integer
    bool visible;
    int8_t layer;
    uint8_t size;
    bool rotationStyle;
    uint8_t costumeIndex;
    uint8_t costumeMax;
    uint8_t threadCount;
    uint8_t variableCount;
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
#endif
