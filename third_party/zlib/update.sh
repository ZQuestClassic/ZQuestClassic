#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"
rm -fr src include

[ ! -d 'zlib' ] && git clone https://github.com/madler/zlib.git
git -C zlib checkout 99623ef7097352da7993ba614161fd6a311e046e # v1.4

mkdir src include
cp -r zlib/*.c src
cp -r zlib/*.h include
cp zlib/LICENSE zlib/README zlib/LICENSE .
rm -fr zlib
