#!/bin/bash
# Shrinks a fuzz target's corpus to the smallest inputs that reach the same code, which also makes
# the fuzzer faster (it runs small inputs first, and more of them). See docs/fuzzing.md.
#
#   bash scripts/fuzzing/minimize_corpus.sh <target>
#
# The corpus it replaces is kept in .tmp/fuzz/corpus-old/. Delete that once the new one works.

set -eo pipefail

TARGET=$1
source "$( dirname -- "${BASH_SOURCE[0]}" )/common.sh"

NEW=$FUZZ/corpus/$TARGET.min
OLD=$FUZZ/corpus-old/$TARGET-$(date +%Y%m%d-%H%M%S)
rm -rf "$NEW"
mkdir -p "$NEW" "$FUZZ/corpus-old"

# With the largest input limit, so every input is read whole and nothing it reaches is lost.
./zplayer -merge=1 "${LIBFUZZER_FLAGS[@]}" -max_len=8388608 \
    "$NEW" "$FUZZ/corpus/$TARGET" $SEEDS 2>&1 | tee "$FUZZ/logs/$TARGET/minimize-$(date +%Y%m%d-%H%M%S).log"

mv "$FUZZ/corpus/$TARGET" "$OLD"
mv "$NEW" "$FUZZ/corpus/$TARGET"
echo "$TARGET: $(ls "$OLD" | wc -l) inputs ($(du -sh "$OLD" | cut -f1)) -> $(ls "$FUZZ/corpus/$TARGET" | wc -l) ($(du -sh "$FUZZ/corpus/$TARGET" | cut -f1))"
