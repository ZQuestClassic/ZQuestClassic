#!/bin/bash

set -e

if ! command -v emcc &> /dev/null
then
  echo "emcc could not be found. You must install emsdk"
  echo "https://emscripten.org/docs/getting_started/downloads.html"
  exit 1
fi

EMCC_VERSION=3.1.8
emsdk install $EMCC_VERSION
emsdk activate $EMCC_VERSION

cd output/_auto
./buildpack.sh
rm -f buildpack/{zelda,zquest,zlauncher,zscript}
rm -rf buildpack/Addons buildpack/docs/ghost buildpack/docs/tango
rm -rf buildpack/**/*.rtf buildpack/**/*.pdf
find buildpack -name "*.dll" -type f -delete
find buildpack -name "*.so" -type f -delete
find buildpack -name "*.exe" -type f -delete
cd -

mkdir -p build_emscripten
cd build_emscripten
if [[ "$DEBUG" ]]; then
  mkdir -p debug
  cd debug
else
  mkdir -p release
  cd release
fi

# Wish I knew how to remove this.
EMCC_CACHE_DIR="$(dirname $(which emcc))/cache"
EMCC_CACHE_INCLUDE_DIR="$EMCC_CACHE_DIR/sysroot/include"
EMCC_CACHE_LIB_DIR="$EMCC_CACHE_DIR/sysroot/lib/wasm32-emscripten"

# temporary workaround until fixed upstream
# emcc's cache will require you to manually do this (one time) for this to be picked up:
#    rm -rf $(dirname $(which emcc))/cache/sysroot/lib/wasm32-emscripten/libSDL2*
# Now you can run this script as normal.
# see https://github.com/libsdl-org/SDL/issues/5428
if [ ! -d "$EMCC_CACHE_DIR/ports/sdl2" ]
then
  embuilder build sdl2
  rm -rf "$EMCC_CACHE_LIB_DIR"
fi

# https://github.com/libsdl-org/SDL/pull/5496
if ! grep -q SDL_THREAD_PTHREAD_RECURSIVE_MUTEX "$EMCC_CACHE_DIR/ports/sdl2/SDL-release-2.0.20/include/SDL_config_emscripten.h"; then
  echo "#define SDL_THREAD_PTHREAD_RECURSIVE_MUTEX 1" >> "$EMCC_CACHE_DIR/ports/sdl2/SDL-release-2.0.20/include/SDL_config_emscripten.h"
fi

# SDL's emscripten audio specifies only one default audio output device, but turns out
# that can be ignored and things will just work. Without this, only SFX will play and MIDIs
# will error on opening a handle to the audio device.
sed -i -e 's/impl->OnlyHasDefaultOutputDevice = 1/impl->OnlyHasDefaultOutputDevice = 0/' "$EMCC_CACHE_DIR/ports/sdl2/SDL-release-2.0.20/src/audio/emscripten/SDL_emscriptenaudio.c"

EMCC_FLAGS=(
  -s USE_FREETYPE=1
  -s USE_VORBIS=1
  -s USE_OGG=1
  -s USE_LIBJPEG=1
  -s USE_SDL=2
  -s USE_SDL_MIXER=2
  -s USE_LIBPNG=1
  -s USE_PTHREADS=1
  -I "$EMCC_CACHE_INCLUDE_DIR/AL"
)
LINKER_FLAGS=(
  --shell-file="../../web/index.html"
  --shared-memory
  -s EXPORTED_FUNCTIONS=_main,_create_synthetic_key_event
  -s EXPORTED_RUNTIME_METHODS=cwrap
  -s FORCE_FILESYSTEM=1
  -s ASYNCIFY=1
  -s FULL_ES2=1
  -s SDL2_MIXER_FORMATS="['mid','ogg','mp3']"
  -s LLD_REPORT_UNDEFINED
  -s INITIAL_MEMORY=4229300224
  -s PTHREAD_POOL_SIZE=15
  # Necessary to avoid a deadlock. Bisected to here:
  # https://chromium.googlesource.com/external/github.com/emscripten-core/emscripten.git/+log/1a0b77c572ad..c48f73a5c763
  -s EXIT_RUNTIME=1
  -s MINIFY_HTML=0
  -lidbfs.js
  -lembind
)

