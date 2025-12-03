This project compiles and interprets scratch projects. It compiles them into a bytecode that imagines a particular machine, whose specifications are satisfied by the mechanics of the interpreter. Its mechanics are as follows:

The machine operates in terms of sprites, which own each point of data necessary for the machine's function.
Sprites own their visual and positional data, such as coordinates, rotation, costume index, costume maximum, visibility status, and visibility layer. The actual image data for the sprite is not stored inside this structure. The virtual machine assumes an external picture processor, and exposes endpoints for this picture processor's use.
Sprites own the data associated with their runtime. Any variables used in a sprite's script are owned by that sprite in a heap area.Each variable is a type-agnostic data point in an array of data points.
Sprites own the threads associated with their operation. Each thread is a container owning the code it runs, the current program counter, and the data associated with the runtime.
The machine is capable of creating and running arbitrarily many threads concurrently.
Each thread uses a stack-based runtime. Its operations are composed of VM primitives, which can accept code-embedded constants as arguments, and scratch block functions, which do not take code-embedded arguments and look to the stack to consume arguments. The VM primitives are involved in pushing values to the stack for use by block functions, performing control flow operations, and utilizing the system string registers for join operations.
The machine has two string registers associated with each thread, which are boundless character array buffers used in string joining. For this purpose, joining as a special case is a VM primitive operation rather than a scratch block function.
# ScratchVMSite
