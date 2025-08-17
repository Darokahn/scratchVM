#ifndef SCRATCH_H
#define SCRATCH_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define STACKMAX 128
#define STRINGREGISTERMAX 4096
#define MAXOPCODE 256
#define LOOPNESTMAX 16 // deepest nesting for `repeat x` loops before failure
#define SPRITEMAX 256 // maximum spritecount
#define VARIABLEMAX 2040 // maximum variable count for whole project
#define THREADRATIO 4 // ratio of threads to sprites; Either each sprite has this many or amount is dispersed unevenly.

extern int THREADMAX;
extern void machineLog(const char* fmt, ...);
typedef uint16_t halfStringPointer; // used to represent the bottom half of a pointer; safe to use with some static buffers.

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
    int number;
    halfStringPointer string; // can either be datasegment-allocated, variable-allocated (heap), or string buffer-allocated (stack)
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
    SCRATCH_BEGINLOOPCONTROL, // Semantic partition.

    // Loop opcodes. 

    SCRATCH_loopJump,        // Interpreter needs to know when a loop iteration finishes for sequencing
    SCRATCH_loopInit,        // Push loop counter to loop stack
    SCRATCH_loopIncrement,   // Increment the top of stack loop counter
    SCRATCH_loopDestroy,     // Destroy the top of stack loop counter
    SCRATCH_jumpIfRepeat,     // Jump if the top of the loop stack has reached a value   @field which value to compare against

    SCRATCH_BEGINEXPRESSIONS, // Semantic partition.

    // Expression opcodes. May pop from the stack; always push to the stack.

    SCRATCH_fetch,           // Fetch some special value such as `x position`            @field which property to get
    SCRATCH_loadVar,         // Load a variable                                          @field variable index
    SCRATCH_loadArrayAt,     // Load from an array                                       @field array name @input array position
    SCRATCH_push,            // Push argument                                            @field value

    SCRATCH_BEGINSTATEMENTS, // Semantic partition.

    // Statement opcodes. Statements always leave the stack empty, unless there has been an error in compilation or implementation.

    SCRATCH_joinString,      // For string join operations                               @input string1 @input string2
    SCRATCH_clone,           // Treat cloning as a privileged primitive operation        @input sprite index
    SCRATCH_jumpIf,          // Jump if top of stack is truthy                           @input condition
    SCRATCH_jump,            // Unconditional jump

    SCRATCH_motionGoto,
};

// Signals a block gives the interpreter about how to continue the sequence
enum SCRATCH_continueStatus {
    SCRATCH_continue,
    SCRATCH_yieldGeneric,
    SCRATCH_yieldGlide, // specific case for animated gliding
    SCRATCH_yieldWait, // specific case for waiting
};

typedef bool (*SCRATCH_functionIterator)(struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, int iteration);
typedef enum SCRATCH_continueStatus (*SCRATCH_function)(struct SCRATCH_sprite* stage, struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, struct SCRATCH_thread* thread);
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
    ONCLICK,
    ONBACKDROP,
    ONLOUDNESS,
    ONMESSAGE,
    ONCLONE
};

union SCRATCH_eventInput { // redundant union; meant for semantic labeling
    int key;
    int backdrop;
    int loudness;
    int message;
};

struct SCRATCH_thread {
    bool active;
    enum SCRATCH_EVENTTYPE startEvent;
    union SCRATCH_eventInput eventCondition;
    enum SCRATCH_opcode* code; // bytecode entry point for this thread
    int programCounter; // pc
    int instructionLength;
    int loopCounterStack[LOOPNESTMAX];
    uint8_t loopCounterStackIndex;
    enum SCRATCH_continueStatus currentOperation;
    union {
        struct {
            int8_t stepsizeX; // how far to change position each iteration
            int8_t stepsizeY;
            int remainingIterations; // how many times to iterate
            int targetX; // what to set position to when done (assume slight error due to rounding)
            int targetY;
        } glideData;
        struct {
            int remainingIterations;
        } waitData;
    } operationData;
};

struct SCRATCH_sprite {
    struct SCRATCH_thread* threads; // storage of all threads
    uint8_t threadCount;
    struct SCRATCH_data* variables; // for data storage
    struct SCRATCH_list* lists; // for list-data storage

    // looks-related data
    bool visible;
    int8_t layer;
    int x;
    int y;
    float size;
    float rotation;
    uint8_t costumeIndex;
    uint8_t costumeMax;
};

void SCRATCH_processBlock(struct SCRATCH_sprite* stage, struct SCRATCH_sprite* sprite, struct SCRATCH_thread* thread);
#endif
