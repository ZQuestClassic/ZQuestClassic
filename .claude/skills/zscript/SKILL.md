---
name: zscript
description: Working with ZScript and ZASM in ZQuest Classic — building zplayer/zeditor/zscript, compiling scripts, the JIT and ZASM optimizer, dumping ZASM/emitted x64, running & updating replay and snapshot tests, recording replays, debugging JIT miscompilations, and benchmarking. Use for anything touching src/parser, src/components/zasm, src/zc/scripting/jit/jit_*, src/zc/zasm_*, ffscript, or tests/replays.
---

# ZScript / ZASM / JIT

ZScript source → (parser/) ZASM bytecode → run by the interpreter (`components/zasm/eval.cpp`)
or compiled by the JIT (`zc/scripting/jit/`: `jit_x64.cpp`, `jit_a64.cpp`, `jit_wasm.cpp`;
`jit_shared.cpp` is the shared native driver). Before either runs, the ZASM optimizer
(`zc/zasm_optimize.cpp`) may rewrite the bytecode.

Pipeline: `src/parser` (lex/grammar/AST/codegen) → linker → `zc/zasm_optimize.cpp`
(optional) → JIT backend or `components/zasm/eval.cpp` (interpreter). On an
Apple Silicon mac the native backend is `jit_a64.cpp`, not x64.

## Building

```bash
cmake -B build -S .                       # once (build dir is build/, outputs to build/Release)
cmake --build build --config Release -t zplayer zeditor zscript base_test_runner copy_resources
```

- Binaries land in `build/Release/`. Incremental builds only recompile what changed; a
  single `.cpp` edit + relink is ~30s, but `ffscript.cpp`/`zelda.cpp` are huge and slow.
- `scripts/run_target.py` picks the build folder from `BUILD_FOLDER` env, else the most
  recently built of `build/RelWithDebInfo`, `build/Release`, `build/Debug`. Most test
  scripts honor `BUILD_FOLDER=build/Release`.
- If ninja says only `copy_resources` ran but you edited a file, the .o looked up-to-date —
  `touch` the source and rebuild.

## Compiling a script standalone & seeing its ZASM

```bash
inc="$PWD/tests/scripts;$PWD/resources/include;$PWD/resources/headers;$PWD/tests/scripts/playground"
build/Release/zscript -input <file.zs> -zasm /tmp/out.zasm -commented -include "$inc" -unlinked -json
```

- `-commented` annotates each ZASM op with source line; `-json` prints diagnostics as JSON
  (look for `"success": true`).
- **`-unlinked` shows the PRE-LINK form.** The linker rewrites some ops, so you will NOT
  see `LOADD`/`SP` here (you see `LOAD`/`SP2`). To see the real ops the JIT compiles, dump
  the linked/JIT form instead (below).
- This is the compiler's ZASM, *before* the zc-side `zasm_optimize` pass.

## Dumping the linked + JIT-compiled ZASM and emitted x64

The most reliable way to see exactly what the JIT compiles (with `; NN OPCODE args`
comments interleaved with the emitted x86-64):

```bash
build/Release/zplayer -headless -replay <file.zplay> -frame 0 \
  -jit -jit-precompile -jit-threads 0 -jit-print-asm \
  -script-runtime-debug-folder /tmp/jitdump -replay-exit-when-done -no_console
# → /tmp/jitdump/zasm/<qst>/zasm-<script>.txt
```

- Add `-no-optimize-zasm` to see the *unoptimized* form the JIT compiles.
- `-frame 0` makes it compile (precompile) and exit fast without running the whole replay.
- `-extract-zasm <qst>` also dumps ZASM, but has a flush bug where only the **last** script
  is written — prefer the `-jit-print-asm` route above.

## JIT / optimizer flags (zplayer)

