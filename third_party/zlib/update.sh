#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"
rm -fr src include

SHA=51b7f2abdade71cd9bb0e7a373ef2610ec6f9daf # v1.3.1
python3 ../update_dependency.py https://github.com/madler/zlib/archive/${SHA}.tar.gz zlib

mkdir src include
cp -r zlib/*.c src
cp -r zlib/*.h include
cp zlib/LICENSE zlib/README .
rm -fr zlib
