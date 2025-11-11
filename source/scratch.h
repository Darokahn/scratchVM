#ifndef SCRATCH_H
#define SCRATCH_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "opcodeEnum.h"
#include "graphics.h"

#define STACKMAX (128)
#define MAXOPCODE (255)
#define LOOPNESTMAX (16) // deepest nesting for `repeat x` loops before failure
#define SPRITEMAX (8) // maximum spritecount
#define IMAGEMAX (16)
#define ALIGN8(ptr) ((void*) (((uint64_t) ptr + 7) & ~7))


#define FRAMESPERSEC 30

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

struct SCRATCH_sprite;
struct SCRATCH_thread;

enum SCRATCH_fieldType {
    SCRATCH_BOOL,
    SCRATCH_NUMBER,
    SCRATCH_DEGREES,
    SCRATCH_STRING,
    SCRATCH_ID,
};

union SCRATCH_field {
    scaledInt32 number;
    uint16_t degrees;
    bool boolean;
    char* string;
    uint8_t id;
};

struct SCRATCH_data {
    uint8_t type;
    union SCRATCH_field data;
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
};

typedef bool (*SCRATCH_functionIterator)(struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, int iteration);
typedef enum SCRATCH_continueStatus (*SCRATCH_function)(struct SCRATCH_sprite* sprite, struct SCRATCH_data* stack, int* stackIndex, struct SCRATCH_thread* thread);
// SCRATCH_functionIterator is used for animated blocks. A SCRATCH_function that returns non-null signals to the thread
// that this block should yield and use the returned enumeration to set a new current task.


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

union SCRATCH_stepSize {
    uint16_t value;
    uint8_t bytes[2];
};

struct SCRATCH_glideData {
    scaledInt32 stepX;
    scaledInt32 stepY;
    uint16_t remainingIterations; // how many times to iterate
    int16_t targetX; // what to set position to when done (assume slight error due to rounding)
    int16_t targetY;
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
    uint8_t startEvent;
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

enum SCRATCH_continueStatus SCRATCH_processBlock(struct SCRATCH_sprite* sprite, struct SCRATCH_thread* thread, const pixel* imageTable[]);
void SCRATCH_processThread(struct SCRATCH_sprite* sprite, struct SCRATCH_thread* thread, const pixel* imageTable[]);
int SCRATCH_visitAllThreads(struct SCRATCH_sprite** sprites, int spriteCount, const pixel* imageTable[]);
struct SCRATCH_sprite* SCRATCH_makeNewSprite(struct SCRATCH_spriteHeader header);
void SCRATCH_initThread(struct SCRATCH_thread*, struct SCRATCH_threadHeader);
void handleInputs();
void clearEvents();
bool SCRATCH_addSprite(struct SCRATCH_sprite* sprite);
bool SCRATCH_wakeSprite(struct SCRATCH_sprite* sprite, enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input);
void SCRATCH_wakeSprites();

bool getEvent(enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input);
void setEvent(enum SCRATCH_EVENTTYPE type, union SCRATCH_eventInput input, bool state);

struct image* getImage(const pixel* images[IMAGEMAX], int spriteIndex, int costumeIndex);

void initData(const struct SCRATCH_header header, struct SCRATCH_sprite* sprites[SPRITEMAX]);
void initImages(const uint8_t* buffer, const pixel* images[IMAGEMAX]);
struct SCRATCH_rect getRect(struct SCRATCH_sprite* s, const pixel* imageTable[]);
bool rectsCollide(struct SCRATCH_rect r1, struct SCRATCH_rect r2);

#endif
