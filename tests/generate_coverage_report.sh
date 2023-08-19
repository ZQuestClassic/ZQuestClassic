#!/bin/bash

# You should first run something like:
#   cmake -S . -B build -G 'Ninja Multi-Config'
#   cmake --build build --config Coverage -t zelda
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
  --html-title "Zelda Classic Coverage Report" \
  --html-theme blue \
  --sort-percentage \
  -r . \
  -f src \
  -e src/dialog \
  -e src/gui \
  -e src/editbox \
  -e src/jwin \
  -e src/base/jwinfsel \
  -e src/base/gui \
  -e src/base/process_management \
  -d \
  "$@"

echo "Open $DIR/.coverage/report/index.html in your browser to view the coverage report."
