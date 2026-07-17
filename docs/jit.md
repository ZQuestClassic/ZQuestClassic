# JIT

`zplayer` executes a quest's ZASM scripts typically by running them in our ZASM interpreter. To speed things up significantly, we can compile ZASM scripts to native machine code using the JIT (just-in-time) compiler. This runs in `zplayer`, not in the `zscript` compiler. There are three backends, all under `src/zc/scripting/jit/`: `jit_x64.cpp` (x86-64), `jit_a64.cpp` (AArch64, e.g. Apple Silicon), and `jit_wasm.cpp` (the browser build). JIT is not supported for 32-bit.

The two native backends share everything except instruction selection: `jit_shared.{h,cpp}` is the runtime driver (compilation pipeline, script/instance lifecycle, hot-function profiling, the run loop) and `jit_codegen_shared.h` is the codegen policy they must apply identically (the D-register cache and its liveness-based flush rules, the per-command emit loop, the compiled-command list).

JIT compilation is on by default on desktop; the shipped web config still disables it (`jit = 1 #? web = 0` in `base_config/zc.cfg`), so browser builds only use it when opted in. It can be toggled with the `[ZSCRIPT] jit` config option, found in the launcher. There is also the `-jit` / `-no-jit` command line switch, or `--(no-)jit` for `run_replay_tests.py`.

`[ZSCRIPT] jit_threads = -2` controls how many threads to use for compilation. See the note in `base_config/zc.cfg` for more.

## How it works

First, details on how ZASM is normally interpreted:

`run_script` sets up the current script object (which contains the ZASM instructions), and the proper context for that script (refInfo and the stack). Normally, it then calls `run_script_int` which enters a loop that processes one ZASM instruction at a time, based on the current instruction counter `ri->pc`. Based on the instruction command, it will have 0-2 args; and may modify the state of either the stack, some "z-register", or some game object.

A "z-register" (in the code, `ffscript.cpp` calls them registers, but the AST parser code calls them Vars) is set/read via `set_register`/`get_register`. A handful of general purposes z-registers exist called `D` and `G` (global). All other z-registers connect some state of the game engine to ZASM - for example, `Link->X = 123` would become the ZASM instruction `SETV LINKX 123`, so `set_register` would get called to set the player's position to the given value.

Function calls in modern quests use `CALLFUNC <destpc>` / `RETURNFUNC` (older 2.55-era compiles instead push the return pc and `GOTO <destpc>` - this is what `is_modern_function_calling()` distinguishes). If there are parameters, they get pushed onto the stack too. There's additional details for the `SP` z-register that I'm brushing over. When the function wants to return, `RETURNFUNC` pops the pc to jump back to.

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

The native backends also cache `D0`..`D7` in host registers between flush points (the D-register cache in `jit_codegen_shared.h`), so runs of instructions like the above usually don't even touch memory - dirty values are written back at waits, branches, and calls into the engine, with a liveness analysis skipping writes no later instruction can observe.

This can result in ~20x better performance for scripts that are just pure math. In practice, quests with large amounts of scripting should see a 5-10x improvement.

`SWITCHKEY` is another z-register that is greatly optimized.

`GOTO` commands will emit a `jmp` to a label that is placed at the correct place in the generated assembly.

The backends differ in what unit they compile and when:

