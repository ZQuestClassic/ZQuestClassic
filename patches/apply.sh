#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# folder patch
function apply_patch_to_emcc_port {
  cd "$1"

  if [ -d .git ]; then
    git clean -fd
    git checkout -- .
  else
    git init
    git add .
    git commit -m init
  fi

  patch -p1 < "$2"
}

EMCC_CACHE_DIR="$(dirname $(which emcc))/cache"
apply_patch_to_emcc_port "$EMCC_CACHE_DIR/ports/sdl2/SDL-release-2.0.20" "$SCRIPT_DIR/sdl2.patch"
apply_patch_to_emcc_port "$EMCC_CACHE_DIR/ports/sdl2_mixer/SDL2_mixer-release-2.0.2" "$SCRIPT_DIR/sdl2_mixer.patch"