CMAKE_BUILD_TYPE=""
if [[ "$DEBUG" ]]; then
  EMCC_FLAGS+=(
    -O2
    -g
  )
  LINKER_FLAGS+=(
    # --threadprofiler
    -s ASSERTIONS=1
  )
  # CMAKE_BUILD_TYPE="Debug"
  CMAKE_BUILD_TYPE="Release"
else
  EMCC_FLAGS+=(
    -O3
  )
  CMAKE_BUILD_TYPE="Release"
fi

# Find memory leaks.
# EMCC_FLAGS+=(-fsanitize=leak)
# LINKER_FLAGS+=(-fsanitize=leak -s EXIT_RUNTIME)

# LINKER_FLAGS+=(-s SAFE_HEAP=1)
# EMCC_FLAGS+=(--memoryprofiler)

emcmake cmake ../.. \
  -D CMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
  -D ALLEGRO_SDL=ON \
  -D WANT_ALLOW_SSE=OFF \
  -D WANT_OPENAL=OFF \
  -D WANT_ALSA=OFF \
  -D SDL2_INCLUDE_DIR="$EMCC_CACHE_INCLUDE_DIR" \
  -D SDL2_LIBRARY="$EMCC_CACHE_LIB_DIR/libSDL2-mt.a" \
  -D CMAKE_C_FLAGS="${EMCC_FLAGS[*]}" \
  -D CMAKE_CXX_FLAGS="${EMCC_FLAGS[*]}" \
  -D CMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS[*]}" \
  -D CMAKE_EXECUTABLE_SUFFIX_CXX=".html"

# The a5 SDL audio system hardcodes a value for # samples that is too high. Until I can upstream a patch to make this
# configurable, just manually change it here!
sed -i -e 's/4096/512/' _deps/allegro5-src/addons/audio/sdl_audio.c

# https://github.com/liballeg/allegro5/pull/1322
sed -i -e 's/(SDL_INIT_EVERYTHING)/(SDL_INIT_EVERYTHING-SDL_INIT_HAPTIC)/' _deps/allegro5-src/src/sdl/sdl_system.c

# This emscripten-specific timer code actually really messes up the framerate, making it go way too fast.
sed -i -e 's/ _al_timer_thread_handle_tick/\/\/_al_timer_thread_handle_tick/' _deps/allegro5-src/src/sdl/sdl_system.c

# Fix allegro's SDL's joysticks for emscripten.
# See https://github.com/liballeg/allegro5/pull/1326
sed -i -e 's/if (count <= 0)/if (false)/' _deps/allegro5-src/src/sdl/sdl_joystick.c
NEEDLE="joysticks = calloc(count, sizeof \* joysticks);"
sed -i -e "s/$NEEDLE$/joysticks = count > 0 ? calloc(count, sizeof * joysticks) : NULL;/" _deps/allegro5-src/src/sdl/sdl_joystick.c

# Fix allegro's SDL's support for joystick button names.
NEEDLE='= "button";'
sed -i -e "s/$NEEDLE$/= SDL_IsGameController(i) ? SDL_GameControllerGetStringForButton(b) : \"button\";/" _deps/allegro5-src/src/sdl/sdl_joystick.c

TARGETS="${@:-zelda zquest}"
cmake --build . -t $TARGETS

"$(dirname $(which emcc))"/tools/file_packager.py zc.data \
  --no-node \
  --preload "../../output/_auto/buildpack@/" \
  --preload "../../freepats@/etc/timidity" \
  --use-preload-cache \
  --js-output=zc.data.js

# https://github.com/emscripten-core/emscripten/issues/11952
HASH=$(shasum -a 256 zc.data | awk '{print $1}')
sed -i -e "s/\"package_uuid\": \"[^\"]*\"/\"package_uuid\":\"$HASH\"/" zc.data.js
if ! grep -q "$HASH" zc.data.js
then
  echo "failed to replace data hash"
  exit 1
fi

# Now start a local webserver in the build_emscripten folder:
#   npx statikk --port 8000 --coi
