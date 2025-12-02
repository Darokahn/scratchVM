#ifndef EXTERNALDEFINITIONS_H
#define EXTERNALDEFINITIONS_H

#include <stdint.h>
#include <stdio.h>

#include "graphics.h"
#include "scratch.h"
#include "externGlobals.h"

void startIO();
void updateIO();
int machineLog(const char* fmt, ...);

void* mallocDMA(size_t size);

void drawSprites(struct SCRATCH_spriteContext* context);
void debugImage(struct image *img, int width, int height);

bool getInput(int inputIndex);

#define MINFAMSIZE 16


// The data necessary for either the desktop or an embedded system to maintain the assets for an app
// Allows extra data, such as the file the app was taken from
typedef struct {
    char name[128];
    uint8_t* programData;
} app_t;

void pollApp(app_t* out);

void selectApp(app_t* out, char* appName);

#define APP_FREE 1
// potentially more flags later

void closeApp(app_t* app, int flags);

#endif
