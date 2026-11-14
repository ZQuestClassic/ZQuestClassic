#!/bin/bash
# Runs one zplayer fuzz target. See docs/fuzzing.md.
#
#   bash scripts/fuzzing/run_fuzzer.sh <target> [jobs] [extra libFuzzer flags...]
#
# Crashes land in .tmp/fuzz/artifacts/<target>/, the growing corpus in .tmp/fuzz/corpus/<target>/,
# and the output of each run in .tmp/fuzz/logs/<target>/. Stop it with Ctrl+C; running it again
# picks up where it left off.

set -e

TARGET=$1
JOBS=${2:-4}
shift 2 || shift $#

source "$( dirname -- "${BASH_SOURCE[0]}" )/common.sh"

# How long an input the fuzzer makes (it cuts longer ones from the corpus down to this). The time
# to run one grows with its size: a 7 MB quest takes 100 ms, a 5 KB one 7 ms. A low limit gets
# through far more inputs, but loses what only the long ones in the corpus reach (a fifth of the
# qst corpus's coverage at 256 KB), so it is best as a second run next to one at the full limit.
MAX_LEN=${MAX_LEN:-8388608}

./zplayer \
    -fork="$JOBS" \
    -ignore_crashes=1 -ignore_timeouts=1 -ignore_ooms=1 \
    -close_fd_mask=1 \
    "${LIBFUZZER_FLAGS[@]}" \
    -max_len="$MAX_LEN" \
    -artifact_prefix="$FUZZ/artifacts/$TARGET/" \
    "$@" \
    "$FUZZ/corpus/$TARGET" $SEEDS 2>&1 | tee "$FUZZ/logs/$TARGET/$(date +%Y%m%d-%H%M%S)-$$.log"
