#!/bin/bash

set -e

if ! command -v emcc &> /dev/null
then
  echo "emcc could not be found. You must install emsdk"
  echo "https://emscripten.org/docs/getting_started/downloads.html"
  exit 1
fi

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

# Wish I knew how to remove this.
EMCC_CACHE_INCLUDE_DIR=$(dirname $(which emcc))/cache/sysroot/include
EMCC_CACHE_LIB_DIR=$(dirname $(which emcc))/cache/sysroot/lib/wasm32-emscripten

# temporary workaround until fixed upstream
# emcc's cache will require you to manually do this (one time) for this to be picked up:
#    embuilder build sdl2
#    <run that sed command below>
#    rm -rf "$(dirname $(which emcc))/cache/sysroot/lib/wasm32-emscripten"
#    Now you can run this script as normal.
# see https://github.com/libsdl-org/SDL/issues/5428
sed -i -e 's/#define FAKE_RECURSIVE_MUTEX 1//' $(dirname $(which emcc))/cache/ports/sdl2/SDL-release-2.0.20/src/thread/pthread/SDL_sysmutex.c

EMCC_FLAGS=(
  -s USE_FREETYPE=1
  -s USE_VORBIS=1
  -s USE_OGG=1
  -s USE_LIBJPEG=1
  -s USE_SDL=2
  -s USE_LIBPNG=1
  -s USE_PTHREADS=1
  -I "$EMCC_CACHE_INCLUDE_DIR/AL"
)
LINKER_FLAGS=(
  --preload-file="../output/_auto/buildpack@/"
  --shared-memory
  -s ASYNCIFY=1
  -s FULL_ES2=1
  -s LLD_REPORT_UNDEFINED
  -s INITIAL_MEMORY=4229300224
  -s PTHREAD_POOL_SIZE=13
)

CMAKE_BUILD_TYPE=""
if [[ "$DEBUG" ]]; then
  EMCC_FLAGS+=(
    -O2
    -g
  )
  LINKER_FLAGS+=(
    --threadprofiler
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

# -D ALLEGRO_WAIT_EVENT_SLEEP=ON \
emcmake cmake .. \
  -D CMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
  -D ALLEGRO_SDL=ON \
  -D WANT_ALLOW_SSE=OFF \
  -D WANT_AUDIO=ON \
  -D WANT_OPENAL=ON \
  -D WANT_ALSA=OFF \
  -D SDL2_INCLUDE_DIR="$EMCC_CACHE_INCLUDE_DIR" \
  -D SDL2_LIBRARY="$EMCC_CACHE_LIB_DIR/libSDL2-mt.a" \
  -D CMAKE_C_FLAGS="${EMCC_FLAGS[*]}" \
  -D CMAKE_CXX_FLAGS="${EMCC_FLAGS[*]}" \
  -D CMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS[*]}" \
  -D CMAKE_EXECUTABLE_SUFFIX_CXX=".html"

cmake --build . -t zelda
