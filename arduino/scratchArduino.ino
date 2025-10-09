/*
I WOULD RATHER CODE USING A KEYBOARD MADE OF MY OWN TOENAILS AND FINGERNAILS THAT I HAD TO RIP OFF OF MY BODY
AND GINGERLY PLACE ONTO THE KEYCAPS THAN MAKE AN ENTIRE PROJECT IN C++
*/

#pragma GCC optimize ("O3")

#include <cstdarg>
#include <cstdio>
#include <TFT_eSPI.h>

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite tftSprite = TFT_eSprite(&tft);

extern "C" {
  #include "scratch.h"
  #include "graphics.h"
  #include "programData.h"
}

extern "C" int main();
extern "C" void startGraphics() {
    tft.init();
    tft.setRotation(1);
    tftSprite.createSprite(LCDWIDTH, LCDHEIGHT);
    tft.fillScreen(TFT_WHITE);
}
extern "C" void updateGraphics(uint16_t* framebuffer) {
    tftSprite.pushSprite(0, 0);
}
extern "C" void* mallocDMA(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_DMA);
}
extern "C" void drawSprites(struct SCRATCH_sprite** sprites, int spriteCount, const pixel** imageTable) {
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
        machineLog("WIDTHRATIO: %f, HEIGHTRATIO: %f\n", WIDTHRATIO, HEIGHTRATIO);
        machineLog("x, y: %d, %d\n", sprite->base.x.halves.high, sprite->base.y.halves.high);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (y + baseY >= LCDHEIGHT || x + baseX >= LCDWIDTH || y + baseY < 0 || x + baseX < 0) continue;
                int row = (y * yStride);
                int index = ((row * imageResolution) + (x * xStride));
                pixel color = image[index];
                // transparent pixels reveal white if they are on the background; do nothing if they are on a sprite.
                if (color == 0) {
                    if (i == 0) color = 0xff;
                    else continue;
                }
                tftSprite.drawPixel(x, y, color);
            }
        }
    }
}


extern "C" int machineLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = Serial.vprintf(fmt, args);
    va_end(args);
    return n;
}
void loop() {}

void setup() {
    Serial.begin(115200);

    main();
    Serial.println("TFT_MISO = " + String(TFT_MISO));
    Serial.println("TFT_MOSI = " + String(TFT_MOSI));
    Serial.println("TFT_SCLK = " + String(TFT_SCLK));
    Serial.println("TFT_CS   = " + String(TFT_CS));
    Serial.println("TFT_DC   = " + String(TFT_DC));
    Serial.println("TFT_RST  = " + String(TFT_RST));

}
