#include "scratch.h"

const enum SCRATCH_opcode codeTemplate[] = {
    SCRATCH_push, 1, 0x41, 0xff,
    SCRATCH_push, 1, 0x0, 0x0,
    SCRATCH_motionGoto,
    SCRATCH_push, 1, 0xa, 0x0,
    SCRATCH_setVar, 0x0, 0x0,
    SCRATCH_push, 0x1, 0x0, 0x0,
    SCRATCH_setVar, 0x1, 0x0,
    // 1:
    SCRATCH_fetchInput, 0x0, 0x0,
    SCRATCH_jumpIfNot, LABEL2, 0x0,
    SCRATCH_loadVar, 0x0, 0x0,
    SCRATCH_incVar, 0x1, 0x0,
    // 2:
    SCRATCH_fetchInput, 0x2, 0x0,
    SCRATCH_jumpIfNot, LABEL3, 0x0,
    SCRATCH_push, 1, 0x0, 0x0,
    SCRATCH_loadVar, 0x0, 0x0,
    SCRATCH_sub,
    SCRATCH_incVar, 0x1, 0x0,
    // 3:
    SCRATCH_push, 1, 0x41, 0xff,
    SCRATCH_loadVar, 0x1, 0x0,
    SCRATCH_goto,
    SCRATCH_loopJump, LABEL1, 0x0,
};

