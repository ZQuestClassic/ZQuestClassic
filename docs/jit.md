# JIT

JIT compilation is off by default. It can be enabled by setting the `[ZSCRIPT] jit = 1` conifg option, found in the launcher. There is also the `-j` command line switch, or `--jit` for `run_replay_tests.py`.

`[ZSCRIPT] jit_precompile = 1` can be set to compile all scripts on quest load, instead of as they are encountered.

## How it works

First, details on how ZASM is normally interpreted:

`run_script` sets up the current script object (which contains the ZASM instructions), and the proper context for that script (refInfo and the stack). Normally, it then calls `run_script_int` which enters a loop that processes one ZASM instruction at a time, based on the current instruction counter `ri->pc`. Based on the instruction command, it will have 0-2 args; and may modify the state of either the stack, some "z-register", or some game object.

A "z-register" (in the code, `ffscript.cpp` calls them registers, but the AST parser code calls them Vars) is set/read via `set_register`/`get_register`. A handful of general purposes z-registers exist called `D` and `G` (global) (there is also `A`, but it was replaced by `G`). All other z-registers connect some state of the game engine to ZASM - for example, `Link->X = 123` would become the ZASM instruction `SETV LINKX 123000`, so `set_register` would get called to set the player's position to the given value.

Function calls are implemented with `PUSHR/PUSHV <thispc>; GOTO <destpc>` (pushing current pc onto stack and go to destination function). If there are parameters, they get pushed too. There's additional details for the `SP` z-register that I'm brushing over. When the function wants to return, a `RETURN` statement will pop pc to jump to from the stack.

The ZASM bytecode interpreter will execute instructions until it encounters either 1) some `WaitX` command or 2) a `QUIT` command. If it somehow gets past the end of the instructions, that is treated as a `QUIT` (although, I think this should never happen). Some examples of `WaitX` are: `WaitFrame` (yield and resume on the next frame) and `WaitEvent` (wait for some game event to occur before continuing). In any case, whenever a script resumes it picks up from that last `WaitX` call with all its state and registers intact. The `refInfo` and `stack` will continue to exist for this script instance (until cleared, like for FFCs when leaving the screen).

There are 1000+ command types, and 5000+ z-registers.

Now, on to the JIT compilation:

Using `asmjit`, each ZASM instruction in a script is translated to an assembly representation, which is then compiled into machine code and returns a function that can be called later. That function will take as parameters the refInfo, the stack, and a few other bookkeeping variables.

Only a fraction of the ZASM command types are compiled into more efficient machine code. The rest call out to the normal interpreter loop to execute. This is both because there are far too many command types, and that for many of them the overhead of the interpreter is a fraction of the time spent processing the command. Examples of commands that benefit greatly from being optimized are ones that just do math operations, set some z-register, or modifies the stack.

Of the commands that _are_ optimized, if it involves reading / writing to a z-register it again will simply call out to the normal `get_register` / `set_register` functions. The big exception here is that the general-purpose z-registers (`D` and `G`) will simply address the memory directly. An example:

`ADDR D2 D3` - Set `D2` to `D2 + D3`.

The interpeter would normally do all this work:

```c
// interpreter loop:
// (29 ops) decode instruction
// (12 ops) check if this is a WaitX command
// (6 ops) check if script is hung
// (4 ops) check if debugger is on to maybe pause
// (3 ops) check debugger again before printing something
// (11 ops) jump to correct place in giant command switch
// (2  ops) call do_add
// (20 ops) do some misc. checks, including incrementing the pc
// (4 ops) jump back to top of loop

// do_add:
// (2 ops) stack stuff
// (8 ops) call get_register twice
// (4 ops) do the actual adding
// (2 ops) call set_register

// (57 ops) get_register
// (48 ops) set_register
```

In total, about 269 ops to add two numbers in general purpose z-registers. But with JIT compilation, the ZASM instruction is optimized into just 4 ops:

```asm
mov ecx, dword ptr [rbp+8]
mov eax, dword ptr [rbp+12]
add ecx, eax
mov dword ptr [rbp+8], ecx
```

