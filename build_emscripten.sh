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
rm -rf buildpack/Addons buildpack/docs/ghost buildpack/docs/tango buildpack/utilities
rm -rf buildpack/headers/ghost_zh/3.0/demo buildpack/headers/GUITest.qst
rm -rf "buildpack/scripts/stdWeapons/example scripts"
rm buildpack/changelog.txt
rm buildpack/music/Isabelle_Z2.nsf
find buildpack -name "*.dll" -type f -delete
find buildpack -name "*.so" -type f -delete
find buildpack -name "*.exe" -type f -delete
find buildpack -name "*.rtf" -type f -delete
find buildpack -name "*.pdf" -type f -delete
find buildpack -name "*.so" -type f -delete
find buildpack -name "*.zip" -type f -delete

rm -rf buildpack_zq
mkdir -p buildpack_zq
mkdir -p buildpack_zq/modules/classic/
mv buildpack/modules/classic/classic_qst.dat buildpack_zq/modules/classic/
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
  -s EXPORTED_FUNCTIONS=_main,_create_synthetic_key_event,_get_shareable_url,_open_test_mode
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

# Ensure that the SDL source code has been downloaded,
# otherwise the patches can't be applied.
if [ ! -d "$EMCC_CACHE_DIR/ports/sdl2" ]
then
  embuilder build sdl2
fi
if [ ! -d "$EMCC_CACHE_DIR/ports/sdl2_mixer" ]
then
  embuilder build sdl2_mixer
fi

# Manually delete libraries from Emscripten cache to force a rebuild.
rm -rf "$EMCC_CACHE_LIB_DIR"/libSDL2-mt.a
rm -rf "$EMCC_CACHE_LIB_DIR"/libSDL2_mixer_mid-mp3-ogg.a

sh ../../patches/apply.sh

TARGETS="${@:-zelda zquest}"
cmake --build . -t $TARGETS

"$(dirname $(which emcc))"/tools/file_packager.py zc.data \
  --no-node \
  --preload "../../output/_auto/buildpack@/" \
  --preload "../../freepats/freepats.cfg@/etc/timidity/freepats.cfg" \
  --use-preload-cache \
  --js-output=zc.data.js

# Zquest also uses zc.data
"$(dirname $(which emcc))"/tools/file_packager.py zq.data \
  --no-node \
  --preload "../../output/_auto/buildpack_zq@/" \
  --use-preload-cache \
  --js-output=zq.data.js

# https://github.com/emscripten-core/emscripten/issues/11952
function fix_hash {
  HASH=$(shasum -a 256 $1 | awk '{print $1}')
  sed -i -e "s/\"package_uuid\": \"[^\"]*\"/\"package_uuid\":\"$HASH\"/" "$2"
  if ! grep -q "$HASH" "$2"
  then
    echo "failed to replace data hash"
    exit 1
  fi
}

fix_hash zc.data zc.data.js
fix_hash zq.data zq.data.js

if [ -f zelda.html ]; then
  sed -i -e 's/__TARGET__/zelda/' zelda.html
  sed -i -e 's|__DATA__|<script src="zc.data.js"></script>|' zelda.html
fi
if [ -f zquest.html ]; then
  sed -i -e 's/__TARGET__/zquest/' zquest.html
  sed -i -e 's|__DATA__|<script src="zc.data.js"></script><script src="zq.data.js"></script>|' zquest.html
fi

cp -r ../../freepats .

# Now start a local webserver in the build_emscripten folder:
#   npx statikk --port 8000 --coi
