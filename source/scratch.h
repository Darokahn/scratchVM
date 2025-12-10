#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "graphics.h"
#include "opcodeEnum.h"

#define STACKMAX (128)
#define MAXOPCODE (255)
#define LOOPNESTMAX (16) // deepest nesting for `repeat x` loops before failure
#define SPRITEMAX (256) // maximum spritecount
#define IMAGEMAX (256)
#define ALIGN8(ptr) ((void*) (((uint64_t) ptr + 7) & ~7))

#define FRAMESPERSEC 30

#define STAGERESOLUTION 128
#define SPRITERESOLUTION 32

#define PI ((float)3.14159265358979323846264338279)

#define halfRotation ((float)((uint64_t)UINT32_MAX + 1) / 2)
#define quarterRotation (halfRotation/2)
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

struct SCRATCH_sprite;
struct SCRATCH_thread;

enum SCRATCH_fieldType {
    SCRATCH_FRACTION,
    SCRATCH_WHOLENUMBER,
    SCRATCH_DEGREES,
    SCRATCH_BOOL,
    SCRATCH_STRING,
    SCRATCH_STATICSTRING,
    SCRATCH_LIST,
    SCRATCH_UNINIT,
};

struct SCRATCH_vector {
    struct SCRATCH_data* data;
    uint16_t capacity;
    uint16_t count;
};


union SCRATCH_field {
    scaledInt32 number;
    uint32_t degrees;
    int32_t wholeNumber;
    bool boolean;
    char* string;
    struct SCRATCH_vector* list;
};

struct SCRATCH_data {
    union SCRATCH_field data;
    uint8_t type;
};

struct SCRATCH_rect {
    int x;
    int y;
    int width;
    int height;
};

struct SCRATCH_list {
    struct SCRATCH_data* items;
    int itemCount;
    int itemCapacity;
};

// values indicating which dynamic value to fetch
enum SCRATCH_fetchValue {
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
    SCRATCH_yieldLogic,
    SCRATCH_killThread,
    SCRATCH_killOtherThreads,
    SCRATCH_killAllThreads,
    SCRATCH_killSprite
};

typedef enum SCRATCH_continueStatus (*SCRATCH_function)(struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, struct SCRATCH_thread* thread);

enum SCRATCH_EVENTTYPE {
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
    uint8_t type;
    union SCRATCH_eventInput input;
};

struct SCRATCH_spriteHeader {
    scaledInt32 x;
    scaledInt32 y;
    uint32_t rotation;  // Rotation maps (0 -> 360) to the entire range of a 16-bit integer
    uint16_t size;      // size maps (0 -> 100) to (0 -> SIZERATIO), which is 1024 at the time of writing.
    bool visible;
    int8_t layer;
    uint8_t rotationStyle;
    uint8_t costumeIndex;
    uint8_t costumeMax;
    uint8_t threadCount;
    uint8_t variableCount;
    uint8_t listCount;
    uint8_t id;
};

struct SCRATCH_threadHeader {
    union SCRATCH_eventInput eventCondition;
    uint16_t entryPoint;
    uint8_t startEvent;
};

// thread has local context, but each instance of thread context lives in the global variable store of its sprite owner with a unique index.
struct SCRATCH_thread {
    struct SCRATCH_threadHeader base;
    struct SCRATCH_vector threadLocals;
    uint16_t programCounter;
    bool active;
};

// a SCRATCH_sprite is designed to sit in a single heap allocation. `variables` should point to the next SCRATCH_data aligned address after the end of `threads`, then `lists` should go after that.
// potential future memory saving: put loop counter variables inside `thread` as bare integers, saving the space overhead of a whole `struct SCRATCH_data` per loop counter.
// likely not worthwhile as it adds memory model compexity to support many instances of `repeat(n)`, which are uncommon.
struct SCRATCH_sprite {
    struct SCRATCH_spriteHeader base;
    char* talkingString;
    // sprite-local storage (global variables: anything declared by the variable object)
    struct SCRATCH_data* variables;     // static storage for all variables.
    struct SCRATCH_vector* lists;       // static storage for any lists pointed to by `variables`. Lists are the exception to the "single heap allocation" rule, as they may point to separate allocations.
    struct SCRATCH_thread threads[];    // static storage for all threads.
};

struct SCRATCH_spriteContext {
    struct SCRATCH_sprite** sprites;
    struct image** imageTable;
    uint16_t* spriteSetIndices;
    struct SCRATCH_sprite* stage;
    uint16_t spriteCount;
    uint16_t currentIndex;
};

enum SCRATCH_continueStatus SCRATCH_processBlock(struct SCRATCH_spriteContext* context, struct SCRATCH_thread* thread, uint8_t* code);
enum SCRATCH_continueStatus SCRATCH_processThread(struct SCRATCH_spriteContext* context, struct SCRATCH_thread* thread, uint8_t* code);
int SCRATCH_visitAllThreads(struct SCRATCH_spriteContext* context, uint8_t* code);
struct SCRATCH_sprite* SCRATCH_makeNewSprite(struct SCRATCH_spriteHeader header, const struct SCRATCH_threadHeader* threads);
void SCRATCH_freeSprite(struct SCRATCH_sprite* sprite);
struct SCRATCH_sprite* SCRATCH_cloneSprite(struct SCRATCH_sprite* template);
void SCRATCH_freeSprites(struct SCRATCH_spriteContext* context);
void SCRATCH_initThread(struct SCRATCH_thread*, struct SCRATCH_threadHeader);
void SCRATCH_vectorInit(struct SCRATCH_vector* vector);
void SCRATCH_vectorPush(struct SCRATCH_vector* vector, struct SCRATCH_data data);
struct SCRATCH_data* SCRATCH_vectorTop(struct SCRATCH_vector* vector);
struct SCRATCH_data* SCRATCH_vectorFromTop(struct SCRATCH_vector* vector, uint16_t index);
struct SCRATCH_data* SCRATCH_vectorAt(struct SCRATCH_vector* vector, uint16_t index);
struct SCRATCH_data SCRATCH_vectorPop(struct SCRATCH_vector* vector);
void handleInputs();
void clearEvents();
bool SCRATCH_addSprite(struct SCRATCH_spriteContext* context, struct SCRATCH_sprite* sprite);
bool SCRATCH_wakeSprite(struct SCRATCH_sprite* sprite, enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input);
void SCRATCH_wakeSprites(struct SCRATCH_spriteContext* context);



struct SCRATCH_rect getRect(struct SCRATCH_spriteContext* context, struct SCRATCH_sprite* operand);
bool rectsCollide(struct SCRATCH_rect r1, struct SCRATCH_rect r2);

struct SCRATCH_data cast(struct SCRATCH_data d, enum SCRATCH_fieldType type, char* stringBuffer);


