ScratchVM: Overview
===============================

What it is
----------
ScratchVM takes a Scratch project, compiles it into a compact binary, and flashes it onto an ESP32. There are two main surfaces:
- `source/`: core C runtime for the Scratch VM build.
- `arduino/`: Arduino-focused build of the runtime with boards.txt/platform overrides.

Runtime (source/)
-----------------
- `main.c`: entry point wiring runtime, graphics, IO.
- `scratch.c` / `scratch.h`: core VM loop.
- `opcodeImpl.h` + `opcodeEnum.h`: opcode definitions and handlers.
- `programData.c` / `.h`: compiled Scratch payload.
- `graphics.h`, `drawing.c`, `letters.c`: rendering helpers.
- `ioFuncs.h`, `externFunctions.c`: platform hooks.
- `globals.h`, `big.h`: shared structs/constants.
- `program.bin`, `definitions.c`: sample compiled artifacts.

Arduino variant (arduino/)
--------------------------
- Mirrors `source/` with `arduino.ino` as the sketch wrapper.
- `firmwareData.h`: packaged firmware data for Arduino build.
- `partitions.csv`: flash layout for the Arduino build.
- `platform.local.txt`: platform overrides (e.g., build flags).

