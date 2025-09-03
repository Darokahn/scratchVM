The roles of each of the source files in this directory:

scratch.c: Provide functions for iteration from one state to the next based on sprites and their operating threads.

graphics.c: Provide functions that use the current state of the program to generate command sets which can be dispatched as visual output.

IOtranslation.c: Provide the set of graphics commands as functions so that they may update the appropriate output source.

main.c: Provide all runtime-associated data for the individual program being run, and coordinate the runtime.

log.c: Provide an implementation for the logging function, allowing it to be agnostic to the platform being used.
