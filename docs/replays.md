# Replays

Replays are `.zplay` text files that ZC can create and play back to replay a game session. All button inputs and RNG seeds are recorded so that ZC can recreate the same playthrough frame-by-frame. [Here is a demo](https://www.youtube.com/watch?v=47sbYAa9RJk&ab_channel=ConnorClark).

This system was created primarily as a way to create automated tests. For more on that, see `tests/run_replay_tests.py`.

A new save file can be recorded by selecting the `ZC > Replay > Enable recording new saves` menu option. This will make any new save file also be recorded. You'll see an alert box telling you where the file will be saved. You can save and continue whenever without harming the recording process.

At any time, you can playback a replay in the `ZC > Replay` menu to watch it again, and can even take manual control whenever you want. This will not modify any exiting saved games. Note that you currently aren't be able to persist a save file when replaying a `.zplay` from this menu.

Later work on this system may introduce more user-facing features, such as savestates, rewinding, or creating a new game file from a replay.

In the meantime, **a great way to contribute to ZC development** is to enable the recording feature for your new games, and provide us your `.zplay` files. You don't have to finish the game, any amount of playthroughs could be helpful. The more we have, the better coverage our testing system will have, and the fewer regressions/compatability bugs there will be!

## Test coverage

- `pip install gcovr`
- `cmake -S . -B build -G 'Ninja Multi-Config'` (can skip if already configured a ninja multi-config build)
- `cmake --build build --config Coverage -t zelda`
- `python tests/run_replay_tests.py --build_folder build/Coverage --replay --ci`
- `bash tests/generate_coverage_report.sh`
- `open tests/.coverage/report/index.html`

Coverage reports are hosted at https://armageddongames.github.io/ZQuestClassic/coverage and are updated automatically.
