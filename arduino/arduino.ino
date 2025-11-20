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

#include <Bonezegei_XPT2046.h>

#define TS_CS 33
#define TS_IRQ 32
Bonezegei_XPT2046 ts(TS_CS, TS_IRQ);
int cursorX = 0;
int cursorY = 0;

extern "C" {
  #include "scratch.h"
  #include "graphics.h"
  #include "letters.h"
}

extern "C" int main();
extern "C" void startIO() {
    Serial.begin(115200);
    tft.init();
    ts.begin();
    tft.setRotation(1);
    tftSprite.createSprite(LCDWIDTH, LCDHEIGHT);
    tft.fillScreen(TFT_BLACK);
    pinMode(33, INPUT);
    pinMode(32, INPUT);
    pinMode(25, INPUT_PULLUP);
}
extern "C" void updateIO(uint16_t* framebuffer) {
    tftSprite.pushSprite((FULLLCDWIDTH - LCDWIDTH) / 2, (FULLLCDHEIGHT - LCDHEIGHT) / 2);
    ts.getInput();
    if (ts.z > 500) {
        cursorX = LCDWIDTH - (ts.x * LCDWIDTH / 4000);
        cursorY = (ts.y * LCDHEIGHT / 4000);
    }
}

extern "C" void* mallocDMA(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_DMA);
}

extern "C" void drawPixel(int x, int y, pixel color) {
    tftSprite.drawPixel(x, y, color);
}

int frameInterval = 1000 / FRAMESPERSEC;

extern "C" const size_t stampSize = sizeof millis();

extern "C" unsigned long getNow() {
    return millis();
}

#define VRX_PIN 32
#define VRY_PIN 33
#define SW_PIN 25

const int ADC_CENTER = 1880;       // Midpoint of 12-bit ADC
const int DEADZONE  = 200;         // Joystick deadzone threshold

extern "C" bool getInput(int index) {
    int vrx = analogRead(VRX_PIN);
    int vry = analogRead(VRY_PIN);
    bool sw  = digitalRead(SW_PIN) == LOW;  // Active-low button

    switch (index) {
        case 0: // UP
            return vry > ADC_CENTER + DEADZONE;
        case 1: // RIGHT
            return vrx > ADC_CENTER + DEADZONE;
        case 2: // DOWN
            return vry < ADC_CENTER - DEADZONE;
        case 3: // LEFT
            return vrx > ADC_CENTER - DEADZONE;
        case 4: // SPACE / action button
            return sw;
    }
    return false;
}

extern "C" int machineLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = Serial.vprintf(fmt, args);
    va_end(args);
    return n;
}

void loop() {
}

void setup() {
    main();
}
