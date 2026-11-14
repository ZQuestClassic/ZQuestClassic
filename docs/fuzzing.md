# Fuzzing

Quests, saves, replays and music files all come from other people, so zplayer must survive any
bytes in them. These scripts run [libFuzzer](https://llvm.org/docs/LibFuzzer.html) with
AddressSanitizer against the code that reads them, and against the ZASM interpreter and JITs
(quests carry ZASM that is not validated beyond its jump targets).

The targets live in `src/zc/fuzz.cpp`, compiled into zplayer when CMake is configured with
`-DWANT_FUZZ=ON`. They run in process, so each input costs one load, not a process launch.

| Target  | Input                                                                                    |
|---------|------------------------------------------------------------------------------------------|
| `qst`   | A quest file (`loadquest`)                                                               |
| `sav`   | A save file                                                                              |
| `zplay` | A replay file                                                                            |
| `music` | First byte picks the decoder: 0 Ogg, 1 MP3 (both Allegro's acodec), 2 GME (.spc .nsf .gbs .vgm .gym), 3 MIDI |
| `zasm`  | Decoded into a ZASM script, run in a game of `playground.qst` by the interpreter, then by the optimizer and JIT |

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

- The build uses the `Asan` config, so asserts and DCHECKs are on. Some of those fire on bad input
  where a release build would handle it; check what the code does without the assert before
  calling it a bug.
- The `zasm` target makes ZASM the compiler never emits, and the engine aborts on some of it on
  purpose. Write those checks as `ZASM_CHECK`, `ZASM_DCHECK` or `ZASM_NOTREACHED` (`zc/scripting/zasm_check.h`):
  they are `CHECK`, `assert` and `NOTREACHED` in every other build, but in a fuzzing build they end
  the script run and the input is dropped, rather than reported as a crash.
- Leak detection is off (`detect_leaks=0`): the loaders' globals make it noisy, and it is not what
  these look for.
- Some crashes only happen after earlier inputs in the same process (left over global state) and
  won't reproduce from the artifact alone. The fuzzer's log in `.tmp/fuzz/logs` has their stacks.
- The `zasm` target's jumps only go forward, it limits calls and never writes the `PC` register,
  so every script ends. Commands that touch files or the network, end the game, or run whole
  frames are left out.

## Future work

Fuzz more things: the ZScript compiler, and the ZASM target with quest debug data (the structured
function scopes it carries) and literals for `GOTOTABLE`/`GOTORANGES`.
