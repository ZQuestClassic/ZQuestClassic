#!/bin/bash

set -ex

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT=$( dirname "$DIR" )/..
TEST_DIR="$ROOT/.tmp/fuzz_corpus"

rm -rf "$TEST_DIR"
mkdir -p "$TEST_DIR"

# Add an entire quest file.
cp "$ROOT/tests/replays/classic_1st/classic_1st.qst" "$TEST_DIR"

# Add every replay test file <5MB, but trim some large sections.
SECTIONS="TILES MIDIS MAPS CSETS DMAPS"
cd "$ROOT/.tmp"
if [ ! -d "zquest-data" ]
then
    git clone https://github.com/connorjclark/zquest-data.git zquest-data
fi
cd zquest-data
git pull
python3 setup.py install
mkdir "$TEST_DIR/trim_these"
# TODO: (2025) this probably doesn't work anymore. Need to copy directory structure.
cp "$ROOT"/tests/replays/**.qst "$TEST_DIR/trim_these"
find "$TEST_DIR/trim_these" -type f -name '*.qst' -size +5M -delete
find "$TEST_DIR/trim_these" -name '*.qst' \
    -exec bash -c "echo \"{}\" ; echo ======= ; python3 examples/trim_sections.py \"{}\" --output {} --trim $SECTIONS" \;
mv "$TEST_DIR"/trim_these/*.qst "$TEST_DIR"
rmdir "$TEST_DIR/trim_these"

# Uncompress files - fuzzing works best on uncompressed data.
find "$TEST_DIR" -name '*.qst' \
    -exec bash -c "echo \"{}\" ; echo ======= ; python3 examples/uncompress.py \"{}\" --output {}" \;

# Minify inputs.
cd "$ROOT/build_fuzz"
afl-cmin -i "$TEST_DIR" -o "$ROOT/.tmp/fuzz_corpus_unique" -t 3000 -- ./zplayer -load-and-quit @@
rm -rf "$TEST_DIR"
mv "$ROOT/.tmp/fuzz_corpus_unique" "$TEST_DIR"

# This takes too long!
# export AFL_MAP_SIZE=258173
# for i in "$TEST_DIR"/*; do
#   afl-tmin -i "$i" -o "$ROOT/.tmp/fuzz_corpus_minify" -t 3000 -- ./zplayer -load-and-quit @@
# done
# rm -rf "$TEST_DIR"
# mv "$ROOT/.tmp/fuzz_corpus_minify" "$TEST_DIR"
