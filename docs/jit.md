# JIT

`zplayer` typically executes a quest's ZASM scripts by running them in our ZASM interpreter. To speed things up significantly, we can compile ZASM scripts to native machine code using the JIT (just-in-time) compiler. This runs in `zplayer`, not in the `zscript` compiler. There are three backends, all under `src/zc/scripting/jit/`: `jit_x64.cpp` (x86-64), `jit_a64.cpp` (AArch64, e.g. Apple Silicon), and `jit_wasm.cpp` (the browser build). JIT is not supported for 32-bit.

| | x64 | a64 | wasm |
|---|---|---|---|
| File | `jit_x64.cpp` | `jit_a64.cpp` | `jit_wasm.cpp` |
| Platform | x86-64 | AArch64 (e.g. Apple Silicon) | browser |
| Compiles | one function at a time (large ones in pieces) | same as x64 | a whole script chunk |
| When | once a function is hot, or all on quest load with `jit_precompile` | same as x64 | on quest load only |
| On by default | yes | yes | no (still experimental) |

## Using the JIT

### Enabling

JIT compilation is on by default on desktop; the shipped web config still disables it (`jit = 1 #? web = 0` in `base_config/zc.cfg`), so browser builds only use it when opted in. It can be toggled with the `[ZSCRIPT] jit` config option, found in the launcher. There is also the `-jit` / `-no-jit` command line switch, or `--(no-)jit` for `run_replay_tests.py`.

### Configuration

```ini
# in zc.cfg ...

[ZSCRIPT]
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

## How the interpreter runs ZASM

`run_script` sets up the current script object (which contains the ZASM instructions), and the proper context for that script (refInfo and the stack). Normally, it then calls `run_script_int` which enters a loop that processes one ZASM instruction at a time, based on the current instruction counter `ri->pc`. Based on the instruction command, it will have 0-3 args; and may modify the state of either the stack, some "z-register", or some game object.

A "z-register" (`ffscript.cpp` calls them registers; the compiler wraps them in `VarArgument`, in `src/parser/Opcode.h`) is set/read via `set_register`/`get_register`. A handful of general purpose z-registers exist called `D` and `G` (global). All other z-registers connect some state of the game engine to ZASM - for example, `Hero->X = 123` would become the ZASM instruction `SETV LINKX 123`, so `set_register` would get called to set the player's position to the given value.

Function calls in modern quests use `CALLFUNC <destpc>` / `RETURNFUNC` (older 2.55-era compiles instead push the return pc and `GOTO <destpc>` - this is what `is_modern_function_calling()` distinguishes). `CALLFUNC` pushes the return pc onto a separate return stack (`ret_stack`, indexed by `ri->retsp`); parameters are pushed onto the data stack (`stack`, indexed by `ri->sp`), and the function addresses them and its locals relative to the `SFRAME` register (`D4`). When the function wants to return, `RETURNFUNC` pops the return pc to jump back to.

The ZASM bytecode interpreter will execute instructions until it encounters either 1) some `WaitX` command or 2) a `QUIT` command. If it somehow gets past the end of the instructions, that is treated as a `QUIT` (this should never happen). Some examples of `WaitX` are: `WaitFrame` (yield and resume on the next frame) and `WaitEvent` (wait for some game event to occur before continuing). In any case, whenever a script resumes it picks up from that last `WaitX` call with all its state and registers intact. The `refInfo` and `stack` will continue to exist for this script instance (until cleared, like for FFCs when leaving the screen).

There are 1000+ command types, and 5000+ z-registers.

## How the JIT works

### Translating ZASM to machine code

Each ZASM instruction in a script is translated to an assembly representation (using `asmjit` on native, or written directly as wasm bytes in the browser), which is then compiled into machine code and returns a function that can be called later. That function will take as parameters the refInfo, the stack, and a few other bookkeeping variables.

Only a fraction of the ZASM command types are compiled into more efficient machine code. The rest call out to the normal interpreter loop to execute. This is both because there are far too many command types, and that for many of them the overhead of the interpreter is a fraction of the time spent processing the command. Examples of commands that benefit greatly from being optimized are ones that just do math operations, set some z-register, or modify the stack.

Of the commands that _are_ compiled, if it involves reading / writing to a z-register it again will simply call out to the normal `get_register` / `set_register` functions. The big exception here is that the general-purpose z-registers (`D` and `G`) will simply address the memory directly. An example:

### Example: `ADDR D2 D3`

`ADDR D2 D3` - Set `D2` to `D2 + D3`.

The interpreter would normally do all this work:

```c
// interpreter loop:
// (3 ops) check loop condition (scommand != 0xFFFF)
// (3 ops) check if debugger or runtime debugging is on
// (13 ops) decode instruction (command + 3 args)
// (4 ops) clear string/vector literal args
// (1 op) record current command for error messages
// (2 ops) check if per-instruction runtime debugging is on
// (5 ops) count instructions to check if script is hung
// (7 ops) jump to correct place in giant command switch
// (2 ops) call do_add
// (2 ops) jump to post-command checks
// (14 ops) check for rare conditions (early return, overflow, invalid ZASM, ...)
// (12 ops) increment the pc and check it for overflow
// (6 ops) count commands run, check if running on behalf of the JIT

