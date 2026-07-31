---
name: replay-batches
description: How to find and fix the class of bugs where global state is leaked when loading multiple quests. For example, a replay test may pass on its own but fail when run after another replay in the **same process** (the "in-process batch"). The "--replay-batch" flag for "tests/run_replay_tests.py" is relevant.
---

# Debugging in-process replay failures

How to find and fix the class of bugs where a replay passes on its own but fails
when run after another replay in the **same process** (the "in-process batch").

## The bug class

ZC normally runs one quest/game per process. The in-process batch harness runs many
replays back-to-back in one process to flush out **global/static state that isn't reset
on game start**. A leftover value from game A bleeds into game B and desyncs it.

Every fix so far has the same shape: a file-static or global that is implicitly
"fresh" at process start but never re-initialized when a new game begins. The fix is
almost always a one-line reset in **`init_game_vars()`** (`src/zc/zelda.cpp`) or
**`HeroClass::init()`** (`src/zc/hero.cpp`), next to the existing resets there.

Examples already fixed (good reference for the pattern):
- `next_uid` sprite counter (`src/sprite.cpp`) → `sprite::reset_uid_counter()`
- `hero_newstep` / `hero_newstep_diag` collision step (`src/zc/hero.cpp`)
- `z3step`, `Hero.hitdir`, `Hero.holditem`, `climb_cover_x/y`, scroll-from state
- `palette_user_tinted` script tint (a **render-only** leak — see below)
- `subscreen_open`, `screen_states` (already cleared via `clear_screen_states()`)

## Tools

Engine flags (build: `cmake --build build --config Release -t zplayer`, exe at
`build/Release/zplayer`):

- `-replay-batch <batch file>`: run every replay in the batch file in one process.
  Each line is `<absolute-path> [frame-limit]`. Mode comes from a bare
  `-assert` / `-replay` / `-update` flag. Always pass `-headless` for hunts.
- `-state-hash-log <file>`: append per-frame FNV-1a hashes of game-state regions
  (hero, rng, globalRAM, objectRAM, sprites, viewport, ffc, screen, scroll) to a file.
  Also dumps `HEROFIELDS` (full `HeroClass::debug_state_string()`) at frame 0.

Runner (`tests/run_replay_tests.py`):

- `--batch --seed N --frame 1000`: shuffle all replays into batches (seeded)
  and batch-run each batch. Writes `failing_batch_NN.txt` for each failing batch and
  prints a re-run command.
- `--replay-batch <file>`: run exactly the replays in one batch file (in order, in one
  process) and report per-replay pass/fail with the failing frame. **This is the reliable
  pass/fail signal.**
- `tests/replays_vary_seed.sh N ...`: run N seeds, stop at first failure.

Scratch repros live in `.tmp/failure_groups/` (gitignored, absolute paths). The
**durable** regression suite is `tests/replay_batches/*.txt` — committed batch files
with paths **relative to the repo root**, run by `tests/test_replays.py::test_batches`
(it rewrites them to absolute paths and asserts every replay in each batch passes).

## Workflow

1. **Find a failing batch.** `python tests/run_replay_tests.py --batch
   --seed 6014 --frame 1000`. Copy the `failing_batch_NN.txt` files somewhere safe.

2. **Find which replay actually fails.** `--replay-batch failing_batch_NN.txt`.
   The runner prints `❌ … failure on frame X`. Note: a replay that fails **solo** is NOT
   an in-process leak — verify solo first and skip it if it fails alone.

3. **Bisect the poisoning predecessor.** Build a 2-line batch file: `<predecessor> 1000`
   then `<target> 1000`. Try each predecessor (and combinations — some need two).
   Reproduce the failure with the runner, not by grepping engine output.

4. **Pin the diverging subsystem with `-state-hash-log`.** Run the target solo and
   poisoned, each with its own log, then diff the target's lines column-by-column to
   find the first frame and the first differing column:
   ```
   ./build/Release/zplayer -assert -headless -replay-batch solo.txt  -state-hash-log /tmp/s.log
   ./build/Release/zplayer -assert -headless -replay-batch pois.txt  -state-hash-log /tmp/p.log
   paste <(grep 'TARGET.zplay frame=' /tmp/s.log) <(grep 'TARGET.zplay frame=' /tmp/p.log) \
     | awk '{split($0,a,"\t");split(a[1],s," ");split(a[2],p," ");
             for(i=2;i<=length(s);i++) if(s[i]!=p[i]) print s[2],"DIFF:",s[i],"vs",p[i]}' | head
   ```

5. **Drill to the exact field.** If the `hero`/`heroint` column diverges, diff the frame-0
   `HEROFIELDS` lines token-by-token (watch out: a naive `grep '[a-z_]+='` drops camelCase
   fields like `diagonalMovement`). If the differing value isn't in `debug_state_string`,
   temporarily add the suspect field to it (or add a temp hash column to `dump_state_hashes`
   in `src/zc/ffscript.cpp`) and rebuild. Iterate until you see the exact leaked field.

6. **Confirm before fixing.** Always prove the field leaks with the instrument first.
   Many plausible suspects (climb_cover, hitdir, z3step) turned out to be real leaks that
   were *not* the cause of a given test — fixing them didn't flip the test. Confirm the
   field differs at the first divergence frame, fix it, and re-verify with the runner.

