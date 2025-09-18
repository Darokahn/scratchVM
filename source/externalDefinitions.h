#ifndef EXTERNALDEFINITIONS_H
#define EXTERNALDEFINITIONS_H

#include <stdint.h>

int machineLog(const char* fmt, ...);

void startGraphics();
void updateGraphics(uint16_t* framebuffer);

void* mallocDMA(size_t size);

#endif
