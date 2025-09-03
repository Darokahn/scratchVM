#include "scratch.h"
#include <stdio.h>

int main() {
    printf("%d\n", __alignof__(struct SCRATCH_data));
    printf("%d\n", __alignof__(struct SCRATCH_thread));
}
