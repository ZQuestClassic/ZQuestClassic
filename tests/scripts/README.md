Be sure to read `CONTRIBUTING.md` first.

All `*.zs` scripts in this folder are compiled and the output of the compiler is saved to `*_expected.txt` files next to each script. If the compiler output no longer matches the expected output, the tests in `test_zscript.py` will fail.

To update the expected output files, you can run `python tests/test_zscript.py --update`, or `python tests/update_snapshots.py` (which covers more than just the zscript output). Be sure to have recently compiled everything before running.

# `tests/scripts` folders

- `alucard`: these are scripts from Alucard's quests. We have replays for these quests, and we also have all the scripts. In `test_zscript.py - test_compile_and_quick_assign` these quests are recompiled and the replays are run on the newly saved qst. This prevents regressions in the ZScript compilation process - the ZASM will not be the same, but the behavior will be.
- `compat`: scripts in `alucard` rely on some removed behavior from older versions of `std`. That lives here.
- `errors`: all these scripts fail to compile. The errors emitted by the compiler are checked here.
- `misc`: random scripts.
- `newbie_boss`: similar to `alucard`, these scripts are recompiled and the `newbie_boss.zplay` replay is run against the new qst.
- `playground`: these scripts are used to implement all the replays in `tests/replays/playground`. See the next section for more.

## Playground

The playground quest (`tests/replays/playground/playground.qst`) is a collection of random screens testing many different aspects of ZC. All the scripts used by `playground.qst` live in `tests/scripts/playground`. The quest file simply compiles `tests/scripts/playground/playground.zs`. There are dozens of replay tests for this quest, they live in `tests/replays/playground`. To run them all, execute this command:

> python tests/run_replay_tests.py --filter playground

### Adding a playground replay test

There's two ways to go about this. First, the more involved but more customizable approach, for when you have a replay that requires setting up a non-trivial screen in the editor:

1. (if the new replay uses zscript) add the script in `tests/scripts/playground`, and include it in `playground.zs`
2. open `playground.qst` in the editor and set up the screen
3. record the replay, move it to `tests/replays/playground`
4. in the `.zplay` file, edit the `M qst` metadata to be just `playground.qst` (not an absolute path)
5. change the version metadata to be `M version latest`
6. confirm it works as expected - `python tests/run_replay_tests.py --filter playground/my_new_test.zplay --show`

If you do _not_ need a particular screen setup (or it is trivial to configure via a script), there is an alternative, simpler option: an "auto" script test. Here's an example for how to add a test called `trigger_secrets.zs`:

1. create a new script: `tests/scripts/playground/auto/trigger_secrets.zs`
2. add `#include "#include "auto/test_runner.zs"`
3. add a `generic` script of the same name as the file (in this example, `trigger_secrets`)
4. in the `run` method, first call `Test::Init()`
5. implement all your test logic in this `generic` script. You can go for as many frames as you like, but the script must end
6. run `python tests/update_auto_script_tests.py` - this recompiles `playground.qst` and creates the `.zplay` file for you
7. confirm it works as expected - `python tests/run_replay_tests.py --filter playground/auto_trigger_secrets.zplay`

View the `Test` namespace in `test_runner.zs` for some helper methods for asserting values are as expected. In addition, anything printed to the console will end up in the `.zplay` file, as well as a hash of every game frame. If any of those things change, the replay test will fail.

If you make any modification to the scripts used by `playground.qst`, or otherwise just want to update the `auto` replay tests, you can run `python tests/update_auto_script_tests.py` again to automatically recompile and update the replays. Make sure to validate any updates made to the replays!