| Flag | Meaning |
|---|---|
| `-jit` / `-no-jit` | enable/disable JIT (default on; config `ZSCRIPT.jit`) |
| `-jit-precompile` | compile ALL scripts synchronously at load (blocks in `zasm_pipeline_init`, so compile time is before the first frame) |
| `-jit-threads 0` | compile synchronously, no worker pool |
| `-jit-print-asm` | dump emitted asm to the script-runtime-debug-folder |
| `-jit-log` | JIT logging to stdout |
| `-jit-cache-registers` / `-no-...` | D-register caching (default on) |
| `-optimize-zasm` / `-no-optimize-zasm` | ZASM optimizer (default on) |
| `-optimize-zasm-experimental` | extra passes (spurious-branches, etc.) |
| `-jit-fatal-compile-errors` | abort on a compile error instead of falling back |
| `-jit-hot-function-loop-count N` / `-jit-hot-function-call-count N` | hot thresholds (when not precompiling) |
| `-test-jit-bisect N` | (native x64/a64) skip compiling the first N functions — needs `ENABLE_BISECT_TOOL` in `jit_shared.cpp`, see "Localizing a JIT miscompilation (native)". For the wasm/web JIT use a *per-instance* bisect instead, see "Debugging the wasm (web) JIT" |

## Running replay tests

```bash
# Quick set (fast):
python tests/run_replay_tests.py --filter playground --filter z3 --filter misc --build_folder build/Release

# A specific replay (pass an ABSOLUTE path; relative paths fail to load):
python tests/run_replay_tests.py --build_folder build/Release "$PWD/tests/replays/playground/auto_bug_mod_int_min.zplay"
```

Useful flags: `--jit`/`--no-jit` (default jit on), `--optimize-zasm`/`--no-optimize-zasm`
(default on), `--update` (re-record/update in place), `--test_results <dir>`,
`--retries N`, `--filter <substr>` (repeatable), `--no-headless`.

- With JIT, the harness uses `-jit-precompile` for all replays **except** `yuurand*` and
  `freedom*` (those test hot-compilation).
- Other replays take a while; stick to the quick set unless you need a specific one.

## Auto script tests (the easy way to add a scripted test)

Each `tests/scripts/playground/auto/<name>.zs` is a `generic script <name>` that
`#include "auto/test_runner.zs"`, calls `Test::Init()` first and `Test::End()` last.
Helpers: `Test::Init()`, `Test::End()`, `Test::AssertEqual(actual, expected)`,
`Test::Assert(cond)`, `Test::Fail(msg)`. Assertion failures print and `Game->Exit()`.

```bash
python tests/update_auto_script_tests.py          # regenerate auto.zs, recompile playground.qst, record/update replays
python tests/update_auto_script_tests.py --test   # just run them, no update
```

- This recompiles `playground.qst`, so **all** auto replays churn (and `playground.qst`,
  `auto.zs`). That's expected when adding/changing a script — commit them together.
- After adding a script you also need its compiler snapshot: run
  `python tests/test_zscript.py --update TestZScript.test_zscript_compiler_expected_zasm`
  (creates `<name>_expected.txt`), and `python tests/test_optimize_zasm.py --update`
  (optimizer stats in `tests/snapshots/optimize_zasm/playground.txt` shift when a script
  is added).
- See `tests/scripts/README.md`.

## Snapshot tests

```bash
python tests/test_zscript.py --update          # compiler output per script (*_expected.txt) + more
python tests/test_optimize_zasm.py --update    # optimizer stats (tests/snapshots/optimize_zasm/)
python tests/test_jit.py --update              # JIT asm snapshots (tests/snapshots/jit/) — currently SKIPPED
python tests/update_snapshots.py               # runs the snapshot updaters
```

## Recording a replay manually

```bash
build/Release/zplayer -headless -test <qst> <dmap> <scr> \
  -record /tmp/out.zplay -replay-name <name> -frame <N> -replay-exit-when-done -no_console
```

- `-frame N` stops recording at frame N. For an auto/generic test, `-replay-name <name>`
  selects which `generic script` runs (on the test dmap).
