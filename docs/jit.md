# JIT

`zplayer` executes a quest's ZASM scripts typically by running them in our ZASM interpreter. To speed things up significantly, we can compile ZASM scripts to native machine code using the JIT (just-in-time) compiler. This runs in `zplayer`, not in the `zscript` compiler. There are two backends for the JIT: `jit_x64.cpp`, and `jit_wasm.cpp` (for the browser build). JIT is not supported for 32-bit.

JIT compilation is off by default. It can be enabled by setting the `[ZSCRIPT] jit = 1` config option, found in the launcher. There is also the `-jit` command line switch, or `--(no-)jit` for `run_replay_tests.py`.

`[ZSCRIPT] jit_threads = -2` controls how many threads to use for compilation. See the note in `base_config/zc.cfg` for more.

## How it works

First, details on how ZASM is normally interpreted:

`run_script` sets up the current script object (which contains the ZASM instructions), and the proper context for that script (refInfo and the stack). Normally, it then calls `run_script_int` which enters a loop that processes one ZASM instruction at a time, based on the current instruction counter `ri->pc`. Based on the instruction command, it will have 0-2 args; and may modify the state of either the stack, some "z-register", or some game object.

A "z-register" (in the code, `ffscript.cpp` calls them registers, but the AST parser code calls them Vars) is set/read via `set_register`/`get_register`. A handful of general purposes z-registers exist called `D` and `G` (global). All other z-registers connect some state of the game engine to ZASM - for example, `Link->X = 123` would become the ZASM instruction `SETV LINKX 123`, so `set_register` would get called to set the player's position to the given value.

Function calls are implemented with `PUSHR/PUSHV <thispc>; GOTO <destpc>` (pushing current pc onto stack and go to destination function). If there are parameters, they get pushed too. There's additional details for the `SP` z-register that I'm brushing over. When the function wants to return, a `RETURNFUNC` statement will pop pc to jump to from the stack.

The ZASM bytecode interpreter will execute instructions until it encounters either 1) some `WaitX` command or 2) a `QUIT` command. If it somehow gets past the end of the instructions, that is treated as a `QUIT` (although, I think this should never happen). Some examples of `WaitX` are: `WaitFrame` (yield and resume on the next frame) and `WaitEvent` (wait for some game event to occur before continuing). In any case, whenever a script resumes it picks up from that last `WaitX` call with all its state and registers intact. The `refInfo` and `stack` will continue to exist for this script instance (until cleared, like for FFCs when leaving the screen).

There are 1000+ command types, and 5000+ z-registers.

Now, on to the JIT compilation:

Using `asmjit`, each ZASM instruction in a script is translated to an assembly representation, which is then compiled into machine code and returns a function that can be called later. That function will take as parameters the refInfo, the stack, and a few other bookkeeping variables.

Only a fraction of the ZASM command types are compiled into more efficient machine code. The rest call out to the normal interpreter loop to execute. This is both because there are far too many command types, and that for many of them the overhead of the interpreter is a fraction of the time spent processing the command. Examples of commands that benefit greatly from being optimized are ones that just do math operations, set some z-register, or modifies the stack.

Of the commands that _are_ compiled, if it involves reading / writing to a z-register it again will simply call out to the normal `get_register` / `set_register` functions. The big exception here is that the general-purpose z-registers (`D` and `G`) will simply address the memory directly. An example:

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

Scripts are compiled per-function. Once a function is run enough (lots of calls to it, or it loops a lot internally), it will be compiled. Compilation happens in a worker pool – the main thread does not wait for it to be compiled. Once the worker pool compiles a function, the main thread can then run it when executing that function. Until then, the interpreter handles executing that function.

If precompiling is enabled, all functions are compiled on quest load, before the game starts.

When a `WaitX` command is hit, the compiled function will save the location of that command and return. The next time it is called, it will jump straight to the next instruction after that `WaitX` command. This is tracked via `JittedScript::pc_to_address`.

## Configuration

```ini
# in zc.cfg ...

# Enable for JIT-compilation of ZASM scripts.
jit = 1
# Number of background threads for compiling scripts.
# -1 to set to number of available CPU threads
# -x to set to (number of available CPU threads) / x
# 0 to only compile on the main thread
jit_threads = -2
# When a function is called this many times, it is deemed "hot" and will be compiled by the JIT.
jit_hot_function_call_count = 10
# When a function loops this many times, it is deemed "hot" and will be compiled by the JIT.
jit_hot_function_loop_count = 1000
# Compile all scripts on quest load, and wait for it to finish before starting the game.
# Note that this totally ignores the above two "hot" thresholds.
jit_precompile = 0
```

## Debugging

`zplayer -extract-zasm game.qst` will write all scripts as text ZASM to `zscript-debug/zasm`. This is useful for reference.

These are useful config options:

- `[ZSCRIPT] jit_print_asm = 1`: for each script, write a file to `zscript-debug/zasm` printing the x64 assembly
- `[ZSCRIPT] jit_fatal_compile_errors = 1`: exit program if JIT fails to compile any script

And some CLI options:

- `-script-runtime-debug 1` to print a register/stack summary to `zscript-debug/` before executing every script
- `-script-runtime-debug 2` same as above, but prints for every instruction (lots of data)

Given a replay that fails only when using JIT, this is how you can debug what's wrong with the compiled code:

1. Have a failing replay, for example: `tests/replays/playground/maths.zplay`
1. Run `python scripts/jit_runtime_debug.py --replay_path tests/replays/playground/maths.zplay`
1. The script will run that replay w/ JIT, then w/o JIT, then show you where the registers/stack/pc first differ
1. Hopefully, you'll now see the exact instruction that results in a problem. If a fix seems non-trivial, it may be best to figure out how to create similar ZASM using a much smaller script, so you don't have to wait however long the replay is when iterating on a fix.

It can also be useful to compile only the script you're debugging in `jit_create_script_impl`: `if (script->id != {ScriptType::Player, 1}) return nullptr;`

## WASM backend

See https://gist.github.com/connorjclark/874f1034809ce475a8e3ea7e09a8cc40 for some notes on the WASM backend.

## Learning materials

### x64 assembly

- https://web.stanford.edu/class/cs107/guide/x86-64.html
- https://www.cs.uaf.edu/2017/fall/cs301/lecture/09_11_registers.html
- https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention
- https://www.cavestory.org/guides/csasm/guide/redefine_imul_idiv.html
- https://github.com/FFmpeg/asm-lessons/blob/main/lesson_01/index.md

### asmjit

- https://eli.thegreenplace.net/2017/adventures-in-jit-compilation-part-2-an-x64-jit/
- https://gitter.im/asmjit/asmjit?at=5fd5247b91e8cb3e8cf92b62
