#!/bin/bash

# A very basic patching system. Only supports a single patch per directory.
# To update a patch:
#   1) cd to the directory
#   2) make your changes
#   3) git add -u
#   4) git diff --staged | pbcopy
#   5) overwrite existing patch file with new one

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
EMCC_DIR="$(dirname $(which emcc))"
EMCC_CACHE_DIR="$EMCC_DIR/cache"

NO_GIT_CLEAN=false
GIT_CLEAN=true

# folder patch
function apply_patch {
  cd "$1"
  echo "Applying patch: $2"
  echo "            to: $1"

  if [ -d .git ]; then
    git restore --staged .
    # Cleaning is the sensible thing to do, unless there are build-time generated
    # files (ex: allegro will create some header configuration files based on the environment).
    if $3 ; then
      git clean -fdq
    fi
    git checkout -- .
  else
    git init > /dev/null
    git add .
    git commit -m init
  fi

  patch -s -p1 < "$2"
  cd - > /dev/null
}

echo "Applying patches ..."

echo -e "__pycache__\ncache\n" > "$EMCC_DIR"/.gitignore
apply_patch "$EMCC_DIR" "$SCRIPT_DIR/emscripten.patch" $GIT_CLEAN

# Ensure that the SDL source code has been downloaded,
# otherwise the patches can't be applied.
if [ ! -d "$EMCC_CACHE_DIR/ports/sdl2" ]
then
  embuilder build sdl2-mt
  embuilder clear sdl2-mt
fi
if [ ! -d "$EMCC_CACHE_DIR/ports/sdl2_mixer/SDL_mixer-zc-fork-1" ]
then
  rm -rf "$EMCC_CACHE_DIR/ports/sdl2_mixer"
  embuilder build sdl2_mixer_mid
  embuilder clear sdl2_mixer_mid
fi

apply_patch "$EMCC_CACHE_DIR/ports/sdl2/SDL-release-2.32.0" "$SCRIPT_DIR/sdl2.patch" $GIT_CLEAN
apply_patch "$EMCC_CACHE_DIR/ports/sdl2_mixer/SDL_mixer-zc-fork-1" "$SCRIPT_DIR/sdl2_mixer.patch" $GIT_CLEAN
apply_patch _deps/allegro5-src "$SCRIPT_DIR/allegro5.patch" $NO_GIT_CLEAN

echo "Done applying patches!"
