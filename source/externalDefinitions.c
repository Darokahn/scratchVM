#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>

#include "graphics.h"
#include "scratch.h"
#include "programData.h"
#include "externalDefinitions.h"

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
pixel* screen;

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
    screen = mallocDMA(LCDWIDTH * LCDHEIGHT * sizeof *screen);
}

void updateGraphics() {
    SDL_UpdateTexture(texture, NULL, screen, LCDWIDTH * sizeof(*screen));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) exit(0);
    }
    debugImage(screen, LCDWIDTH, LCDHEIGHT);
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

void drawSprites(struct SCRATCH_sprite** sprites, int spriteCount, const pixel** imageTable) {
    for (int i = 0; i < spriteCount; i++) {
        struct SCRATCH_sprite* sprite = sprites[i];
        const pixel* image = getImage(imageTable, i, sprite->base.costumeIndex);
        int imageResolution;
        if (i == 0) imageResolution = 128;
        else imageResolution = 32;
        int baseX = sprite->base.x.halves.high * WIDTHRATIO;
        int baseY = sprite->base.y.halves.high * HEIGHTRATIO;
        int width = ((float)sprite->base.widthRatio / 255) * LCDWIDTH;
        int height = ((float)sprite->base.heightRatio / 255) * LCDHEIGHT;
        float xStride = ((float)imageResolution) / width;
        float yStride = ((float)imageResolution) / height;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (y + baseY >= LCDHEIGHT || x + baseX >= LCDWIDTH || y + baseY < 0 || x + baseX < 0) continue;
                int row = (y * yStride);
                int index = ((row * imageResolution) + (x * xStride));
                pixel color = image[index];

                // transparent pixels reveal white if they are on the background; do nothing if they are on a sprite.
                if (color == 0) {
                    if (i == 0) color = (pixel) 0xffff;
                    else continue;
                }
                screen[(y + baseY ) * LCDWIDTH + x + baseX] = color;
            }
        }
    }
}

void debugImage(pixel *img, int width, int height) {
    uint8_t pixels[(width * height * 21) + (5 * 128) + 1];
    char* pixelPointer = pixels;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixel pixel = img[y * width + x];

            uint8_t r = (pixel >> 5) & 7;
            uint8_t g = (pixel >> 2) & 7;
            uint8_t b = pixel & 3;

            uint8_t R = (r * 255) / 7;
            uint8_t G = (g * 255) / 7;
            uint8_t B = (b * 255) / 3;

            pixelPointer += sprintf(pixelPointer, "\x1b[48;2;%u;%u;%um  ", R, G, B);
        }
        // Reset and newline
        pixelPointer += sprintf(pixelPointer, "\x1b[0m\n");
    }
    puts(pixels);
}
