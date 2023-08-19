# Zelda Classic

Zelda Classic is a game and editing tool that lets you create custom Legend of Zelda quests.

Homepage:
https://www.zeldaclassic.com/

The homepage contains information about the latest release, links to the latest binaries, and a repository of custom quests for you to enjoy.

[Latest Downloads](http://zeldaclassic.com/downloads)

[Our Discord](https://discord.gg/ddk2dk4guq)

We support Windows, OSX, and Linux. There is also an experimental [web version](https://hoten.cc/zc/play/) that can also be played on mobile devices.

## Development

See [`docs/building.md`](./docs/building.md).

Zelda Classic builds Allegro 5 from source. The code base still uses Allegro 4 APIs, but uses [Allegro-Legacy](https://github.com/NewCreature/Allegro-Legacy) to translate into Allegro 5.

The compiled binaries require a number of resource files to be available at runtime. By default, building the project will copy those files to the correct location. They will only be copied if they do not already exist–so you can modify these files (like the various `.cfg` files) in your build directory to your liking. To completely refresh the resources file, build the `copy_resources` target (ex: `cmake --build build -t copy_resources`)

Logs are written to `allegro.log`.

We have dozens of replay tests (`tests/replays/*.zplay`), which are text files that have recorded inputs of actual gameplay. To avoid regressions, we play this input back to the game engine, and in CI make sure that every single frame is drawn as expected. This command will run a single replay:

```sh
python3 tests/run_replay_tests.py --filter tests/replays/classic_1st.zplay
```

See [`docs/replays.md`](./docs/replays.md) for more.

## Contributing to Zelda Classic

We encourage third-party submission of patches and new features! If you're interesting in contributing to Zelda Classic's development, please read the CONTRIBUTE file.

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