- **Native (`jit_x64.cpp` / `jit_a64.cpp`):** scripts are compiled **per-function**. Once a function is run enough (lots of calls to it, or it loops a lot internally), it will be compiled. Compilation happens in a worker pool – the main thread does not wait for it to be compiled. Once the worker pool compiles a function, the main thread can then run it when executing that function. Until then, the interpreter handles executing that function. If precompiling is enabled, all functions are compiled on quest load, before the game starts.
- **wasm (`jit_wasm.cpp`, browser build):** the **entire script chunk** (e.g. `@single`, which contains all of a quest's generic/ffc scripts and their functions) is compiled to a single wasm module at once. Only precompiling is supported - there is no hot-threshold path (so the `jit_hot_function_*` settings below don't apply to the browser build). See the [WASM backend](#wasm-backend) section for how it works.

When a `WaitX` command is hit, the compiled function saves where to resume and returns; the next time it is called it picks up at the instruction after that `WaitX`. The native backends track resume points via `JittedScript::pc_to_resume_address` (x64 jumps straight to the saved native address through an annotated indirect branch; a64 re-enters the function, which dispatches by comparing `ctx->pc` against its resume points - the indirect-branch form works on AArch64 too, but measures ~10% slower on script-heavy quests because an indirect edge can't carry per-edge register fixups, so it's kept only behind `-jit-a64-annotated-resume`). On the wasm backend it is a saved block index (`wait_index`) that the function's loop-switch dispatches on when re-entered.

A few AArch64-specific notes (`jit_a64.cpp`):

- If emitting a function reports an asmjit error, the function is discarded and stays interpreted (always correct, just slower for that one function). A spill frame too large for AArch64's load/store offsets (InvalidDisplacement) is treated as an expected capacity limit; any other emit error is a codegen bug and aborts under `jit_fatal_compile_errors` (which CI runs with). `-jit-a64-max-vregs` (default 10000) additionally skips compiling functions so register-heavy they'd likely be discarded anyway.
- Conditional branches (`b.cond`) only reach ±1 MB, which the largest functions exceed; branches that may reach far are emitted via `emit_cond_branch`, and register-allocator fixup trampolines are placed inline by a patch to our asmjit fork (`third_party/asmjit.patch`, which also extends spill addressing past the ~16 KB encodable range via a reserved scratch register).

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

- `[ZSCRIPT] jit_print_asm = 1`: for each script, write a file to `zscript-debug/zasm` printing the emitted assembly (x64 or a64)
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

The browser build can't emit native machine code, so `jit_wasm.cpp` compiles ZASM to a WebAssembly module instead, which the browser then compiles to machine code itself. Unlike the x64 backend there is no asmjit-style assembler: the backend writes the wasm binary format directly (`wasm_compiler.h`).

### Compilation unit and pipeline

The unit of compilation is a whole ZASM chunk, compiled into one wasm module. Modern quests link all their scripts into a single `@single` chunk, so they get one big module; quests compiled before that linking existed have one chunk per script, so they produce many modules (e.g. stellar_seas builds 300+). Only precompiling is supported (no hot-function path), and it happens at quest load in two stages (`jit_precompile_scripts_impl`):

1. **Codegen** (ZASM -> wasm bytes) is pure C++ and runs in parallel on background threads.
2. **Browser compile** must be issued from the main thread. All generated modules are handed to the browser in a single batch (`em_compile_wasm_batch` -> `ZC.compileScriptWasmModuleBatch` in `web/zasm.js`), which compiles them concurrently on the browser's internal thread pool. Batching matters: issuing modules one at a time costs a full asyncify unwind/rewind of the C++ stack per module, which used to dominate precompile time.

`jit_threads` applies to the codegen stage (`0` forces a fully-serial path). The engine worker pool itself stays disabled on web due to technical constraints.

### Module structure

Within a chunk, functions are split by whether they can yield (transitively reach a `WaitX`; see `zasm_find_yielding_functions`):

- **Non-yielding functions** compile to individual wasm functions with real structured control flow (`WasmStructurer` in `wasm_structurer.h`, a dominator-tree-guided translation). Calls to them are plain wasm calls; the native call stack handles returns. Irreducible control flow falls back to the loop-switch lowering below.
- **All yielding functions are inlined into one "yielder" function** built as a loop-switch: a `loop` + `br_table` dispatch over every basic block, because its call/return/resume targets are runtime values. Dispatch values are `(cfg block id + 1)`. A `CALLFUNC` to a yielding function pushes the return continuation's dispatch value onto a per-instance return stack (`call_stack_rets`) and jumps back to the dispatch; `RETURNFUNC` pops it. Non-suspending loops inside the yielder are still compiled as real wasm loops nested in the dispatch (see `detect_yielder_regions`).

The module exports one `run(...)` function that receives pointers (refInfo, stack, instance state, ...) and stores them in wasm globals; hot values (the script `sp`, the refInfo/stack/global-D pointers) are kept in wasm locals inside compiled code.

Not every ZASM command is compiled (see `command_is_compiled`); runs of uncompiled commands call back into the interpreter in batches via `do_commands`. All engine callbacks (`get_register`/`set_register`/`do_commands`/...) are bound as **direct wasm->wasm imports** - `web/zasm.js` resolves the engine's raw exports (`rawEngineExport`) so script->engine calls skip JS entirely; this roughly halved script time when introduced.

### Yielding and resuming

A `WaitX` stores the next block's dispatch value in the instance's `wait_index` and returns cleanly from `run()`. On the next call, the run wrapper dispatches straight to that block; `call_stack_rets`/`call_stack_ret_index` restore the inlined call stack. `wait_index == 0` means "not started yet" and dispatches to the script's entry.

`RUNGENFRZSCR` (generic frozen scripts) cannot run its nested engine frames inside a wasm frame - the engine advances frames via asyncify, which can't unwind through the (non-instrumented) script module. Instead it yields cleanly back to `run_script` with `frozen_dest_reg` set; the C++ driver runs the frozen engine frame, writes the result, and resumes the script (see the driver loop in `run_script`).

The same asyncify constraint means a script *started* while another jitted script is executing (e.g. an FFC running an enemy's script mid-frame) runs its first slices on the interpreter (`jit_can_start_script`). When the JIT later takes it over mid-run, `jit_create_script_impl` translates the interpreter's resume state: at a yield, `ri->pc` and every live `ret_stack` entry are dispatchable yielder blocks, whose dispatch values were recorded at compile time in `JittedScript::resume_dispatch`. Anything unmapped declines (an error under `jit_fatal_compile_errors`) and the script finishes its run interpreted. The x64 backend needs none of this - it dispatches on `ri->pc` and shares the interpreter's `ret_stack`, which is also what makes its hot-function upgrades trivially safe.

### Debugging

- A **native build with `-DJIT_BACKEND=wasm`** generates (but can't run) the same modules - fast, execution-free codegen validation. With `-jit-save-wasm` it writes `wasm/<qst>/<module>.wasm` next to the exe; check structure with `wasm2wat <f> --enable-threads`. This is also how to verify a codegen refactor is byte-identical.
- Each codegen strategy has an escape-hatch flag for A/B and bisection: `-jit-wasm-structured`, `-jit-wasm-structured-yielder`, `-jit-wasm-sp-local`, `-jit-wasm-pointer-locals`, `-jit-wasm-inline-regs`, `-jit-wasm-bail-on-frozen-generic` (all default on except the bail).
- `-jit-run-lo N` / `-jit-run-hi N` force `run_script` calls outside `[lo, hi)` to the interpreter, for bisecting which invocation must be jitted to reproduce a divergence. The gate is only active when `-jit-run-hi` is set (its default of -1 disables it), so `-jit-run-lo` alone has no effect. `scripts/jit_runtime_debug.py` automates this against a native no-jit baseline and works on web replays (pass `--baseline_build_folder build/Release` to speed up baseline collection).

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
