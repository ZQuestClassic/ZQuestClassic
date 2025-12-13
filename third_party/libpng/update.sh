#!/bin/bash

set -e
shopt -s extglob

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"
rm -rf !(update.sh)

[ ! -d 'libpng' ] && git clone https://github.com/pnggroup/libpng.git
git -C libpng checkout 49363adcfaf098748d7a4c8c624ad8c45a8c3a86 # v1.6.51

mkdir src
cp -r libpng/*.c libpng/*.h libpng/intel libpng/arm src
mkdir scripts
cp libpng/scripts/pnglibconf.h.prebuilt scripts
cp libpng/LICENSE libpng/README libpng/LICENSE .
rm src/example.c
rm src/pngtest.c
rm -fr libpng
