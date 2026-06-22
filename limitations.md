# There is a need for a single place to store the limitations of this engine, so they can be communicated clearly when used in (for example) an educational context.

## I will differentiate the limitations which are only temporary and which I intend on improving, from the ones which are intended and out of scope to fix.

### IN SCOPE:

- ARGUMENT UNFAITHFULNESS: Scratch has a host of arguments which are typically selected from a drop-down, but may also accept an argument. These have a round shape in the gui. This is different from the square-shaped dropdowns, which are "fields" and cannot be selected using an argument. The drop-down bubbles will evaluate their string argument for a match to any of their options. The current engine, which was designed without even knowing about this feature, compiles strings into a corresponding numeric option before runtime ever touches them. The solution will be string interning, and translating those numeric options into address identities.
- RESOLUTION: The resolution will always be limited for performance reasons on the esp32, but I intend to at least improve the options available for resolution.
- VISUAL EFFECTS: There are currently no supported visual effects; and moreover, rotation is not shown visually (as if sprites always had the right-left rotation option selected). I intend to draw and cache modified sprites into a buffer in memory, with limited real estate.
- COLLISIONS: The engine currently only checks rectangle overlap, which is different from Scratch's per-pixel collision. I intend to allow the user to select per-pixel collision at compilation.
- GENERAL INACCURACY: While I have taken very special care to make this engine recreate Scratch behavior as faithfully as possible, there are inherent desynchronizations between this C code base and the Scratch Javascript code base. In particular, Scratch relies somewhat on the graphical engine for feedback. However, these inaccuracies are all fixable with yet more special care.

### OUT OF SCOPE:

- PERFORMANCE: The engine performs about as well as I need it to, and it runs simple games at a playable speed, on esp32, even when a vast majority of compute time is in expensive CPU drawing and SPI transfer.
- SPRITE LIMIT: 256 sprites has been plenty as far as I have tested, and performance/memory typically cause unplayability/crash before it is reached on the esp32.
- SOUND: The engine is currently silent. This may be migrated one day to the in-scope features, but sound adds a major tax to the memory and CPU requirements of the engine.
