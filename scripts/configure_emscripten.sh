#!/bin/bash

set -e

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT=$( dirname "$DIR" )

if ! command -v emsdk &> /dev/null
then
  echo "emsdk could not be found."
  echo "https://emscripten.org/docs/getting_started/downloads.html"
  exit 1
fi

cd "$ROOT"
mkdir -p build_emscripten

EMCC_VERSION=4.0.8
emsdk install $EMCC_VERSION
emsdk activate $EMCC_VERSION
source emsdk_env.sh

cd timidity
node convert-sf.js
cd -

cd build_emscripten

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
  -s USE_ZLIB=1
  -pthread
  -I "$EMCC_CACHE_INCLUDE_DIR/AL"
)
LINKER_FLAGS=(
  --shared-memory
  -s STACK_SIZE=5MB
  -s DEFAULT_PTHREAD_STACK_SIZE=2MB
  -s EXPORTED_RUNTIME_METHODS=cwrap,wasmMemory
  -s IMPORTED_MEMORY=1
  -s FORCE_FILESYSTEM=1
  -s ASYNCIFY=1
  -s FULL_ES2=1
  -s SDL2_MIXER_FORMATS="['mid']"
  -s INITIAL_MEMORY=200MB
  -s ALLOW_MEMORY_GROWTH=1
  -s PTHREAD_POOL_SIZE=15
  # Necessary to avoid a deadlock. Bisected to here:
  # https://chromium.googlesource.com/external/github.com/emscripten-core/emscripten.git/+log/1a0b77c572ad..c48f73a5c763
  -s EXIT_RUNTIME=1
  -s MINIFY_HTML=0
  -s ENVIRONMENT=web,worker
  -lidbfs.js
  -lproxyfs.js
  -lembind
  -pthread
)
EMCC_AND_LINKER_FLAGS=(
  # Error when using -fwasm-exceptions:
  #   unexpected expression type
  # UNREACHABLE executed at /opt/s/w/ir/cache/builder/emscripten-releases/binaryen/src/passes/Asyncify.cpp:1072!
  # em++: error: '/Users/connorclark/tools/emsdk/upstream/bin/wasm-opt --post-emscripten -O3 --low-memory-unused --asyncify --pass-arg=asyncify-asserts --pass-arg=asyncify-imports@env.invoke_*,env.__call_main,env.emscripten_sleep,env.emscripten_wget,env.emscripten_wget_data,env.emscripten_idb_load,env.emscripten_idb_store,env.emscripten_idb_delete,env.emscripten_idb_exists,env.emscripten_idb_load_blob,env.emscripten_idb_store_blob,env.SDL_Delay,env.emscripten_scan_registers,env.emscripten_lazy_load_code,env.emscripten_fiber_swap,wasi_snapshot_preview1.fd_sync,env.__wasi_fd_sync,env._emval_await,env._dlopen_js,env.__asyncjs__* --zero-filled-memory --strip-producers zquest.wasm -o zquest.wasm -g --mvp-features --enable-threads --enable-mutable-globals --enable-bulk-memory --enable-sign-ext --enable-exception-handling' failed (received SIGABRT (-6)
  # -fwasm-exceptions
  -fexceptions
  -profiling-funcs
)

# Makes linking faster, but can't actually run the result because ASYNCIFY is disabled.
# https://github.com/emscripten-core/emscripten/issues/18001
if [[ "$FAST_LINK" ]]; then
  LINKER_FLAGS+=(
    -s ASYNCIFY=0
    -s WASM_BIGINT
    -s LEGALIZE_JS_FFI=0
    -s ERROR_ON_WASM_CHANGES_AFTER_LINK
  )
fi

# Find memory leaks.
# EMCC_FLAGS+=(-fsanitize=leak)
# LINKER_FLAGS+=(-fsanitize=leak -s EXIT_RUNTIME)

# LINKER_FLAGS+=(-s SAFE_HEAP=1)
# EMCC_FLAGS+=(--memoryprofiler)

CMAKE_CXX_FLAGS="${EMCC_FLAGS[*]} ${EMCC_AND_LINKER_FLAGS[*]} -D_NPASS"
CMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS[*]} ${EMCC_AND_LINKER_FLAGS[*]}"

# Release flags.
CMAKE_CXX_FLAGS_RELEASE="${CMAKE_CXX_FLAGS[*]} -O3"
CMAKE_EXE_LINKER_FLAGS_RELEASE="${CMAKE_EXE_LINKER_FLAGS}"

# Debug flags.
CMAKE_CXX_FLAGS_DEBUG="${CMAKE_CXX_FLAGS[*]}"
CMAKE_EXE_LINKER_FLAGS_DEBUG="${CMAKE_EXE_LINKER_FLAGS}"

CMAKE_CXX_FLAGS_DEBUG+=(
  # Any lower and get "Compiling function failed: local count too large"
  -O2
)
CMAKE_EXE_LINKER_FLAGS_DEBUG+=(
  -s ASSERTIONS=1
)

