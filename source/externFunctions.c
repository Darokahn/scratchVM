#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "graphics.h"
#include "scratch.h"
#include "ioFuncs.h"
#include "programData.h"

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
pixel* screen;

void startIO() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(
            "Scratch Project",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            LCDWIDTH * 3, LCDHEIGHT * 3,
            0
            );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, LCDWIDTH, LCDHEIGHT);

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        LCDWIDTH, LCDHEIGHT
    );
    screen = malloc(LCDWIDTH * LCDHEIGHT * sizeof *screen);
}

int cursorX = 0;
int cursorY = 0;
int updateIO(app_t* app) {
    SDL_UpdateTexture(texture, NULL, screen, LCDWIDTH * sizeof(*screen));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) exit(0);
    }
    return 0;
}

int machineLog(const char* format, ...) {
    int result;
    va_list args;
    va_start(args, format);

    // Forward all arguments to printf
    result = vprintf(format, args);

    va_end(args);
    return result;
}

void* mallocDMA(size_t size) {
    return malloc(size);
}

void drawPixel(int x, int y, pixel color) {
    if (y < 0 || y >= LCDHEIGHT || x < 0 || x >= LCDWIDTH) return;
    screen[y * LCDWIDTH + x] = color;
}

void debugImage(struct image *imgObj, int width, int height) {
    pixel* img = imgObj->pixels;
    char pixels[(width * height * 21) + (5 * 128) + 1];
    char* pixelPointer = (char*) &pixels;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixel pixel = img[y * width + x];

            uint8_t r = (pixel >> 11) & 31;
            uint8_t g = (pixel >> 5) & 63;
            uint8_t b = pixel & 31;

            uint8_t R = (r << 3) | (r >> 2); // replicate high bits to low
            uint8_t G = (g << 2) | (g >> 4);
            uint8_t B = (b << 3) | (b >> 2);

            pixelPointer += sprintf(pixelPointer, "\x1b[48;2;%u;%u;%um  ", R, G, B);
        }
        // Reset and newline
        pixelPointer += sprintf(pixelPointer, "\x1b[0m\n");
    }
    puts(pixels);
}

int frameInterval = 1000 / FRAMESPERSEC;

uint64_t getNow() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t ms = ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000;
    return ms;
}

const uint8_t *keyboardState;
bool getInput(int index) {
    int numkeys;
    keyboardState = SDL_GetKeyboardState(&numkeys);
    switch (index) {
        case 0: return keyboardState[SDL_SCANCODE_UP];
        case 1: return keyboardState[SDL_SCANCODE_LEFT];
        case 2: return keyboardState[SDL_SCANCODE_DOWN];
        case 3: return keyboardState[SDL_SCANCODE_RIGHT];
        case 4: return keyboardState[SDL_SCANCODE_SPACE];
    }
    return 0;
}

void* pollApp(char* nameOut) {
}

int selectApp(app_t* out, char* appName) {
    struct dataHeader h;
    uint8_t magic[9];
    magic[8] = 0;
    int fd = open("source/program.bin", O_RDONLY);
    read(fd, &magic, 8);
    read(fd, &h, sizeof h);
    int size = h.dataSize;
    uint8_t* programData = malloc(size);
    lseek(fd, 8, SEEK_SET);
    read(fd, programData, size);
    strcpy(out->name, "app");
    out->programData = programData;
    close(fd);
}

void closeApp(app_t* app, int flags) {
    free(app->programData);
}