- A recording with no per-frame input (e.g. a script that needs no keys) produces a
  **step-less** replay that, on playback, **stops at frame 0** (replay ends when recorded
  steps are exhausted). To make such a replay run N frames, append a terminal step
  `Q <N> 3` and set `M frames <N>`. `-frame N` caps normal (step-ful) replays but BREAKS
  step-less ones (they run 0 frames).

## ZASM facts & JIT gotchas

- ZASM numbers are **fixed-point ×10000** (so `5` is `50000`). `div_10000` converts a
  fixed-point stack offset to an index.
- The `int` type is a lie - it's the fixed-point type described above.
- The `float` type is a lie - there is no float type, it's just an int.
- **`LOAD` vs `LOADD`:** `LOAD` uses a raw integer offset; `LOADD`/`STORED` use a
  fixed-point offset (`/10000`). `LOADD`/`STORED` appear mostly in **older-compiled
  quests** (2.55-era, e.g. `keys.qst`); modern compiles emit `LOAD`. `LOADI`/`STOREI`
  (stack access by a register offset) are still common (e.g. `eiyuu.qst`).
- **D-register cache:** the JIT caches `D0..D7` in virtual registers; `get_z_register`
  returns the cached reg. **Mutating that reg in place corrupts the cache** unless you
  write it back via `set_z_register` or copy it first. This is a recurring bug class
  (fixed by `compute_stack_offset` for LOAD/STORE-family ops and `immutable_cast_bool`
  for comparisons). When adding codegen, never run an in-place mutator (`div_10000`,
  `cast_bool`, arithmetic) directly on a `get_z_register` result you don't store back.
- **The `-no-optimize-zasm` CI pass exists to catch JIT bugs the optimizer hides** — the
  optimizer often rewrites away the exact bytecode pattern that triggers a JIT bug, so the
  default suite (optimizer on) misses them. `ci.py replay-tests --no-optimize-zasm` /
  `test.yml` run the playground subset with the optimizer off.
- The `const` type modifier only makes that variable unassignable - it does not prevent
  modifying the variable's fields.

## Localizing a JIT miscompilation (native)

Works for both native backends (x64 and a64). When a replay fails only under JIT
(esp. only with `-no-optimize-zasm`):

1. Enable `#define ENABLE_BISECT_TOOL` in `src/zc/scripting/jit/jit_shared.cpp` (the
   comment above it documents an alternate find-first-fail workflow), rebuild zplayer.
2. Binary-search `N`:
   ```bash
   python tests/run_replay_tests.py --filter <replay> --no-optimize-zasm \
     --extra_args="-jit-threads 0 -test-jit-bisect N"
   ```
   `-test-jit-bisect N` runs the first N functions interpreted and JIT-compiles the rest.
   The flip point (fail at N-1, pass at N) means **function index N-1** is miscompiled.
3. Map index → function name with a temporary trace in `compile_and_queue_function`, then
   dump that function's asm with `-jit-print-asm` and read it.
4. Revert the `#define` and any trace when done.

## Debugging the wasm (web) JIT

The wasm JIT (`src/zc/scripting/jit/jit_wasm.cpp`) only runs in the **web build**, so a bug that
reproduces only there needs a different workflow than the x64 tool above. This is hard,
slow (~8 min/build), and some bug classes defeat bisection — read this before diving in.

### Reproduce & read the result

```bash
# build the web engine (repackages packages/web; -t zplayer does NOT — stale-wasm trap)
source ~/tools/emsdk/emsdk_env.sh && cmake --build build_emscripten --config Release -t web
# run a replay (ABSOLUTE path required — /host/ maps to the real fs)
python tests/run_replay_tests.py --build_folder build_emscripten/Release --frame 4000 "$ABS.zplay"
```

- **`--no-jit` is the correctness oracle.** If the replay passes with `--no-jit` (it almost
  always does — the interpreter is correct on every platform) but fails with the JIT, the
  bug is 100% JIT codegen. Confirm this first.
