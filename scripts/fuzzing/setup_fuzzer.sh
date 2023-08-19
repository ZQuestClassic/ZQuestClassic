#!/bin/bash

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT=$( dirname "$DIR" )/..

if [[ ! -n "${IN_DOCKER}" ]]; then
    docker pull aflplusplus/aflplusplus
    docker run --name zc-fuzz -it -d -v "$ROOT":/src aflplusplus/aflplusplus
    docker exec -e IN_DOCKER=1 -it zc-fuzz bash /src/scripts/fuzzing/setup_fuzzer.sh
    exit
fi

set -ex

# Setup deps.
apt update -y
apt install -y lsb-core software-properties-common ccache ninja-build build-essential gcc-multilib g++-multilib libx11-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev libxcursor1 libasound2-dev libgtk-3-dev flex bison elfutils

if cmake --version | grep -q '3.22'; then
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
    apt-add-repository -y "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
    apt update -y
    apt remove -y --purge --auto-remove cmake
    apt install -y cmake
fi

apt clean all

cd /src

# Build target.
if [ ! -d build_fuzz ]
then
    CC=afl-cc CCX=afl-cc++ cmake -B build_fuzz -S . -DWANT_FUZZ=ON
fi
cmake --build build_fuzz -t zplayer
cd build_fuzz
cp -r RelWithDebInfo/* .
cd -
export CI=1

# Create fuzz corpus.
if [ ! -d ".tmp/fuzz_corpus" ]
then
    python3 -m pip install Cython numpy
    bash scripts/fuzzing/create_fuzz_corpus.sh
fi

echo "fuzzer is ready to begin"
