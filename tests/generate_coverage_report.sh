#!/bin/bash

# You should first run something like:
#   cmake -S . -B build -G 'Ninja Multi-Config'
#
# Then clear results from a previous run:
#   find build/ -name '*.gcov' -delete
#   find build/ -name '*.gcda' -delete
#   find build/ -name '*.gcno' -delete
#
# Then run instrumented binaries:
#   cmake --build build --config Coverage --clean-first
#   python tests/run_replay_tests.py --build_folder build/Coverage --replay --ci

set -e

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT=$( dirname "$DIR" )

cd "$DIR"
rm -rf .coverage || true
mkdir -p .coverage/report

cd "$ROOT"
gcovr \
  -v \
  --html-details tests/.coverage/report/index.html \
  --json tests/.coverage/coverage.json \
  --html-title "ZQuest Classic Coverage Report" \
  --html-theme blue \
  --sort-key uncovered-percent \
  -r . \
  -f src \
  -d \
  "$@"

echo "Open $DIR/.coverage/report/index.html in your browser to view the coverage report."
