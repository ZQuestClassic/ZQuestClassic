# Contribute to ZQuest Classic

## Overview

First, learn [how to build](./docs/building.md) the project.

ZQuest Classic uses [Allegro 5](https://github.com/liballeg/allegro5), but it began life with Allegro 4. The code base still uses Allegro 4 APIs, but uses [Allegro-Legacy](https://github.com/NewCreature/Allegro-Legacy) to utilize Allegro 5 as the backend. This allows us to run on modern systems and utilize newer Allegro 5 APIs without abandoning decades of code.

The compiled binaries require a number of resource files to be available at runtime. By default, building the project will copy those files to the correct location. They will only be copied if they do not already exist–so you can modify these files (like the various `.cfg` files) in your build directory to your liking. To completely refresh the resources file, build the `copy_resources` target (ex: `cmake --build build -t copy_resources`).

Logs are written to `allegro.log`.

## Testing

Our tests use Python (3.11.4+), so first you should create a virtual env, then install our dependencies:

```sh
python -m venv venv
source venv/bin/activate
python -m pip install -r requirements.txt
```

You can run `source venv/bin/deactivate` to exit the virtual environment (or just close your terminal session).

### Replays

We have hundreds of replay test files (`tests/replays/**/*.zplay`), which are text files that have recorded inputs of actual gameplay. This accounts for 100+ hours of real user gameplay. To avoid regressions, we play this input back to the game engine, and in CI make sure that every single frame is drawn as expected. See [`./docs/replays.md`](./docs/replays.md) for further detail about the replay system.

```sh
# runs a single replay test:
python tests/run_replay_tests.py --filter tests/replays/classic_1st/classic_1st.zplay
# equivalent to:
python tests/run_replay_tests.py --filter classic_1st.zplay
# or to run all classic_1st replays:
python tests/run_replay_tests.py --filter classic_1st
```

* by default this runs against `build/Release`, but you can configure which folder to use with `--build_folder`
* by default this runs ZC in headless mode and as fast as possible; this can be configured with `--no-headless` and `--throttle_fps` (or `--show` to do both)
* limit how many frames of the replay runs like: `--frame 100`
* use `--help` to see more options

We have a local webserver for running these replays from a browser, and for visualizing how a replay fails with a compare report. See [./tests/replay_runner](./tests/replay_runner/README.md) for more.

### User uploaded replays

Users can opt into uploading replays of the quests they play. Only replays for known quests in the database are uploaded. To download them for local use, run `python scripts/replay_uploads.py` (requires an auth key, ask Connor for one).

### Python test suite

Separate from the replay rests, we have a number of test files written in Python (`./tests/test_*.py`).

* to run all of them: `python -m unittest discover tests`
* to run a single one: `python tests/test_zplayer.py`
* to run a single test inside one: `python tests/test_zeditor.py -k quick_assign`
* by default, the most recently built build folder is used. Set the `BUILD_FOLDER` environment variable to use a specific folder: `BUILD_FOLDER=build/Debug python tests/test_zplayer.py`

Some tests validate against known good output - these are called snapshot tests. The known good output for these tests are kept in `tests/snapshots`. For example, `test_zscript.py` compiles many ZScript scripts and ensures the output of the compiler remains unchanged. If a code change modifies the output of one of these tests, you must update the snapshot results (either specifically with `python test_zscript.py --update`, or everything with `python tests/update_snapshots.py`). Be sure to visually confirm the change in output is expected.

### More on testing

See [./tests/scripts/README.md](./tests/scripts/README.md) for more information about replay and ZScript tests.

## Submitting a PR

If you are unfamiliar with the GitHub PR process, read this [guide](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request).

PRs should be an isolated change.

Make sure the CI (GitHub Actions) continues to pass for all environments.

It's encouraged to engage in a discussion before submitting a PR - drop by [our Discord](https://discord.gg/uStAnHJhPM) or start a discussion in a GitHub issue, and we can guide you towards making an effective contribution.