- **Web exit-code semantics** (from `web/tests/run_replay.js`): **0** = pass; **120**
  (`ASSERT_FAILED_EXIT_CODE`) = replay/graphics/rng divergence (a *value* miscompile — the
  script produced different state); **1** = the tab **renderer crashed** or an uncaught
  `pageerror`/`RuntimeError` fired. Exit 1 with empty stderr and stdout ending mid-frame =
  the renderer process died = severe **memory corruption**, *not* a wasm trap or `abort()`
  (those print a message). `run_replay_tests` reports 1/120 as "failed w/ exit code N".
- `al_trace` reaches captured stdout **during** execution, but the last lines right before
  `zc_exit` are dropped — `fflush(stdout)` after a diagnostic print if you need it to survive.
- COEP `ERR_BLOCKED_BY_RESPONSE` (1–2 line stdout, empty roundtrip) is a load flake under
  build load — just re-run. Clean up the server with `lsof -ti tcp:8000 | xargs kill`
  (never `pkill -f webserver.mjs` — it matches the orchestrating command line).

### Bisect: sound vs layout-contaminated (the key lesson)

- **Per-instance bisect is Heisenbug-STABLE and reliably names the culprit *script*.** Add a
  temporary flag in `run_script` (`src/zc/ffscript.cpp`) that JIT-compiles only the first N
  *distinct* `(type, i)` script instances (first-seen order) and interprets the rest — make
  the decision **sticky per instance** so a yielding script never switches backend mid-run.
  N=0 all-interp passes; binary-search the smallest N that fails; instance N-1 is the culprit.
  This is stable across re-runs because it's a coarse behavioral switch, not a codegen change.
- **Codegen-toggle bisects are LAYOUT-CONTAMINATED for memory-corruption bugs — do not trust
  them.** Forcing a pc-range or a command *type* to interpret (via `command_is_compiled` —
  it has a documented list of "optional" commands you can comment out, or `compile_command_interpreter`)
  changes the wasm module's size/layout, which *moves* a layout-sensitive corruption. This
  produces stable-but-FALSE pinpoints (e.g. it will finger the highest-volume op like
  `PUSHR D`, or a command in a function that never even executes). Use these only to
  narrow *classes* interpreted-in-bulk, and corroborate every pinpoint against the
  interpreter — never conclude "command X is buggy" from a codegen toggle alone.

### Localize a *value* miscompile (exit 120 / rng-desync)

Use `scripts/jit_runtime_debug.py` — it collects a `--no-jit` baseline and the JIT run with
`-script-runtime-debug 2 -script-runtime-debug-frame N` and diffs the per-command
`pc: N OP | D:<regs> stack:<...>` lines to find the first divergent instruction. **Caveats
for web replays** (fix before relying on it): its `RunResult` dataclass rejects the current
`rng_desync` result field; it needs a recorded `failing_frame` (a *hard crash* reports frame
0 → it trims the replay to nothing); and the full-runtime-debug pass stalls on long (100s of
frames) replays. Manual fallback: run a native `--no-jit` baseline and the web JIT both with
`-script-runtime-debug 2 -script-runtime-debug-frame N`, route the trace to stdout, and diff.
Note: this only catches divergences visible in **script registers/stack** — an in-bounds
wild write into engine memory (renderer crash, exit 1) shows nothing here.

### wasm JIT architecture (differs fundamentally from x64 — scopes the bug)

- `@single` (all generic/ffc scripts) is compiled as **one module**. Removing the
  per-chunk `RUNGENFRZSCR`/`STACKWRITEATVV_IF` bail in `jit_compile_script` JITs the whole
  chunk — that's how you enable generic content, but it exposes every codegen bug the bail
  was shielding.
- **Yielding** functions (anything that transitively `WaitX`es) are **inlined into one
  "yielder"** wasm function with a loop-switch (`br_table` over block ids) + a per-instance
  `call_stack_rets` return stack; yield/resume saves/restores `g_idx_sp`,
  `call_stack_ret_index`, `wait_index` in the run-wrapper. **Non-yielding** functions are
  separate wasm functions called with native `emitCall`.
