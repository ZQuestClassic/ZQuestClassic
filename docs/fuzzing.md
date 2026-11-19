# Fuzzing

Quests, saves, replays and music files all come from other people, so zplayer must survive any
bytes in them. These scripts run [libFuzzer](https://llvm.org/docs/LibFuzzer.html) with
AddressSanitizer against the code that reads them, and against the ZASM interpreter (quests
carry ZASM that is not validated when loaded).

Only bugs that a quest, save or replay could exploit are worth fixing on this branch: writes out
of bounds, uses after free, and reads out of bounds. A crash that only stops the program (a null
dereference, a CHECK, deep recursion, running out of memory) is not.

The targets live in `src/zc/fuzz.cpp`, compiled into zplayer when CMake is configured with
`-DWANT_FUZZ=ON`. They run in process, so each input costs one load, not a process launch.

| Target  | Input                                                                                    |
|---------|------------------------------------------------------------------------------------------|
| `qst`   | A quest file (`loadquest`)                                                               |
| `sav`   | A save file                                                                              |
| `zplay` | A replay file                                                                            |
| `music` | First byte picks the decoder: 0 Ogg, 1 MP3 (both Allegro's acodec), 2 GME (.spc .nsf .gbs .vgm .gym), 3 MIDI |
| `zasm`  | Decoded into a ZASM script, run in a game of `playground.qst` by the interpreter       |

## Running

Needs LLVM's clang (on macOS: `brew install llvm`; Apple's clang has no libFuzzer).

```sh
bash scripts/fuzzing/build_fuzzer.sh      # build_fuzz/zplayer
python3 scripts/fuzzing/create_corpus.py  # seed inputs from tests/, in .tmp/fuzz/seeds
bash scripts/fuzzing/run_fuzzer.sh qst 4  # 4 parallel jobs; extra args go to libFuzzer
```

`run_fuzzer.sh` keeps going after a crash. Crashes are saved in `.tmp/fuzz/artifacts/<target>/`,
and the corpus it grows in `.tmp/fuzz/corpus/<target>/` (so a second run starts from where the
first left off). Add `-max_total_time=<seconds>` to stop on its own.

How fast a target runs depends mostly on how big its inputs are: `qst` takes 100 ms for a 7 MB
quest, and 7 ms for a 5 KB one. Two things help. A corpus that has grown large is worth shrinking
now and then to the smallest inputs that reach the same code:

```sh
bash scripts/fuzzing/minimize_corpus.sh qst
```

And a second run with a low limit on input size gets through many more inputs. It can't keep what
only the long inputs in the corpus reach, so run it next to one at the full limit (they share the
corpus):

```sh
MAX_LEN=262144 bash scripts/fuzzing/run_fuzzer.sh qst 4
```

Then group the crashes by where they happen:

```sh
python3 scripts/fuzzing/triage.py qst
```

Each group is marked `high`, `medium` or `low` by how exploitable its ASan report looks (see
`severity()` in `triage.py`), most severe first. `--min_severity medium` leaves out the rest.

To reproduce one, run zplayer on it from the build folder (the target comes from an env var, as
libFuzzer reads the whole command line as its own):

```sh
cd build_fuzz
ZC_FUZZ=qst ./zplayer ../.tmp/fuzz/artifacts/qst/crash-...
```

For `zasm`, also set `ZC_FUZZ_ZASM_QST` to `tests/replays/playground/playground.qst` (the run and
triage scripts do), and `ZC_FUZZ_PRINT=1` to see the script an input decodes to.

The corpus is stored with compression taken off (mutating compressed bytes rarely gets anywhere).
To add your own quests or saves, unpack them the same way:

```sh
ZC_FUZZ=unpack ./zplayer quest.qst ../.tmp/fuzz/corpus/qst/quest.qst
```

## Notes

- The build uses the `Asan` config at `-O1`, but with asserts off (`NDEBUG`, and no `DEBUGMODE` for
  Allegro's `ASSERT`) as in a release build: an assert can stop a run just before the bug a release
  build would go on to hit. CHECKs stay on.
- Leak detection is off (`detect_leaks=0`): the loaders' globals make it noisy, and it is not what
  these look for.
- Some crashes only happen after earlier inputs in the same process (left over global state) and
  won't reproduce from the artifact alone. Their stacks are in the `-asan` folder next to the
  run's log in `.tmp/fuzz/logs/<target>/`: libFuzzer's own log keeps only the first line of each
  crash (timeouts included). The reports there don't name the input.
- The `zasm` target's jumps only go forward, and it leaves out `GOTOR`, `RETURN` and `LOOP` and
  never writes the `PC` register, so every script ends. Commands that touch files, end the game,
  or run whole frames are left out too.

## Future work

Run the `zasm` target's scripts through the JIT as well (on x64 builds only, and only for players
who turn it on).
