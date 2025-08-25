#include <stdio.h>

__attribute__((naked))
int machineLog(const char* fmt, ...) {
    goto *&printf;
}
