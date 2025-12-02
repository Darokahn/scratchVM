#include <stdbool.h>
#include <stdint.h>

struct _vecInt {
    int length;
    int capacity;
    bool staticAllocated;
    int data[];
};

#define STATICMIN 32

typedef struct {
    struct _vecInt vec;
    int data[STATICMIN];
} vecIntStatic;

// an important nuance of the system: vecInt primitive must be a pointer, or else functions cannot move it silently.
typedef struct _vecInt* vecInt;

void vecIntInit(vecInt* localStorage) {
    // initialize vecInt to point to the local storage
}

void vecPush(vecInt* vec, int value) {
    // push the value; if this would exceed the space, retarget vec to point to a new heap allocation. Caller does not know this happened unless it checks whether the pointer has been reassigned.
}

// misc other vector functions

// optional macro to hide what's going on under the hood
#define newVec(name) vecIntStatic __##name; vecInt name = (vecInt)&__##name; vecIntInit(&name);

int main() {
    newVec(myvec);
}