Not only is this far fewer ops, but there are no jumps. For example, the intrepreter has to jump in a big switch block to know to call `do_add`, which likely makes the CPU's instruction cache less effective.

This can result in ~20x better performance for scripts that are just pure math. In practice, quests with large amounts of scripting should see a 5-10x improvement.

`SWITCHKEY` is another z-register that is greatly optimized.

`GOTO` commands will emit a `jmp` to a label that is placed at the correct place in the generated assembly.

Function calls are tricky. They work by pushing the current pc onto the stack, and recalling that pc on `RETURN`. The JIT compilation turns the function call into a `jmp`, and stores on a stack a _function call return address_ (defined by generating a label just after the function call). On `RETURN` code is emitted to pop from this stack and `jmp` to the location. Given these are arbitary jumps and not `call` ops, the assembler is also told where a `RETURN` `jmp` can possibly return to, which helps asmjit with register allocation. In the future, these may be compiled into actual function calls on-demand.

When a `WaitX` command is hit, the compiled function will save the location of that command and return. The next time it is called, it will jump straight to the next instruction after that `WaitX` command. This is tracked via `ri->wait_index`.

## Debugging

These are useful config options:

- `[ZSCRIPT] print_zasm = 1`: for each script, write a file to `zscript-debug/zasm` printing the ZASM instructions
- `[ZSCRIPT] jit_print_asm = 1`: for each script, write a file to `zscript-debug/zasm` printing the x64 assembly
- `[ZSCRIPT] jit_exit_on_failure = 1`: exit program if JIT fails to compile any script

Given a replay that fails only when using JIT, this is how you can debug what's wrong with the compiled code.

1. Determine the first frame that fails in the replay
1. At the top of `script_debug_pre_command`, add an early exit based on that frame number. For example, if the first failing frame is 471704, add: `if (replay_get_frame() < 471704 - 100) return;`. This will prevent emitting any debugging information until 100 frames _before_ the first failure (you may need to change this value). This should be enough for context, and prevents the entire thing taking forever to run
1. Clear any current `zscript-debug` directories: `rm -rf build/Release/zscript-debug*`
1. Collect the _baseline_ debugging output by running the replay with JIT turned off. You can stop the replay a few frames after the first failing frame. To enable the debugging output, set `runtime_debug` in `script_debug.cpp` to `2`.
1. Move the results to a new directory. For example: `mv build/Release/zscript-debug build/Release/zscript-debug-baseline`
1. Run the replay again, but with the JIT enabled.
1. Find the first file that has different output: `diff --brief --recursive build/Release/zscript-debug* | sort -V`
1. Whatever the file, diff them however you like. For example: `vimdiff -c 'set diffopt=context:5' build/Release/zscript-debug*/**/debug-47169.txt`. You could also use https://www.diffchecker.com/text-compare/
1. Hopefully, you'll now see the exact instruction that results in a problem. If a fix seems non-trivial, it may be best to figure out how to create similar ZASM using a much smaller script, so you don't have to wait however long the replay is when iterating on a fix.

The above _usually_ works to pinpoint the instruction that first messes up.

Because it takes so long to debug print the state of scripts after each instruction, it isn't feasible to just debug print for every frame, hence the early exit in step 2. But it's possible that even looking back at 100, 1000, 10000+ frames won't show you the first time a register or stack value differs from what is expected. The state of a script's memory can mess up far before it ever causes a visual regression in the replay. To get a better value, you can repeat the above steps but instead  set `runtime_debug` in `script_debug.cpp` to `2`. Use this more exact value to repeat the above process.

It can also be useful to compile only the script you're debugging in `jit_create_script_handle`: `if (script->debug_id != 3598) return nullptr;`

## Learning materials

### x64 assembly

- https://web.stanford.edu/class/cs107/guide/x86-64.html
- https://www.cs.uaf.edu/2017/fall/cs301/lecture/09_11_registers.html
- https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention
- https://www.cavestory.org/guides/csasm/guide/redefine_imul_idiv.html

### asmjit

- https://eli.thegreenplace.net/2017/adventures-in-jit-compilation-part-2-an-x64-jit/
- https://gitter.im/asmjit/asmjit?at=5fd5247b91e8cb3e8cf92b62
