ScratchVM: Big Cobble Overview
===============================

What it is
----------
ScratchVM takes a Scratch project, compiles it into a compact binary, and flashes it onto an ESP32. There are three main surfaces:
- `webConverter/`: browser UI + Flask server to download/compile projects and push them over serial or via ESP Web Tools.
- `source/`: core C runtime for the Scratch VM build.
- `arduino/`: Arduino-focused build of the runtime with boards.txt/platform overrides.

High-level flow
---------------
1) User opens `webConverter/index.html` (or `patch-notes.html` / `future-updates.html`) and grabs a Scratch project via SBDL.
2) `index.mjs` calls `utils/compile.js` to translate the SB3 into packed bytecode and assets (`programData.bin`, `definitions.c`).
3) The compiled bytes go two ways:
   - Stored/uploaded on the Flask server (`/upload`).
   - Sent over Web Serial (`utils/serialTools.js`) to the device; alternatively, flashed via the `esp-web-install-button` manifest in `scratchVM-image/manifest.json`.
4) Device runs the C VM (`source/` or `arduino/`), which reads `programData` arrays and executes opcodes defined in `opcodeImpl.h` / `opcodeEnum.h`.

Web converter pieces (key files)
--------------------------------
- `index.html`: main UI; includes ESP Web Tools install button using `scratchVM-image/manifest.json`.
- `navigation.mjs`: builds nav links; now includes Patch Notes and Future Updates pages.
- `features.mjs`: shared loader that fetches documents from `/api/documents/{patches|planned_features}` and renders cards.
- `patch-notes.html`, `future-updates.html`: minimal pages that list server docs.
- `index.mjs`: orchestration for download/compile/send; state machine for UI buttons.
- `utils/compile.js`: converts SB3 to internal structures, outputs byte arrays and C definitions.
- `utils/serialTools.js`: Web Serial connect/send; 115200 baud; no explicit erase logic.
- `server.py`: Flask static host + JSON upload/report APIs + document listing for `patches/` and `planned_features/`.

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

Flashing notes
--------------
- Web flashing uses ESP Web Tools; it writes only listed partitions—no full-chip erase unless explicitly requested (not enabled here).
- Serial sending in the UI just streams bytes; actual flashing logic would need a bootloader writer if you bypass ESP Web Tools.

Where to put content
--------------------
- Patch notes: drop files in `webConverter/patches/`.
- Future updates: drop files in `webConverter/planned_features/`.
- Uploaded binaries: land in `webConverter/upload/` via the Flask endpoint.

Known UX constraints
--------------------
- If Web Serial fails, ensure Chrome/Edge, localhost/https, and no other app owns the port.
- If ESP Web Tools fails, try lowering speed (e.g., add `install-speed="115200"` to the install button) and ensure proper BOOT/EN wiring.

How to rebuild in words
-----------------------
- Start the Flask server (`python3 webConverter/server.py`) to serve static files and handle uploads.
- Open the web UI, fetch a Scratch project, compile, and push via serial or ESP Web Tools.
- On device, the C VM from `source/` or `arduino/` consumes `programData` and runs the project.


