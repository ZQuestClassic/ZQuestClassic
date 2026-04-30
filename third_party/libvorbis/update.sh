#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"
rm -fr lib include COPYING

SHA=2d79800b6751dddd4b8b4ad50832faa5ae2a00d9
python3 ../update_dependency.py https://github.com/xiph/vorbis/archive/${SHA}.tar.gz vorbis_temp

cp -r vorbis_temp/lib vorbis_temp/include vorbis_temp/COPYING .
rm -fr vorbis_temp
