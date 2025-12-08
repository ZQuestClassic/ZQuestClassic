#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"
rm -fr src include

[ ! -d 'libjpeg-turbo' ] && git clone https://github.com/libjpeg-turbo/libjpeg-turbo
git -C libjpeg-turbo checkout 826a4f86f746c32833a21d8d35bc4a7707b5dec3
cp -r libjpeg-turbo/src libjpeg-turbo/LICENSE.md libjpeg-turbo/README.ijg .
rm -fr src/md5
rm -fr libjpeg-turbo
