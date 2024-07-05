# ZQuest Classic

ZQuest Classic is an engine for making games similar to the original NES Zelda.

Website: [zquestclassic.com](https://zquestclassic.com)

[Latest Downloads](https://zquestclassic.com/releases/)

[Our Discord](https://discord.gg/ddk2dk4guq)

We support Windows (Win7+), OSX (12+), and Linux. There is also an experimental [web version](https://zquestclassic.com/play/) that can also be played on mobile devices.

## Development

See [`docs/building.md`](./docs/building.md).

ZQuest Classic builds Allegro 5 from source. The code base still uses Allegro 4 APIs, but uses [Allegro-Legacy](https://github.com/NewCreature/Allegro-Legacy) to translate into Allegro 5.

The compiled binaries require a number of resource files to be available at runtime. By default, building the project will copy those files to the correct location. They will only be copied if they do not already exist–so you can modify these files (like the various `.cfg` files) in your build directory to your liking. To completely refresh the resources file, build the `copy_resources` target (ex: `cmake --build build -t copy_resources`)

Logs are written to `allegro.log`.

We have dozens of replay tests (`tests/replays/*.zplay`), which are text files that have recorded inputs of actual gameplay. To avoid regressions, we play this input back to the game engine, and in CI make sure that every single frame is drawn as expected. This command will run a single replay:

```sh
python3 tests/run_replay_tests.py --filter tests/replays/classic_1st.zplay
```

See [`docs/replays.md`](./docs/replays.md) for more.

## Contributing to ZQuest Classic

We encourage third-party submission of patches and new features! If you're interesting in contributing to ZQuest Classic's development, please read the CONTRIBUTE file.

## License

GPLv3. See the [LICENSE](./LICENSE).
