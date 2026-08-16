# ZQuest Classic

ZQuest Classic (ZC) is a game engine for creating 2D zelda-like games.

## Source Code Layout

This describes the layout of code inside `src/`. ZQuest Classic is a game runtime (ZC) and a quest editor (ZQuest).

```
src/
├── core/          Quest data structures & serialization: qst.cpp is top-level .qst file I/O,
│                  qst_*.cpp serialization per domain (combos, maps, items, dmaps, ...)
├── zc/            Game runtime (zplayer): zelda.cpp main game loop, hero.cpp, guys.cpp
│                  (enemy AI), ffscript.cpp (script integration), scripting/jit/ (script JIT;
│                  per-platform backends jit_x64/a64/wasm/none), zasm_* (bytecode optimization
│                  pipeline), saves.cpp, replay.cpp, script_debug.cpp, debugger/ (VM debugger)
├── zq/            Quest editor (zeditor): zq_class.cpp main editor class, render*.cpp,
│                  autocombo/ pattern generators, package.cpp quest packaging
├── parser/        ZScript compiler (zscript): ffscript.lpp/.ypp flex/bison grammar, AST +
│                  visitors, SemanticAnalyzer, BuildVisitors (codegen), LibrarySymbols
│                  (built-in declarations), DocVisitor/MetadataVisitor (docs/metadata)
├── components/    Reusable subsystems: zasm/ (bytecode VM: eval.cpp interpreter, table.cpp
│                  opcode table), sound/ (midi, zcmusic, zcmixer), worker_pool/
├── dialog/        ~167 dialog boxes for editor and game, built with gui/ — see dialog/README.md
├── gui/           Low-level widget system + declarative builder API — see gui/README.md
├── base/          General utilities (util, containers, version, http, ints.h, expected.h)
├── zalleg/        Thin Allegro 5 wrapper (rendering, colors, menus, packfile I/O)
├── launcher/      Launcher app (zlauncher)
└── ...            Other small modules: test_runner, zconsole (logging), zupdater,
                   al5_img (image loading), metadata, standalone
```

Dependencies flow roughly as:

- `zc/` and `zq/` both depend on `core/` for shared quest data structures.
- `dialog/` provides UI for both `zc/` and `zq/`, built on top of `gui/`.
- `parser/` compiles ZScript source → bytecode fed into `components/zasm/` and `zc/`'s VM/JIT.
- `base/` and `zalleg/` are low-level helpers used everywhere.

### Notable top-level `src/` files

Large or important files that live directly in `src/` rather than a subdirectory:

| File | Purpose |
|------|---------|
| `subscr.cpp/.h` (~1 MB) | Subscreen/HUD system |
| `new_subscr.cpp/.h` | Rewrite of the subscreen system |
| `tiles.cpp/.h` | Tile graphics and manipulation |
| `sprite.cpp/.h` | Sprite base class and rendering |
| `items.cpp/.h` | Item pickups and inventory |
| `defdata.cpp/.h` (~350 KB) | Default data for all object types |
| `gamedata.cpp/.h` | Player save/game state |
| `ffc.cpp/.h` | Free-form combos (animated tile objects) |
| `slopes.cpp/.h` | Slope geometry and collision |
| `pal.cpp/.h` | Palette management |
| `advanced_music.cpp/.h` | Tracker-style music system |
| `music_playback.cpp/.h` | Music playback coordination |
| `zcsfx.cpp/.h` | Sound effects |

## Building

```bash
# configure build folder (if not already present)
cmake -B build -S . # build within your worktree

# build
cmake --build build --config Release -t all copy_resources
```

Common cmake targets are: zplayer zeditor zscript base_test_runner

## Building (web version)

```sh
# setup emsdk tools
source ~/tools/emsdk/emsdk_env.sh
# configure build_emscripten folder
bash scripts/configure_emscripten.sh
# build
cmake --build build_emscripten --config Release -t web
```

## Replays

You can run a small, quick set of replays like this:

```bash
python tests/run_replay_tests.py --filter playground --filter z3 --filter misc --build_folder build/Release
```

where the builder folder is a path to where the zplayer executable is.

You can run other replays too, but they take awhile. So it's best to run these shorter ones.

When running `zplayer` or `zeditor` directly (e.g. to run a single replay outside the
test harness), always pass `-headless`, and give replay paths as absolute paths.

## Commit messages

The changelog is built from commit messages, so keep these rules in mind:

* Summarize the issue / feature succintly and without using implementation specifics like variable names
* Wrap the commit body at the conventional 72 columns.
* For any implementation-specific details, first add a line "end changelog" (surrounded by blank lines); then provide the technical details, if relevant. Try not to be too wordy.
* If there are no user-facing changes, you can also add a ! and the entire commit will be hidden from the changelog. Ex: "fix(zc)!: ..."
* Also add a ! when a fix is for a bug that was introduced in a prerelease version (i.e. it never shipped in a final release): users of final releases never saw the bug, so the fix doesn't belong in the changelog. Still include the "Regressed in ..." citation.
* Use the correct scope: `zq` for editor changes, `zc` for player changes, `zscript` for scripting changes (even if in the zplayer runtime scripting engine), and none if it touches multiple components. A commit-msg hook enforces the valid scopes — see `scripts/git_hooks/common.py` for the full list.
* If a commit fixes a bug, the last changelog line of the message should cite when the bug was introduced. Ex: `Regressed in 3.0.0-prerelease.134+2025-09-28 (fae2ad2b).` - where the first part is the first released version that has the bug, and the second part is the short hash of the commit that introduced it. By "last changelog line" that means before any "end changelog" line. If the bug existed with the introduction of the feature (so not really a regression), instead say "Bug introduced when ... was added in 3.0.0-prerelease.134+2025-09-28 (fae2ad2b)."
* If there is a Discord thread associated with the commit, end the commit message with it like this: `Discord: https://discord.com/channels/876899628556091432/1519766866027872347`. There should be an empty line before it.

## Working on releases/2.55

Use a git worktree for releases/2.55 work (like cherry-picks) instead of switching the main checkout, unless explicitly told otherwise:

```bash
# create it if it doesn't exist yet
git worktree add ../ZQuestClassic-2.55 releases/2.55
```

Then run all 2.55 git/build/test commands from that worktree. The `build_255` folder in the main checkout is a build dir configured for the 2.55 branch; a worktree should get its own build folder.

## Misc.

* Never use sub-agents, unless explicitly asked to by the user.

## Discord

Run `scripts/threads.py` to read a Discord thread conversation. The thread id is the last segment of the thread's URL - for example, to read https://discord.com/channels/876899628556091432/1529206920379105291 run:

> python scripts/threads.py show 1529206920379105291

The script needs the `discord.py` package, and a Discord bot token in either the `ZC_DISCORD_API_KEY` env var or a `~/.zc_discord_key` file (just the token).
