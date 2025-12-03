#pragma once
#include "scratch.h"
#include "graphics.h"

typedef struct {
    char name[128];
    uint8_t* programData;
} app_t;

void startIO();
void updateIO(app_t* app);
int machineLog(const char* fmt, ...);

void drawSprites(struct SCRATCH_spriteContext* context);
void debugImage(struct image *img, int width, int height);

#define APP_FREE 1
// potentially more flags later

bool getInput(int inputIndex);
void pollApp(app_t* out);
void selectApp(app_t* out, char* appName);
void closeApp(app_t* app, int flags);


