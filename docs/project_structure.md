# Project structure

## Third-party code

Vendored libraries are kept in the `third_party` folder.

All of ZC's C++ code is in `src`.

## Foundational libraries

The foundation of ZC is built on these libraries, each depending on the previous one:

- `base`: Low-level utilities, helpers. No dependency on Allegro.
- `zalleg`: Allegro-related functionality. Depends on base.
- `core`: All ZC-related data structures and logic needed by multiple apps. Depends on zalleg.
- `gui`: GUI components needed by multiple apps. Depends on core.

## Components

Standalone libraries depending on only `base` or `zalleg` are kept in `src/components`:

- `src/components/scc`
- `src/components/sound`
- `src/components/worker_pool`
- `src/components/zasm`

These components may not depend on each other.

`core` or any app may depend on a component.

## Apps

Every target that creates an executable is in the `./src/apps` folder:

> TODO: not true yet.

- `src/apps/launcher`
- `src/apps/parser`
- `src/apps/standalone`
- `src/apps/zc`
- `src/apps/zq`
- `src/apps/zupdater`

Most (but not all) apps depend on Allegro via `zalleg`.

## Dialogs

Dialogs used by more than one app are in `src/dialog`. Currently, any app that needs one compiles it as a separate compilation unit.

A dialog only one app uses lives in that app instead: `src/zq/dialog` for the editor, `src/zc/dialog` for the player.

## Loose files

Currently, many source files are not within any organized location described above, but rather are included into potentially multiple apps as separate compilation units.

This includes all files defined in the `modules/*.txt` files. For example:

- Files directly in the src folder (`./src/*.cpp`)
- Everything in `src/dialog`

Many of these files check which app is compiling them (`IS_PLAYER / IS_EDITOR` defines) to build properly. Eventually, most these files should be moved to `core` so that they are compiled once.