# Separate debug info.
# Debug doesn't work great because of -O2 above, but can't get lower yet.
# One time, I saw this crash in DevTools ("extension accessed memory out of bounds"). That never seemed to happen when
# debug info was not separated. Shrug.
CMAKE_CXX_FLAGS_DEBUG+=(
  # Can't do DWARF 5 because ASYNCIFY (binaryren does not support: https://github.com/emscripten-core/emscripten/issues/17814#issuecomment-1256818854)
  # -gdwarf-5
  # -g -gsplit-dwarf -gpubnames
  -gseparate-dwarf
  # -gsplit-dwarf
)
CMAKE_EXE_LINKER_FLAGS_DEBUG+=(
  -gseparate-dwarf
)

# TODO: remove when this is fixed
# https://github.com/emscripten-core/emscripten/issues/18090
embuilder build sdl2

embuilder build libpng libpng-mt

embuilder build ogg vorbis

emcmake cmake \
  -G "Ninja Multi-Config" \
  -D ALLEGRO_SDL=ON \
  -D WANT_ALLOW_SSE=OFF \
  -D WANT_OPENAL=OFF \
  -D WANT_ALSA=OFF \
  -D SDL2_INCLUDE_DIR="$EMCC_CACHE_INCLUDE_DIR" \
  -D SDL2_LIBRARY="$EMCC_CACHE_LIB_DIR/libSDL2-mt.a" \
  -D ZLIB_INCLUDE_DIR="$EMCC_CACHE_INCLUDE_DIR" \
  -D ZLIB_LIBRARY="$EMCC_CACHE_LIB_DIR/libz.a" \
  -D PNG_INCLUDE_DIRS="$EMCC_CACHE_INCLUDE_DIR" \
  -D PNG_LIBRARIES="$EMCC_CACHE_LIB_DIR/libpng-mt.a" \
  -D VORBIS_INCLUDE_DIRS="$EMCC_CACHE_INCLUDE_DIR" \
  -D VORBIS_LIBRARIES="$EMCC_CACHE_LIB_DIR/libvorbis.a" \
  -D OGG_INCLUDE_DIRS="$EMCC_CACHE_INCLUDE_DIR" \
  -D OGG_LIBRARIES="$EMCC_CACHE_LIB_DIR/libogg.a" \
  -D CMAKE_C_FLAGS_RELEASE="${CMAKE_CXX_FLAGS_RELEASE[*]}" \
  -D CMAKE_CXX_FLAGS_RELEASE="${CMAKE_CXX_FLAGS_RELEASE[*]}" \
  -D CMAKE_EXE_LINKER_FLAGS_RELEASE="${CMAKE_EXE_LINKER_FLAGS_RELEASE[*]}" \
  -D CMAKE_C_FLAGS_RELWITHDEBINFO="${CMAKE_CXX_FLAGS_RELEASE[*]}" \
  -D CMAKE_CXX_FLAGS_RELWITHDEBINFO="${CMAKE_CXX_FLAGS_RELEASE[*]}" \
  -D CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO="${CMAKE_EXE_LINKER_FLAGS_RELEASE[*]}" \
  -D CMAKE_C_FLAGS_DEBUG="${CMAKE_CXX_FLAGS_DEBUG[*]}" \
  -D CMAKE_CXX_FLAGS_DEBUG="${CMAKE_CXX_FLAGS_DEBUG[*]}" \
  -D CMAKE_EXE_LINKER_FLAGS_DEBUG="${CMAKE_EXE_LINKER_FLAGS_DEBUG[*]}" \
  ${ZC_EMCC_CMAKE_EXTRA_FLAGS[@]} \
  ..

# TODO: can this be removed?
# Manually delete libraries from Emscripten cache to force a rebuild.
rm -rf "$EMCC_CACHE_LIB_DIR"/libSDL2-mt.a "$EMCC_CACHE_LIB_DIR"/libSDL2.a
rm -rf "$EMCC_CACHE_LIB_DIR"/libSDL2_mixer_mid.a
# This would work except you can't clear port variants.
# https://github.com/emscripten-core/emscripten/issues/16744
# embuilder clear sdl2-mt sdl2_mixer_gme_mid_mod_mp3_ogg

bash ../web/patches/apply.sh

# TODO: why doesn't emscripten build this for us?
embuilder build sdl2-mt
embuilder build sdl2_mixer_mid

echo "
done configuring emscripten build.

to build the web version:
  cmake --build build_emscripten --config Release -t web

to build just a single app:
  cmake --build build_emscripten --config Release -t web_zplayer
  cmake --build build_emscripten --config Release -t web_zeditor

you only need to re-run configure_emscripten.sh if something in this file is changed

be sure to start a local webserver in the web package folder:
  cd build_emscripten/Release/packages/web && npx statikk --port 8000 --coi
"
