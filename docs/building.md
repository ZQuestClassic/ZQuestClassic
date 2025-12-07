Some prerequisites to building from source:

- CMake (3.29 or later)
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

# Building on Windows

## Dependencies

- Install flexbison (Windows, use https://chocolatey.org/: `choco install winflexbison3`)

## (optional) Optional dependencies

Various features rely on third-party libraries that are optional for building. Without these libraries, the features will be disabled. The features include:

* OGG support (libogg, libvorbis)
* Updater / replay uploader (curl)
* Secure websockets (openssl)

To get these libraries on Windows, you can use `vcpkg`:

```sh
# need to place vcpkg somewhere. I like to use ~/tools but can be anywhere.
cd ~/tools

# download and setup vcpkg
git clone https://github.com/microsoft/vcpkg
cd vcpkg
./bootstrap-vcpkg.bat

# install libraries
./vcpkg.exe install --triplet x64-windows libogg libvorbis curl openssl
```

You then need to configure your CMake build with the `vcpkg` toolchain (replace the path with where you installed it!):

```sh
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/c/Users/cjamc/tools/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Note: this needs to be a fresh build folder, otherwise the toolchain won't be updated. If you already have a `build` folder delete it first.

## (optional) Using Ninja and MSVC

Typically, on Windows you want to use Visual Studio as the cmake generator, but if you want to use Ninja instead here's how:

> 1. Ensure you have installed the C++/CLI tools for C++ desktop development in the Visual Studio Installer [(reference)](https://gitlab.kitware.com/cmake/cmake/-/issues/19815#note_636971)
> 
> 1. Launch `cmd.exe` and configure the environment to use the Microsoft toolchain: `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`
Change 64 to 32 if you want to build 32bit.
> 1. Configure: `cmake -S . -B build -G "Ninja Multi-Config" -DCMAKE_WIN32_EXECUTABLE=1`
> 1. Build: `cmake --build build --config Debug -t zplayer`

One reason to use Ninja is to use clang instead of MSVC (just skip the environment configuration above); or to use `ccache` for faster builds.

## Building

After configuring the build, you can either build from the command line (like `cmake --build build`), or open the Visual Studio solution file create in `build`. Either way, the binaries will be generated in the `build/{Release|Debug}` folder.

## Installing

You can either use Visual Studio to install, or use cmake from the command line.

* Visual Studio: select the `Release` configuration, then run the `package_zc` target
* cmake: run `cmake --build build --config Release --target package_zc`

The result will be in a zip file `build/Release/packages`, such as `build/Release/packages/ZQuestClassic-3.0.0-dev.zip`. Unzip it anywhere you like - it is fully portable.

# Building on macOS

## Install dependencies

```sh
brew install ninja ccache libogg libvorbis
```

## Building

Configure:

```sh
cmake -G 'Ninja Multi-Config' -B build -S .
```

Build:

```sh
cmake --build build --config Debug
```

Run:

```sh
./build/Debug/zlauncher
```

## Installing

The following command will install to `/Applications/ZQuest Classic.app`:

```sh
cmake --install build --config Release
```

You may instead wish to install it somewhere else. If so, set the `--prefix` option in the above install command.

# Building on Linux

## Install dependencies

### Linux Mint

```sh
sudo apt update
sudo apt install python3-pip ninja-build libgtk-3-dev libasound2-dev libssl-dev libcurl4-openssl-dev libstdc++-12-dev
```

### Ubuntu

```sh
sudo apt update
sudo apt install build-essential gcc-multilib g++-multilib libx11-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev libxcursor1 libasound2-dev libgtk-3-dev flex bison
```

Then before running, you need some additional packages installed:
<!-- TODO: why both? I think when using GCC we need to link with libopengl, but with clang we need libglu? -->
```sh
sudo apt install libopengl0 libglu1
```

## Building

Supported compilers: gcc 11.4+, clang 17+

Configure:

```sh
# be in the ZQuestClassic git checkout

# configure with clang:
CC=clang-17 CXX=clang++-17 cmake -G 'Ninja Multi-Config' -B build -S .
# or configure with gcc:
cmake -G 'Ninja Multi-Config' -B build -S .
```

> NOTE: skip to "Installing" if you are not developing and just want a user installation

Build:

```sh
cmake --build build --config Debug
```

Run:

```sh
./build/Debug/zlauncher
```

## Installing

The following command will install to `/opt/zquestclassic`:

```sh
cmake --install build --config Release --prefix /opt/zquestclassic
```

Open the launcher with `/opt/zquestclassic/bin/zlauncher`.

You may instead wish to install it to your home directory (such as `~/zquestclassic`).

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
> be sure to start a local webserver:
>   node scripts/webserver.mjs

# ccache

`ccache` can be used to cache build results, which makes switching between branches and re-building much faster.

> OSX: `brew install ccache`
>
> Windows: `choco install ccache`

When configuring cmake, use these flags:
> `-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache`

<!-- set CCACHE_CONFIGPATH=C:\Users\cjamc\code\ZeldaClassic\ccache.conf -->

Note: you'll need to set the env variable `CCACHE_SLOPPINESS=time_macros`.

Note: on Windows, Debug can't be cached yet. https://github.com/ccache/ccache/issues/1040

Some care is needed to ensure build outputs are deterministic. If you do a clean build (`cmake --build build --clean-first`), and then run `ccache -z` and do another clean build, then `ccache -s` will give a report of cache misses/hits. The cache should be full and there should be 0 misses. If there are misses, something in the build output is non deterministic. Read this [article](https://interrupt.memfault.com/blog/ccache-debugging) for how to debug.

Other useful links:

- https://crascit.com/2016/04/09/using-ccache-with-cmake/
