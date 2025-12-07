#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"
rm -fr src include

[ ! -d 'dumb' ] && git clone https://github.com/kode54/dumb.git
git -C dumb checkout 396caa4d31859045ccb5ef943fd430ca4026cce8
cp -r dumb/src dumb/include dumb/LICENSE dumb/README.md .
rm -fr dumb
