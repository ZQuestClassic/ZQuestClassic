#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
EMCC_CACHE_DIR="$(dirname $(which emcc))/cache"

# folder patch
function apply_patch {
  cd "$1"

  if [ -d .git ]; then
    git checkout -- .
  else
    git init
    git add .
    git commit -m init
  fi

  patch -p1 < "$2"
  cd -
}

apply_patch "$EMCC_CACHE_DIR/ports/sdl2/SDL-release-2.0.20" "$SCRIPT_DIR/sdl2.patch"
apply_patch "$EMCC_CACHE_DIR/ports/sdl2_mixer/SDL2_mixer-release-2.0.2" "$SCRIPT_DIR/sdl2_mixer.patch"
apply_patch _deps/allegro5-src "$SCRIPT_DIR/allegro5.patch"
