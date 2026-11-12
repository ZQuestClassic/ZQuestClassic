#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"
rm -fr src include

SHA=99623ef7097352da7993ba614161fd6a311e046e # v1.4
python3 ../update_dependency.py https://github.com/madler/zlib/archive/${SHA}.tar.gz zlib

mkdir src include
cp -r zlib/*.c src
cp -r zlib/*.h include
cp zlib/LICENSE zlib/README .
rm -fr zlib
