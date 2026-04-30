#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"
rm -fr src include COPYING

SHA=0288fadac3ac62d453409dfc83e9c4ab617d2472
python3 ../update_dependency.py https://github.com/xiph/ogg/archive/${SHA}.tar.gz ogg_temp

cp -r ogg_temp/src ogg_temp/include ogg_temp/COPYING .
rm -fr ogg_temp
