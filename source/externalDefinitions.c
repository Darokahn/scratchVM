#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>

#include "graphics.h"

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;

void startGraphics() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(
            "Scratch Project",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            LCDWIDTH, LCDHEIGHT,
            0
            );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        LCDWIDTH, LCDHEIGHT
    );
}

void updateGraphics(uint16_t* framebuffer) {
    SDL_UpdateTexture(texture, NULL, framebuffer, LCDWIDTH * sizeof(*framebuffer));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Event e;
    SDL_PollEvent(&e);
}

__attribute__((naked))
int machineLog(const char* fmt, ...) {
    goto *&printf; // GCC only
}

void* mallocDMA(size_t size) {
    return malloc(size);
}
