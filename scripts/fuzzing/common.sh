# Shared by run_fuzzer.sh and minimize_corpus.sh: checks the target and sets up its folders and
# environment. Expects TARGET to be set.

ROOT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/../.." &> /dev/null && pwd )
BUILD=${BUILD_FOLDER:-$ROOT/build_fuzz}
FUZZ=$ROOT/.tmp/fuzz

if [ -z "$TARGET" ]; then
    echo "usage: $(basename "$0") <qst|sav|zplay|music|zasm> ..."
    exit 1
fi

mkdir -p "$FUZZ/corpus/$TARGET" "$FUZZ/artifacts/$TARGET" "$FUZZ/logs/$TARGET"
SEEDS=$FUZZ/seeds/$TARGET
[ -d "$SEEDS" ] || SEEDS=

# Leaks are not what this is looking for, and the loaders' globals make them noisy. handle_abort
# gives failed asserts and CHECKs a full ASan report, rather than libFuzzer's one-line "deadly
# signal".
export ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=0:allocator_may_return_null=1:handle_abort=1}
# Symbolized stacks in the log, for crashes that only happen after earlier inputs (and so may not
# reproduce from the artifact alone).
if [ -z "$ASAN_SYMBOLIZER_PATH" ]; then
    for s in /opt/homebrew/opt/llvm/bin/llvm-symbolizer /usr/local/opt/llvm/bin/llvm-symbolizer "$(command -v llvm-symbolizer)"; do
        if [ -x "$s" ]; then export ASAN_SYMBOLIZER_PATH=$s; break; fi
    done
fi
export ZC_FUZZ=$TARGET
# The zasm target runs its scripts in a game of this quest.
export ZC_FUZZ_ZASM_QST=${ZC_FUZZ_ZASM_QST:-$ROOT/tests/replays/playground/playground.qst}

# -detect_leaks=0: ASAN_OPTIONS doesn't stop libFuzzer from running an input a second time to look
# for a leak, which it does whenever a run allocates more than it frees (most quest loads).
LIBFUZZER_FLAGS=(-detect_leaks=0 -timeout=20 -rss_limit_mb=4096 -malloc_limit_mb=2048)

# The game's working directory must be the build folder (it loads its .dat files from there).
cd "$BUILD"
