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

EMCC_VERSION=3.1.24
emsdk install $EMCC_VERSION
emsdk activate $EMCC_VERSION
source emsdk_env.sh

cd timidity
node convert-sf.js
cd -

cd web
npm install
cd -

packages_dir="$ROOT/build_emscripten/packages"
python scripts/package.py --build_folder build_emscripten --skip_binaries --skip_archive --cfg_os web

cd "$packages_dir/zc"
rm -rf docs/ghost docs/tango
rm -rf headers/ghost_zh/3.0/demo headers/GUITest.qst
rm -rf "scripts/stdWeapons/example scripts"
rm changelog.txt
rm music/Isabelle_Z2.nsf
find . -name "*.rtf" -type f -delete
find . -name "*.pdf" -type f -delete
find . -name "*.zip" -type f -delete
cd -

cd "$packages_dir"
LAZY_LOAD=(
  tilesets/classic.qst
)
mkdir -p zc_lazy
for f in ${LAZY_LOAD[@]}; do
  dir=$(dirname $f)
  mkdir -p "zc_lazy/$dir"
  mv "zc/$f" "zc_lazy/$dir"
done

rm -rf zq
mkdir -p zq
mkdir -p zq/modules/classic/
mv zc/modules/classic/classic_qst.dat zq/modules/classic/
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
  -s USE_PTHREADS=1
  -I "$EMCC_CACHE_INCLUDE_DIR/AL"
)
LINKER_FLAGS=(
  --shared-memory
  -s EXPORTED_RUNTIME_METHODS=cwrap
  -s FORCE_FILESYSTEM=1
  -s ASYNCIFY=1
  -s FULL_ES2=1
  -s SDL2_MIXER_FORMATS="['mid','mod','ogg','mp3','gme']"
  -s LLD_REPORT_UNDEFINED
  -s INITIAL_MEMORY=200MB
  -s ALLOW_MEMORY_GROWTH=1
  -s PTHREAD_POOL_SIZE=15
  # Necessary to avoid a deadlock. Bisected to here:
  # https://chromium.googlesource.com/external/github.com/emscripten-core/emscripten.git/+log/1a0b77c572ad..c48f73a5c763
  -s EXIT_RUNTIME=1
  -s MINIFY_HTML=0
  -lidbfs.js
  -lproxyfs.js
  -lembind
  -lpthread
)
EMCC_AND_LINKER_FLAGS=(
  # Error when using -fwasm-exceptions:
  #   unexpected expression type
  # UNREACHABLE executed at /opt/s/w/ir/cache/builder/emscripten-releases/binaryen/src/passes/Asyncify.cpp:1072!
  # em++: error: '/Users/connorclark/tools/emsdk/upstream/bin/wasm-opt --post-emscripten -O3 --low-memory-unused --asyncify --pass-arg=asyncify-asserts --pass-arg=asyncify-imports@env.invoke_*,env.__call_main,env.emscripten_sleep,env.emscripten_wget,env.emscripten_wget_data,env.emscripten_idb_load,env.emscripten_idb_store,env.emscripten_idb_delete,env.emscripten_idb_exists,env.emscripten_idb_load_blob,env.emscripten_idb_store_blob,env.SDL_Delay,env.emscripten_scan_registers,env.emscripten_lazy_load_code,env.emscripten_fiber_swap,wasi_snapshot_preview1.fd_sync,env.__wasi_fd_sync,env._emval_await,env._dlopen_js,env.__asyncjs__* --zero-filled-memory --strip-producers zquest.wasm -o zquest.wasm -g --mvp-features --enable-threads --enable-mutable-globals --enable-bulk-memory --enable-sign-ext --enable-exception-handling' failed (received SIGABRT (-6)
  # -fwasm-exceptions
  -fexceptions
)

