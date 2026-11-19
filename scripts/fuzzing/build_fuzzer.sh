#!/bin/bash
# Configures and builds the fuzzing build of zplayer in build_fuzz/. See docs/fuzzing.md.

set -e

ROOT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/../.." &> /dev/null && pwd )
BUILD=${BUILD_FOLDER:-$ROOT/build_fuzz}

# libFuzzer ships with LLVM's clang, but not Apple's.
if [ -z "$CC" ]; then
    if [[ "$(uname)" == "Darwin" ]]; then
        LLVM=$(brew --prefix llvm 2>/dev/null || true)
        if [ ! -x "$LLVM/bin/clang" ]; then
            echo "Needs LLVM's clang for libFuzzer: brew install llvm"
            exit 1
        fi
        export CC=$LLVM/bin/clang CXX=$LLVM/bin/clang++
    else
        export CC=clang CXX=clang++
    fi
fi

EXTRA_ARGS=()
if [[ "$(uname)" == "Darwin" ]]; then
    # Without an explicit sysroot, Homebrew's clang puts the SDK's include folder (found for
    # libcurl) ahead of libc++'s headers.
    EXTRA_ARGS+=(-DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)")
    if [ -x /opt/homebrew/opt/bison/bin/bison ]; then
        EXTRA_ARGS+=(-DBISON_EXECUTABLE=/opt/homebrew/opt/bison/bin/bison)
    fi
fi

if [ ! -f "$BUILD/CMakeCache.txt" ]; then
    # Asan: ASan at -O1. Asserts are off (NDEBUG), as in a release build: only bugs a quest or
    # save could exploit in a release matter here, and an assert can stop the run just before
    # one. WANT_ERROR_CHECKING is off because a newer clang than CI's turns new warnings into
    # errors; Allegro's Objective-C trips one too. FMT_CONSTEVAL is empty because the vendored fmt
    # is too old for a newer clang's consteval checks.
    cmake -B "$BUILD" -S "$ROOT" -G Ninja \
        -DCMAKE_BUILD_TYPE=Asan \
        -DWANT_FUZZ=ON \
        -DWANT_ZUPDATER=OFF \
        -DWANT_GIT_HOOKS=OFF \
        -DWANT_ERROR_CHECKING=OFF \
        "-DCMAKE_C_FLAGS=-DNDEBUG -Wno-error=incompatible-pointer-types" \
        "-DCMAKE_CXX_FLAGS=-DNDEBUG -DFMT_CONSTEVAL=" \
        "${EXTRA_ARGS[@]}"
fi

cmake --build "$BUILD" -t zplayer copy_resources
