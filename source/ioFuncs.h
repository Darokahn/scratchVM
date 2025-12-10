#pragma once
#include "scratch.h"
#include "graphics.h"

typedef struct {
    int programDataSize;
    uint8_t* programData;
    char name[12];
} app_t;

void startIO();
int updateIO(app_t* app);
int machineLog(const char* fmt, ...);

void drawSprites(struct SCRATCH_spriteContext* context);
void debugImage(struct image *img, int width, int height);

#define APP_FREE 1
// potentially more flags later

bool getInput(int inputIndex);
void* pollApp(char* nameOut);
int selectApp(app_t* out, char* appName);
void loadApp(app_t* out, int offset);
void closeApp(app_t* app, int flags);