7. **Fix** by resetting the global/static on game start in `init_game_vars()` or
   `HeroClass::init()` (mind the `replay_version_check(N)` guard pattern used by `z3step` —
   it preserves old-replay behavior while fixing new ones). Add a comment explaining the
   leak. Rebuild and confirm the repro passes via the runner.

8. **Add a regression batch file** — REQUIRED for every in-process leak fix. Create
   `tests/replay_batches/<name>.txt` with the minimal `<predecessor> <frame>` /
   `<target> <frame>` lines that reproduced the failure, using paths **relative to the repo
   root** (e.g. `tests/replays/playground/ghost_armos.zplay 200`) and a leading `#` comment
   describing the leak it guards. `test_batches` picks it up automatically;
   run it to confirm it passes now (and ideally that it fails before your fix). This is what
   prevents the leak from silently coming back.

9. **Commit** each distinct leak as its own `fix(zc): …` commit (engine fixes go at the
   tail of history); commit the regression batch file with it.

## Reading a `.roundtrip` file (what diverged, in replay terms)

When an `-assert` replay fails, the engine writes `<name>.zplay.roundtrip` next to the
replay output (e.g. `.tmp/test_results/<id>/<run>/batch_<g>/<name>.zplay.roundtrip`); the
runner also embeds it in the failure output. It is the most direct view of *what* diverged,
in the replay's own event vocabulary — often more legible than a state-hash diff.

It is a side-by-side annotation (written in `replay_save`, `src/zc/replay.cpp`):
- The **left** column is what **this run actually did** (the recomputed `record_log`).
- Where a step differs from the recording, the line is annotated `… « <step>`. **The right
  side of `«` is what *should* have happened** — the recorded/expected step (`replay_log`).
- Trailing lines with a **blank left** and `« <step>` are recorded steps the run never
  reached (it diverged or ended early). A run that desyncs and quits early looks like a few
  `«`-only lines at the bottom.

The **first `«` line is the first divergence.** Read it as: left = what your (poisoned) run
produced, right = what the correct (solo) run produced.

Step syntax (from `print_step`): `M k v` = metadata header; `C <frame> g <hash>` = per-frame
framebuffer (gfx) hash; `C <frame> <text>` = engine comment (`init_game`, `cont_game`,
`getitem <name>`, `sfx <name>`, screen-state, …); `D <frame> [k] <btn/key>` = press, `U` =
release; `Q <frame> <quit>` = the `Quit` value was set. The diverging step usually names the
subsystem that leaked — a menu action, an item grant, a `Quit`, a warp.

Worked example (terror_27 → hero_of_dreams): left showed `D 1024 k F6` → `Q 1024 1` →
`C 1025 init_game` (the poisoned run's F6 press *quit immediately*); the `«` right side
showed `D 1024 k F6 / U 1025 k F6 / D 1035 Start / …` (the recording's F6 opened a menu the
player navigated and *continued* from). Conclusion: the F6 menu took the wrong path when
poisoned → F6-menu availability state leaked from the previous quest.

## Render-only leaks (game state matches, replay still fails)

If **all** `-state-hash-log` columns match but the replay still fails, the leak is in
**rendered output**, not game state. Debug replays record a per-frame framebuffer hash
(the `g` comments, via `replay_step_gfx` / `hash_bitmap`); the runner's "failure on frame X"
can be a gfx-hash mismatch. The state-hash instrument can't see this by default.

To pin it, temporarily add a `gfx` column to `dump_state_hashes` that calls a helper
wrapping `hash_bitmap(framebuf)` (expose one from `replay.cpp`). The framebuffer hash is
computed on a **24bpp** conversion, so it reflects the **palette** — index-based game
state can match while colors differ.

Worked example (the `palette_user_tinted` fix): terror_of_necromancy_demo6_34, poisoned by
enigma, had identical game state but a constant framebuffer diff from frame 1. The map
viewer (`ViewMap`) made it visually obvious. Chain: gfx differs → palette (RAMpal) differs
→ `nonblack=14` vs `234` (game palette blacked, GUI colors survive) → narrowed within
`init_game()` to the step between `loadfullpal` and `loadlvlpal` → `loadpalset`/`loadlvlpal`
route to `tempgreypal` instead of RAMpal (and `restoreTint` blacks RAMpal) when
`isUserTinted()` is true → `palette_user_tinted` leaked `true` from enigma's script tint.
Fix: `isUserTinted(false)` in `init_game_vars()`.

## Gotchas

- **Don't trust `grep unexpected` on headless `-replay-batch`.** `-unexpected.png` files
  are only written in non-headless / snapshot mode. Use the `-state-hash-log` diff, or the
  runner's `--replay-batch` pass/fail, as the source of truth.
- Some replays need **two** predecessors to reproduce; one alone won't poison.
- `-snapshot` and some report features reset per-replay in batch — use single-replay mode
  when you need them.
- A divergence in `heroint` (the comprehensive hero hash) with no visible/gameplay effect
  can be a harmless leaked field; confirm it actually causes the failure before committing.
