#include "programData.h"
#include "scratch.h"

struct SCRATCH_sprite* sprites[SPRITEMAX];
int spriteCount;
enum SCRATCH_opcode code[1024];
uint8_t* imageTable[IMAGEMAX];
