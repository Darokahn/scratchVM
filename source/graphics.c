#include "scratch.h"
#include <stdint.h>

#define SCREENWIDTH 480
#define SCREENHEIGHT 360

uint8_t screen1[SCREENHEIGHT][SCREENWIDTH];
uint8_t screen2[SCREENHEIGHT][SCREENWIDTH];

void drawSprites(struct SCRATCH_sprite** sprites, int spriteCount, 
