#ifndef EXTERNGLOBALS_H
#define EXTERNGLOBALS_H

extern const struct SCRATCH_header header;
extern const uint8_t programData[];
extern enum SCRATCH_opcode* code;
extern bool events[];
extern int eventTypeOffsets[3];
extern bool inputState[5];

#endif