// do_add:
// (8 ops) stack stuff
// (11 ops) get_register twice (inlined: D registers take a fast path)
// (1 op) do the actual adding
// (5 ops) set_register (inlined fast path)
```

In total, about 100 ops to add two numbers in general purpose z-registers. But with JIT compilation, the ZASM instruction is optimized into just 4 ops:

```x86asm
mov ecx, dword ptr [rbp+8]
mov eax, dword ptr [rbp+12]
add ecx, eax
mov dword ptr [rbp+8], ecx
```

Not only is this far fewer ops, but there are no branches. The interpreter takes 22 branches to run this one instruction (on both x64 and arm64). Most are checks that almost never fire, which the CPU predicts correctly. The expensive one is the switch's indirect jump to the `ADDR` case: its target depends on which ZASM command comes next, so the CPU has to guess it, and a wrong guess throws away the work it started on (around 15-20 cycles on modern CPUs).

Using a JIT compiler to compile ZASM like this can result in ~20x better performance for scripts that are just pure math. In practice, quests with large amounts of scripting should see a 5-10x improvement.

### D-register cache (native backends)

The native backends also cache `D0`..`D7` in host registers between flush points (see `jit_codegen_shared.h`), so runs of instructions like the above usually don't touch memory. Dirty values are written back at waits, branches, function calls and returns, and calls into the engine. A liveness analysis skips writes that no later instruction can observe; it is summarized per function so that a value kept in a register across a `CALLFUNC`, or left for the caller after a `RETURNFUNC`, counts as live (see `zasm_analyze_function_liveness`).

### Other compiled commands

`SWITCHKEY` (the value a `switch` compares against) is also kept in a register by the JIT instead of going through `get_register` / `set_register`.

`GOTO` commands will emit a `jmp` to a label that is placed at the correct place in the generated assembly.

### Compilation unit and timing

The backends differ in what unit they compile and when:

- **Native (`jit_x64.cpp` / `jit_a64.cpp`):** scripts are compiled **per-function**. Once a function is run enough (lots of calls to it, or it loops a lot internally), it will be compiled. Compilation happens in a worker pool – the main thread does not wait for it to be compiled. Once the worker pool compiles a function, the main thread can then run it when executing that function. Until then, the interpreter handles executing that function. If precompiling is enabled, all functions are compiled on quest load, before the game starts. Very large functions are compiled in pieces (see [Large functions are compiled in pieces](#large-functions-are-compiled-in-pieces)).
- **wasm (`jit_wasm.cpp`, browser build):** the **entire script chunk** (e.g. `@single`, which contains all of a quest's generic/ffc scripts and their functions) is compiled to a single wasm module at once. Only precompiling is supported - there is no hot-threshold path (so the `jit_hot_function_*` settings below don't apply to the browser build). See the [WASM backend](#wasm-backend) section for how it works.

### Yielding and resuming

When a `WaitX` command is hit, the compiled function saves where to resume and returns; the next time it is called it picks up at the instruction after that `WaitX`.

On the wasm backend the resume point is a saved block index (`wait_index`) that the function's loop-switch dispatches on when re-entered.

## Native backends (x64, a64)

### Code layout

The two native backends share everything except instruction selection: `jit_shared.{h,cpp}` is the runtime driver (compilation pipeline, script/instance lifecycle, hot-function profiling, the run loop) and `jit_codegen_shared.h` is the codegen policy they must apply identically (the D-register cache and its liveness-based flush rules, the per-command emit loop, the compiled-command list).

### Resume dispatch

The native backends track resume points via `JittedScript::pc_to_resume_address`. x64 jumps straight to the saved native address through an annotated indirect branch. a64 picks per function: with <= 32 resume points it compares `ctx->pc` against each one on entry (direct branches, so the register allocator can put fixup moves on each edge); past that it uses the same indirect branch as x64. The limit is low because the chain runs on every entry, including every return from a callee. Decompiled 2.50-era global scripts with thousands of call sites ran up to 30x slower than the interpreter without it. `-jit-a64-annotated-resume` forces the indirect form everywhere.

### Direct calls between compiled functions

The driver loop in `jit_shared.cpp` normally orchestrates every script function call: a `CALLFUNC` exits the caller's compiled code back to C++, which looks up the callee (a binary search over function start pcs), pushes the ZASM return pc, and re-enters the callee's compiled code through its prologue; every `RETURNFUNC` does the mirror image, plus a resume-address lookup to re-enter the caller mid-function. That round-trip exists because a script can `WaitX` mid-call-stack: native stack frames can't survive returning to C++, so call state has to live in the ZASM return stack instead.

But a function that can never yield (no `WaitX` transitively reachable - the same `may_yield` analysis the wasm backend uses to split its module) never needs to suspend, so calls to it are emitted as plain native calls on the machine stack. The call site loads the callee's entry from the per-script `direct_entry_table`, passes the return pc in `ctx->call_pc`, sets `ctx->entry_mode = 1`, and calls. Until the callee is compiled and committed its slot holds `jit_direct_not_compiled`, a stub that rewrites `ctx` into the driver-path call the site would otherwise have made, so the slot doubles as the hot-swap point without the call site needing a branch of its own. The callee's entry pushes the ZASM return stack via `jit_direct_enter` (so the driver's view of the call chain stays exact) and skips its resume dispatch; its `RETURNFUNC` pops the return stack and returns `EXEC_RESULT_RETURN` to the call site instead of to the driver.

#### Unwinding

Everything unusual is handled by one rule: a direct call site treats any result other than `EXEC_RESULT_RETURN` as an unwind, and returns it unchanged from its own function. A `QUIT`, an error from an interpreter batch, or a driver-path fallback deep in a chain of direct calls just ripples up the native frames to the driver, which handles it exactly as if it had made the calls itself - the ZASM return stack and `ctx->pc`/`ctx->sp` were maintained identically along the way. (`ctx->pc` mirrors the driver's call/return updates deliberately: error stack traces are built from it.)

#### Guard rails

- **Recursion depth:** past 256 ZASM call frames, `jit_direct_enter` converts the call into a driver-path call, since native frames are real stack memory (`stack_limit.zs`'s 10000-deep recursion exercises this).
- **x64 `saved_regs`:** the call site preserves the single-slot `ctx->saved_regs` pinned-register save area around the nested call, which nested prologues would otherwise clobber.

  It copies the area into one stack slot shared by all of the function's call sites rather than into virtual registers: registers live across a call get spilled, each into its own never-reused slot, so with thousands of call sites the frame grew by hundreds of KB (~316 KB for the Panoply global script described below; ~7.7 KB with the shared slot).

  This matters because asmjit emits no stack probes: Windows grows a thread's stack one guard page (4 KB) at a time, so the first write into a frame larger than a page can land past the guard page and crash. Keeping call sites out of the frame size makes that far less likely, but does not rule it out.

  A big enough function can still have a frame over a page from its other spills, and is only safe if earlier, deeper calls have already committed that much stack.

#### Cost

This removes the dominant cost for call-dense scripts - std.zh-heavy quests make thousands of tiny helper calls per frame, sometimes averaging only a dozen ZASM instructions of real work per call. Across the `measure_performance.py` benchmark set, total script time dropped ~30%, with the most call-heavy replays down 60%+. The tradeoff is compile time and memory: asmjit's register allocator tracks liveness per basic block for every register live across blocks, so its cost grows with blocks x those registers, and every call site adds blocks. Call sites are kept to one extra block each (no branch on an uncompiled callee, and the non-RETURN unwinds share one exit) and keep no register of their own live across a block. Disable with `-no-jit-direct-calls`.

The worst case seen is the 149k-instruction decompiled 2.50-era global script of "The Adventure of Link and Zelda: Panoply of Calatia" (replay upload `69162522CA4C5DAD4DA0CAEDA505FBFB.qst`), with ~12.7k direct call sites. It is the quest's hot global script:

| Panoply global script, compiled whole (a64, median of 3) | without direct calls | with |
|---|---|---|
| Compile that function | ~0.6 s | ~1.1 s |
| Quest precompile | ~0.66 s | ~1.14 s |
| Peak memory (43.7k-frame replay) | ~1.41 GB | ~1.63 GB |
| Total script time | ~5.5 s | ~1.6 s |

Measured with `-jit-precompile -jit-threads 0 -jit-log` (compile times), `measure_memory.py --runs 3` (memory) and `measure_performance.py --mode jit --runs 3` (script time). Such functions are now compiled in pieces (below), which bounds that cost.

### Large functions are compiled in pieces

The memory asmjit needs to compile a function grows about linearly with its size up to ~20k ZASM instructions (at most ~5.4 KB per instruction on a64), and much faster past that, because register allocation tracks liveness per basic block for every register live across blocks. Decompiled 2.50-era global scripts can have 100k-150k instructions in one function; the Panoply script above needed 1.4 GB to compile whole.

So functions over `-jit-max-piece-size` (non-NOP) instructions, default 10000, are split into pieces of about equal size (`jit_split_function`). Each piece is compiled as if it were its own function (`JitPiece`), so a function's pieces compile in parallel. Control moves between pieces through the driver:

- **Leaving a piece** works like a GOTO out of the function: flush the D-register cache, store `ctx->pc`, and return `EXEC_RESULT_CONTINUE`.
- **Entering a piece mid-way** works like resuming after a wait: jump targets reached from other pieces are registered as resume points.

A piece starts at the start of a block where it can. A block longer than a piece (straight-line code, like a script initializing a huge array) is cut once the piece is full, so no piece is much bigger than `-jit-max-piece-size`. A piece never starts where a comparison's flags are still to be read (by the next non-NOP instruction, or by the target of a jump - only hand-written ZASM jumps to a flag reader, and then the jump and its target stay in one piece) or while `SWITCHKEY` is live, since neither survives leaving compiled code. Only a function's first piece can be entered by a direct call.

For testing, `-jit-max-piece-size 50` splits nearly every function at nearly every allowed point.

### AArch64 notes

A few AArch64-specific notes (`jit_a64.cpp`):

- If emitting a function reports an asmjit error, the function is discarded and stays interpreted (always correct, just slower for that one function). A piece too large to encode (InvalidDisplacement - see below) is treated as an expected capacity limit; any other emit error is a codegen bug and aborts under `jit_fatal_compile_errors` (which CI runs with). `-jit-a64-max-vregs N` (off by default) additionally skips compiling functions with more than N virtual registers; it is a bisection/debugging knob.
- Conditional branches (`b.cond`) only reach ±1 MB, so on a64 `-jit-max-piece-size` is capped at 15000: the densest code over every script of ~1000 quests is ~57 bytes per instruction, which keeps any branch in a piece within reach. Register-allocator fixup trampolines are placed inline by commits in our asmjit fork, which also extends spill addressing past the ~16 KB encodable range via a reserved scratch register (spill frames reach ~190 KB).

## WASM backend

The browser build can't emit native machine code, so `jit_wasm.cpp` compiles ZASM to a WebAssembly module instead, which the browser then compiles to machine code itself. Unlike the native backends, there is no assembler library: the backend writes the wasm binary format directly (`wasm_compiler.h`).

### Compilation unit and pipeline

The unit of compilation is a whole ZASM chunk, compiled into one wasm module. Modern quests link all their scripts into a single `@single` chunk, so they get one big module; quests compiled before that linking existed have one chunk per script, so they produce many modules (e.g. stellar_seas builds 300+). Only precompiling is supported (no hot-function path), and it happens at quest load in two stages (`jit_precompile_scripts_impl`):

1. **Codegen** (ZASM -> wasm bytes) is pure C++ and runs in parallel on background threads.
2. **Browser compile** must be issued from the main thread. All generated modules are handed to the browser in a single batch (`em_compile_wasm_batch` -> `ZC.compileScriptWasmModuleBatch` in `web/zasm.js`), which compiles them concurrently on the browser's internal thread pool. Batching matters: issuing modules one at a time costs a full asyncify unwind/rewind of the C++ stack per module.

`jit_threads` applies to the codegen stage (`0` forces a fully-serial path). The engine worker pool itself stays disabled on web due to technical constraints.

### Module structure

Within a chunk, functions are split by whether they can yield (transitively reach a `WaitX`; see `zasm_find_yielding_functions`):

- **Non-yielding functions** compile to individual wasm functions with real structured control flow (`WasmStructurer` in `wasm_structurer.h`, a dominator-tree-guided translation). Calls to them are plain wasm calls; the native call stack handles returns. (The native backends apply the same idea to non-yielding callees - see "Direct calls between compiled functions" above.) Irreducible control flow falls back to the loop-switch lowering below.
- **All yielding functions are inlined into one "yielder" function** built as a loop-switch: a `loop` + `br_table` dispatch over every basic block, because its call/return/resume targets are runtime values. Dispatch values are `(cfg block id + 1)`. A `CALLFUNC` to a yielding function pushes the return continuation's dispatch value onto a per-instance return stack (`call_stack_rets`) and jumps back to the dispatch; `RETURNFUNC` pops it. Non-suspending loops inside the yielder are still compiled as real wasm loops nested in the dispatch (see `detect_yielder_regions`).

The module exports one `run(...)` function that receives pointers (refInfo, stack, instance state, ...) and stores them in wasm globals; hot values (the script `sp`, the refInfo/stack/global-D pointers) are kept in wasm locals inside compiled code.

Not every ZASM command is compiled (see `command_is_compiled`); runs of uncompiled commands call back into the interpreter in batches via `do_commands`. All engine callbacks (`get_register`/`set_register`/`do_commands`/...) are bound as **direct wasm->wasm imports** - `web/zasm.js` resolves the engine's raw exports (`rawEngineExport`) so script->engine calls skip JS entirely.

### Yielding and resuming

A `WaitX` stores the next block's dispatch value in the instance's `wait_index` and returns cleanly from `run()`. On the next call, the run wrapper dispatches straight to that block; `call_stack_rets`/`call_stack_ret_index` restore the inlined call stack. `wait_index == 0` means "not started yet" and dispatches to the script's entry.

`RUNGENFRZSCR` (generic frozen scripts) cannot run its nested engine frames inside a wasm frame - the engine advances frames via asyncify, which can't unwind through the (non-instrumented) script module. Instead it yields cleanly back to `run_script` with `frozen_dest_reg` set; the C++ driver runs the frozen engine frame, writes the result, and resumes the script (see the driver loop in `run_script`).

The same asyncify constraint means a script *started* while another jitted script is executing (e.g. an FFC running an enemy's script mid-frame) runs its first slices on the interpreter (`jit_can_start_script`). When the JIT later takes it over mid-run, `jit_create_script_impl` translates the interpreter's resume state: at a yield, `ri->pc` and every live `ret_stack` entry are dispatchable yielder blocks, whose dispatch values were recorded at compile time in `JittedScript::resume_dispatch`. Anything unmapped declines (an error under `jit_fatal_compile_errors`) and the script finishes its run interpreted. The native backends need none of this - they dispatch on `ri->pc` and share the interpreter's `ret_stack`, which is also what makes their hot-function upgrades trivially safe.

## Debugging

### Tools

`zplayer -extract-zasm game.qst` will write all scripts as text ZASM to `zscript-debug/zasm`. This is useful for reference.

These are useful config options:

- `[ZSCRIPT] jit_print_asm = 1`: for each script, write a file to `zscript-debug/zasm` printing the emitted assembly (x64 or a64)
- `[ZSCRIPT] jit_fatal_compile_errors = 1`: exit program if JIT fails to compile any script

And some CLI options:

- `-script-runtime-debug 1` to print a register/stack summary to `zscript-debug/` before executing every script
- `-script-runtime-debug 2` same as above, but prints for every instruction (lots of data)

### Debugging a JIT-only replay failure

Given a replay that fails only when using JIT, this is how you can debug what's wrong with the compiled code:

1. Have a failing replay, for example: `tests/replays/playground/maths.zplay`
1. Run `python scripts/jit_runtime_debug.py --replay_path tests/replays/playground/maths.zplay`
1. The script will run that replay w/ JIT, then w/o JIT, then show you where the registers/stack/pc first differ
1. Hopefully, you'll now see the exact instruction that results in a problem. If a fix seems non-trivial, it may be best to figure out how to create similar ZASM using a much smaller script, so you don't have to wait however long the replay is when iterating on a fix.

It can also be useful to compile only the script you're debugging in `jit_create_script_impl`: `if (script->id != {ScriptType::Player, 1}) return nullptr;`

### WASM backend

- A **native build with `-DJIT_BACKEND=wasm`** generates (but can't run) the same modules - fast, execution-free codegen validation. With `-jit-save-wasm` it writes `wasm/<qst>/<module>.wasm` next to the exe; check structure with `wasm2wat <f> --enable-threads`. This is also how to verify a codegen refactor is byte-identical.
- Each codegen strategy has an escape-hatch flag for A/B and bisection: `-jit-wasm-structured`, `-jit-wasm-structured-yielder`, `-jit-wasm-sp-local`, `-jit-wasm-pointer-locals`, `-jit-wasm-inline-regs`, `-jit-wasm-bail-on-frozen-generic` (all default on except the bail).
- `-jit-run-lo N` / `-jit-run-hi N` force `run_script` calls outside `[lo, hi)` to the interpreter, for bisecting which invocation must be jitted to reproduce a divergence. The gate is only active when `-jit-run-hi` is set (its default of -1 disables it), so `-jit-run-lo` alone has no effect. `scripts/jit_runtime_debug.py` automates this against a native no-jit baseline and works on web replays (pass `--baseline_build_folder build/Release` to speed up baseline collection).

## Flags reference

Boolean flags can be negated with a `-no` prefix (e.g. `-no-jit-wasm-structured`). Where a config option is listed, the flag overrides it.

| Flag | Backend | Default | What it does |
|---|---|---|---|
| `-jit` | all | on (off on web) | Enable the JIT. Config: `[ZSCRIPT] jit` |
| `-jit-precompile` | all | off | Compile every script on quest load. Config: `jit_precompile` |
| `-jit-threads N` | all | -2 | Compile threads. Config: `jit_threads` |
| `-jit-hot-function-call-count N` | native | 10 | Config: `jit_hot_function_call_count` |
| `-jit-hot-function-loop-count N` | native | 1000 | Config: `jit_hot_function_loop_count` |
| `-jit-log` | all | off (on in CI) | Log compilation to `allegro.log`. Config: `jit_log` |
| `-jit-print-asm` | native | off | Config: `jit_print_asm` |
| `-jit-fatal-compile-errors` | all | off | Config: `jit_fatal_compile_errors` |
| `-jit-direct-calls` | native | on | [Direct calls between compiled functions](#direct-calls-between-compiled-functions) |
| `-jit-max-piece-size N` | native | 10000 | [Split larger functions into pieces](#large-functions-are-compiled-in-pieces) of at most this many instructions (at most 15000 on a64) |
| `-jit-a64-annotated-resume` | a64 | off | Always use the indirect-branch [resume dispatch](#resume-dispatch) |
| `-jit-a64-max-vregs N` | a64 | off | Skip compiling functions with more than N virtual registers |
| `-jit-save-wasm` | wasm | off | Write modules to `wasm/<qst>/` |
| `-jit-wasm-structured`, `-jit-wasm-structured-yielder`, `-jit-wasm-sp-local`, `-jit-wasm-pointer-locals`, `-jit-wasm-inline-regs` | wasm | on | Codegen escape hatches |
| `-jit-wasm-bail-on-frozen-generic` | wasm | off | Codegen escape hatch |
| `-jit-run-lo N` / `-jit-run-hi N` | all | off | Bisect which `run_script` call must be jitted |
| `-script-runtime-debug 1\|2` | all | off | Register/stack dumps per script (1) or per instruction (2) |

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
