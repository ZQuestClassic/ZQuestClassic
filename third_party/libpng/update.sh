#!/bin/bash

set -e
shopt -s extglob

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"
rm -rf !(update.sh)

[ ! -d 'libpng' ] && git clone https://github.com/pnggroup/libpng.git
git -C libpng checkout cd952f49f95bb27154ae77dbb103032d95f6e580 # v1.6.59

mkdir src
cp -r libpng/*.c libpng/*.h libpng/intel libpng/arm src
mkdir scripts
cp libpng/scripts/pnglibconf.h.prebuilt scripts
cp libpng/LICENSE libpng/README libpng/LICENSE .
rm src/example.c
rm src/pngtest.c
rm -fr libpng
