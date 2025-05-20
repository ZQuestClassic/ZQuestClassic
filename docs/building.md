Some prerequisites to building from source:

- CMake (3.28 or later)
- Install flexbison (Windows, use https://chocolatey.org/: `choco install winflexbison3`)
- For Windows: Visual Studio 2019 (but later is better)

# Configuring the build

Advanced CMake users can likely skip this section.

You can either use the CMake GUI, or the following command to initialize the build system:

```sh
cmake -S . -B build
```

On Windows, this defaults to creating a Visual Studio solution file. You can open that to actually build.

You can also build from the terminal:

```sh
cmake --build build
```

Tip: use `-t` to build a single target (instead of everything), and use `--config Release` (or `--config Debug`) to specify the optimization level.

OSX/Linux developers will benefit from using Ninja (this is faster than the default Makefile generator). For a multi-configuration Ninja build, use: `cmake -G 'Ninja Multi-Config' -S . -B build`.

After building, you'll find the executables in `build/{Release|Debug}`.

# Quick-start: Windows with MSVC and CMake GUI

Download CMake and run the CMake GUI. It will prompt your for the location of the source code, and the location in which to build the binaries. Specify the root (the folder containing this file) for the former and the `build` folder for the latter.

Click "Generate." This will create a Visual Studio project file for you in the build directory. You can then open up the project file in MSVC and do editing/compilation/debugging in MSVC. You do not need to touch CMake again unless you want to change project configuration options or add/remove source files.

# Building on Linux

## Install Dependencies

### Linux Mint

```sh
sudo apt update
sudo apt install python3-pip ninja-build libgtk-3-dev libasound2-dev libssl-dev libcurl4-openssl-dev libstdc++-12-dev
# Can skip this if already have cmake 3.24 or higher
pip install cmake --upgrade
```

### Ubuntu

```sh
sudo apt update
sudo apt install build-essential gcc-multilib g++-multilib libx11-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev libxcursor1 libasound2-dev libgtk-3-dev libdumb1-dev flex bison
```

Then before running, you need some additional packages installed:
<!-- TODO: why both? I think when using GCC we need to link with libopengl, but with clang we need libglu? -->
```sh
sudo apt install libopengl0 libglu1
```

## Building

> NOTE: currently gcc cannot be used, so use clang 17+

Install clang 17:

```sh
wget https://apt.llvm.org/llvm.sh
chmod u+x llvm.sh
sudo ./llvm.sh 17
# verify
clang-17 --version
# clean up
rm llvm.sh
```

Configure:

```sh
# be in the ZQuestClassic git checkout
# Configure
CC=clang-17 CXX=clang++-17 cmake -G 'Ninja Multi-Config' -B build -S .
```

> NOTE: skip to "Installing" if you are not developing and just want a user installation

Build:

```sh
cmake --build build --config Debug
```

Run:

```sh
cd build/Debug
./zlauncher
```

## Installing

```sh
cmake --build build --config Release -t install
```

By default this will install to `~/zquestclassic`. You can open the launcher with: `cd ~/zquestclassic; ./bin/zlauncher`

# Building w/ OGG support on Windows

You'll need to install libogg and libvorbis. This is typically already present on Unix systems, but for Windows you can use `vcpkg`:

```sh
cd ~/tools
git clone https://github.com/microsoft/vcpkg
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg.exe install --triplet x64-windows libogg libvorbis
```

You then need to configure your CMake build with the `vcpkg` toolchain:

```sh
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/c/Users/cjamc/tools/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Note: this needs to be a fresh build folder, otherwise the toolchain won't be updated.

If these libraries are not present, you can still build but won't be able to play OGG music.

# Building w/ tracker (DUMB - IT, XM, S3M, MOD) for updater on Windows

```sh
./vcpkg.exe install --triplet x64-windows dumb
```

# Building w/ CURL for updater on Windows

```sh
./vcpkg.exe install --triplet x64-windows curl
```

# Building with Ninja and MSVC

Typically, on Windows you want to use Visual Studio as the cmake generator, but if you want to use Ninja here's how:

> 1. Ensure you have installed the C++/CLI tools for C++ desktop development in the Visual Studio Installer [(reference)](https://gitlab.kitware.com/cmake/cmake/-/issues/19815#note_636971)
> 
> 1. Launch `cmd.exe` and configure the environment to use the Microsoft toolchain: `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`
Change 64 to 32 if you want to build 32bit.
> 1. Configure: `cmake -S . -B build -G "Ninja Multi-Config" -DCMAKE_WIN32_EXECUTABLE=1`
> 1. Build: `cmake --build build --config Debug -t zplayer`

One reason to use Ninja is to use clang instead of MSVC (just skip the environment configuration above);
or to use `ccache` for faster builds.

# Building the Web version

First setup the `build_emscripten` cmake folder:

```sh
bash scripts/configure_emscripten.sh
```

Now, build with cmake as normal. Note the output of the above script:

> to build the web version:
>   cmake --build build_emscripten --config Debug -t web
> 
> to build just a single app:
>   cmake --build build_emscripten --config Debug -t web_zplayer
>   cmake --build build_emscripten --config Debug -t web_zeditor
> 
> you only need to re-run configure_emscripten.sh if something in this file is changed
> 
> be sure to start a local webserver in the web package folder:
>   cd build_emscripten/Debug/packages/web && npx statikk --port 8000 --coi

# ccache

`ccache` can be used to cache build results, making switching between branches and re-building
much faster.

> OSX: `brew install ccache`
>
> Windows: `choco install ccache`

When configuring cmake, use these flags:
> `-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache`

<!-- set CCACHE_CONFIGPATH=C:\Users\cjamc\code\ZeldaClassic\ccache.conf -->

Note: you'll need to set the env variable `CCACHE_SLOPPINESS=time_macros`.

Note: on Windows, Debug can't be cached yet. https://github.com/ccache/ccache/issues/1040

Some care is needed to ensure build outputs are determinstic. If you do a clean build (`cmake --build build --clean-first`), and then run `ccache -z` and do another clean build, then `ccache -s` will give a report of cache misses/hits. The cache should be full and there should be 0 misses. If there are misses, something in the build output is non deterministic. Read this [article](https://interrupt.memfault.com/blog/ccache-debugging) for how to debug.

Other useful links:

- https://crascit.com/2016/04/09/using-ccache-with-cmake/
