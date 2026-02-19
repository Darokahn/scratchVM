How does function calling work in this scratch clone engine?

Functions in Scratch are called "My blocks".

The engine is written in C.

First, understand that the organization of executing/scheduled modules is root -> several sprites -> several threads.

Execution is divided into "statements". Each statement starts and ends with an empty stack, so there is no need for a persistent stack.

A few other members of state are necessary, such as when the sprite is performing a task which is animated across several frames. This is modeled generically, though there are really only two instances: waiting and gliding.

These state variables go onto a per-thread stack. The stack is an uncomplicated, naively-implemented `SCRATCH_vector` implementation. The structure holds the generic `SCRATCH_data` type. The same `SCRATCH_vector` type is used to support list-type variables. This is slightly overengineered for the two tasks that require thread-local storage, as they cannot occur concurrently. A single generic storage slot could be configured to store the data for either task. This is indeed the design that was used before a minor rewrite.

The reason for this rewrite is to keep thread-local storage in a single spot while also supporting function calls, the third and final reason to have thread-local storage. Given function calls and the option for arbitrarily deep recursion, a single stack is a sensible choice.

Other persistent storage has its own designated system, which is the sprite-local (NOT thread-local) variables. While variables are sprite-local, the background sprite's variables are exposed to each sprite. This could be modeled generically that each nested sprite has access to its parent's variables, but scratch only allows background->all others nesting.

Note that functions are somewhat "tagged-on", and there are not formal locals or local declarations. Therefore, functions do not need a mechanism for locals beyond their arguments. All variables are global and accessible at any point in the program.

Enough static analysis could potentially insert semantic declarations if it notices the global variables are used in this manner. In this case, automatic storage and a stack could be employed. However, this is an unnecessary convolution.

Now that the mechanism for thread-local storage is understood, the details of function dispatch can be understood.

Functions are dispatched by simulating a textbook ABI. They are instantiated at an arbitrary spot in the text, and called by pushing their arguments to thread-local storage (treating the return address as an argument), and then jumping to that arbitrary spot in text. When the function refers to local variables, the compiler emits a lookup to the top of the stack at a baked-in offset. When the text of the function reaches a return, it should jump back to the value held at the top of the stack and pop all of its arguments.


Because there is room to, I would like to expound on a more convoluted calling architecture that was considered before being crossed off for its complexity. Its goal was to keep the bytecode executor in its same state and implement function calls as generic extensions of its existing features.

First, understand that a vast majority of functions are inlinable. When indirect calling isn't an option (as it is not in Scratch), the only case that is not simply inlinable is recursion. This does not make it *un-inlinable*, but it does complicate things.

When you implement the complicated inlining procedure for recursion and eliminate the need to support indirect calling, there is no actual necessity for a call ABI.

Note that for a whole-program compiler, in fact there is never an actual necessity for a call ABI. A whole-program compiler can see every time you take the address of a function. Calling an address that is not taken from the address of a function must always be undefined, so the compiler does not need to support it. Each indirect call (calling a function pointer) can be a switch statement between all of the functions whose addresses are taken. Static analysis can reduce this range further. Taking the address of a function can even just evaluate to a comptime enumerated integer, with the rule that this is a special type facading as a pointer. This allows the switch statement to be compact and inside a small integer range.

Recursion tends to simply be a way to implement a stack using builtin language constructs; essentially, a way to have the language do the work of a stack for you.

Tail call optimization is a good place to start. For simple recursion, the return argument of the call stack will be repeated many times. In fact, many times only the top frame is actually necessary, and prior frames will not be returned to in a meaningful way. This case does *not* take advantage of the stack, so it doesn't need one. The arguments can have a fixed address which is overwritten.

When a function is inlined, it actually knows exactly where to return at the end of most code paths. This is a simple jump, not a return instruction.

There are some shapes of recursion which actually take advantage of the stack data structure. These are the only cases I consider to be "Truly Recursive" (refactoring them into a loop is non-trivial). The way to refactor them into a loop is to give them a stack for each argument that the function comes back to *after* recurring.

So by giving each inlined instance of a recursive function dedicated per-thread variables (that live in the same space as all of the sprite's variables), you can implement all calls just fine. The minor stipulation is that the argument variable will sometimes need to be a stack type. This removes the inherent stack from recursion and re-inserts it where it's actually necessary.