- Contrast: the x64 JIT returns to `run_script_int` for CALLFUNC, and the interpreter uses a
  `ret_stack` array — so **the yielder/call machinery has no x64 equivalent to diff against**,
  and native-`emitCall` recursion depth is a wasm-only concern (though `STACK_SIZE` is large;
  a 5MB→64MB bump is a quick way to rule stack exhaustion in/out).
- Registers: `get_z_register` has distinct paths for `D`/`GD`/`SP`/`SP2`/
  `does_register_use_stack` (only `MAPDATAEXDOOR`/`SCREENDATAEXDOOR`)/other (C++ call). SP/SP2
  read `g_idx_sp` inline; evaluate a pushed operand **before** `add_sp` (matching interp/x64).

### Fast, execution-free codegen validation (no web build)

Configure a native `-DJIT_BACKEND=wasm` build (`build_wasm_backend`): it *generates* the wasm
module natively (runs interpreted, can't execute it). With `-jit-save-wasm` it writes
`<exe-dir>/wasm/<qst>/<module>.wasm`; validate structure with
`wasm2wat <f> --enable-threads`. A Debug build here also aborts at an unexpected opcode
("unexpected command …"), pinpointing a fused/dropped op instantly. Use this to check every
codegen edit compiles to valid wasm before paying for the 8-min web build.

- `SAFE_HEAP=1` does **not** catch this bug class: it only flags out-of-bounds/misaligned
  accesses in the *main* module (not the runtime-compiled JIT module), so a wrong-but-in-bounds
  wild write is invisible; it also breaks the `zscript.mjs` link (wasm-opt SAFE_HEAP+asyncify).

## Benchmarking script/JIT performance

- **Use `-script-timings`** (see `src/zc/script_timings.h`): accumulates wall-clock time
  spent inside `run_script` across the session and prints a summary (total + per-script
  attribution + the N slowest script-frames; `-script-timings-top N`, default 20) when the
  game/replay ends. Reentrancy-aware (nested frozen-generic runs aren't double counted).
  This isolates scripting cost from engine/render time when comparing JIT vs interpreter.
- For error-spammy scripts (maths divides by zero every frame), set
  `suppress_script_error_logging` so you measure compute, not I/O.
- **Good benchmark: `yuurand_riviere.zplay`** — genuinely script-heavy. JIT is ~2.6× over
  the interpreter there; the ZASM optimizer adds ~1.16× to the interpreter but ~nothing on
  top of JIT.
- **Poor benchmark: `maths.zplay`** — engine/function-call-bound (~1260 calls/frame) and
  error-I/O-heavy, so JIT only shows ~1.1× and it doesn't reflect script-engine speed.
- `keys.qst` and `maths.qst` are old (2.55-era) quests; useful for `LOADD`-path coverage.

## Where things live

- `src/parser/` — lexer/grammar (`ffscript.lpp`/`.ypp`), AST, `BuildVisitors.cpp` (codegen).
- `src/components/zasm/` — interpreter (`eval.cpp`), opcode table (`table.cpp`).
- `src/zc/scripting/jit/` — JIT backends (`jit_x64.cpp`, `jit_a64.cpp`, `jit_wasm.cpp`,
  `jit_none.cpp`; `jit_shared.cpp` native driver, `jit.cpp` enable/precompile control);
  `zc/zasm_optimize.cpp` — optimizer; `zc/zasm_pipeline.cpp` — orchestration.
- `src/zc/ffscript.cpp` — interpreter command impls (`do_div`, `do_mod`, etc.) and
  `scripting_log_error_with_context` / `suppress_script_error_logging`.
- `tests/run_replay_tests.py`, `tests/replays.py`, `tests/update_auto_script_tests.py`,
  `tests/test_zscript.py`, `tests/test_optimize_zasm.py`, `tests/test_jit.py`.
