#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$(dirname $(which emcc))/cache/ports/sdl2_mixer/SDL2_mixer-release-2.0.2"

if [ -d .git ]; then
  git clean -fd
  git checkout -- .
else
  git init
  git add .
  git commit -m init
fi

patch -p1 < "$SCRIPT_DIR/sdl2_mixer.patch"