CONFIG=""
if [[ "$DEBUG" ]]; then
  EMCC_FLAGS+=(
    -O2
    -g
    -DEMSCRIPTEN_DEBUG
  )
  LINKER_FLAGS+=(
    -s ASSERTIONS=1
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

  CONFIG="Debug"
else
  EMCC_FLAGS+=(
    -O3
  )
  CONFIG="Release"
fi

function build_js {
  ESBUILD_ARGS=()
  if ! [[ "$DEBUG" ]]; then
    ESBUILD_ARGS+=(
      --minify
    )
  fi
  npx esbuild --bundle ../../web/main.js --outfile=main.js --sourcemap ${ESBUILD_ARGS[@]}
  cp ../../web/styles.css .
}

# cd $CONFIG && build_js && exit 0

# Find memory leaks.
# EMCC_FLAGS+=(-fsanitize=leak)
# LINKER_FLAGS+=(-fsanitize=leak -s EXIT_RUNTIME)

# LINKER_FLAGS+=(-s SAFE_HEAP=1)
# EMCC_FLAGS+=(--memoryprofiler)

# TODO: remove when this is fixed
# https://github.com/emscripten-core/emscripten/issues/18090
embuilder build sdl2

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
  -D CMAKE_C_FLAGS="${EMCC_FLAGS[*]} ${EMCC_AND_LINKER_FLAGS[*]}" \
  -D CMAKE_CXX_FLAGS="${EMCC_FLAGS[*]} ${EMCC_AND_LINKER_FLAGS[*]} -D_NPASS" \
  -D CMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS[*]} ${EMCC_AND_LINKER_FLAGS[*]}" \
  ${ZC_EMCC_CMAKE_EXTRA_FLAGS[@]} \
  ..

# TODO: can this be removed?
# Manually delete libraries from Emscripten cache to force a rebuild.
rm -rf "$EMCC_CACHE_LIB_DIR"/libSDL2-mt.a "$EMCC_CACHE_LIB_DIR"/libSDL2.a
rm -rf "$EMCC_CACHE_LIB_DIR"/libSDL2_mixer_gme_mid-mod-mp3-ogg.a
# This would work except you can't clear port variants.
# https://github.com/emscripten-core/emscripten/issues/16744
# embuilder clear sdl2-mt sdl2_mixer_gme_mid_mod_mp3_ogg

bash ../web/patches/apply.sh

# TODO: why doesn't emscripten build this for us?
embuilder build sdl2-mt
embuilder build sdl2_mixer_gme_mid_mod_mp3_ogg

TARGETS="${@:-zelda zquest zscript}"
cmake --build . --config $CONFIG -t $TARGETS
cd $CONFIG

"$(dirname $(which emcc))"/tools/file_packager.py zc.data \
  --no-node \
  --preload "$packages_dir/zc@/" \
  --preload "../../timidity/zc.cfg@/etc/zc.cfg" \
  --preload "../../timidity/ultra.cfg@/etc/ultra.cfg" \
  --preload "../../timidity/ppl160.cfg@/etc/ppl160.cfg" \
  --preload "../../timidity/freepats.cfg@/etc/freepats.cfg" \
  --preload "../../timidity/soundfont-pats/oot.cfg@/etc/oot.cfg" \
  --preload "../../timidity/soundfont-pats/2MGM.cfg@/etc/2MGM.cfg" \
  --use-preload-cache \
  --js-output=zc.data.js

# Zquest also uses zc.data
"$(dirname $(which emcc))"/tools/file_packager.py zq.data \
  --no-node \
  --preload "$packages_dir/zq@/" \
  --use-preload-cache \
  --js-output=zq.data.js

function set_files {
  R=$(jq --compact-output --null-input '$ARGS.positional' --args "${LAZY_LOAD[@]}")
  sed -i -e "s|files: \[\]|files: $R|" $1
}

function insert_css {
  sed " /*__INLINECSS__*/  r ../../web/styles.css" "$1" > tmp.html
  mv tmp.html "$1"
}

if [[ "${TARGETS[*]}" =~ "zelda" ]]; then
  cp ../../web/index.html zelda.html
  sed -i -e 's/__TARGET__/zelda/' zelda.html
  sed -i -e 's|__DATA__|<script src="zc.data.js"></script>|' zelda.html
  sed -i -e 's|__SCRIPT__|<script async src="zelda.js"></script>|' zelda.html
  set_files zelda.html
  insert_css zelda.html
fi
if [[ "${TARGETS[*]}" =~ "zquest" ]]; then
  cp ../../web/index.html zquest.html
  sed -i -e 's/__TARGET__/zquest/' zquest.html
  sed -i -e 's|__DATA__|<script src="zc.data.js"></script><script src="zq.data.js"></script>|' zquest.html
  sed -i -e 's|__SCRIPT__|<script async src="zquest.js"></script>|' zquest.html
  set_files zquest.html
  insert_css zquest.html
fi

cp -r ../../timidity .

rm -rf files
mv "$packages_dir/zc_lazy" files

build_js

echo "done configuring emscripten build."
echo ""
echo "run the ninja build rule (ex: ninja -f build-$CONFIG.ninja $CONFIG/zelda.js) to rebuild a target"
echo "you only need to re-run build_emscripten.sh if something in this file is changed"
echo ""
echo "be sure to start a local webserver in the build_emscripten folder:"
echo "    npx statikk --port 8000 --coi"
