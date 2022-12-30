TODO: document how to build from scratch on windows and osx

# Building on Ubuntu

```sh
sudo apt update
sudo apt install build-essential gcc-multilib g++-multilib libx11-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev libxcursor1 libasound2-dev libgtk-3-dev flex bison

# Configure build for GCC
cmake -B build -S .
# Or: configure for clang
CC=clang CXX=clang++ cmake -B build -S .

# Build!
cmake --build build
```

Then before running, you need some additional packages installed:
<!-- TODO: why both? I think when using GCC we need to link with libopengl, but with clang we need libglu? -->
```sh
sudo apt install libopengl0 libglu1
```

# Building with Ninja and MSVC

Typically, on Windows you want to use Visual Studio as the cmake generator, but if you want to use Ninja here's how:

> 1. Ensure you have installed the C++/CLI tools for C++ desktop development in the Visual Studio Installer [(reference)](https://gitlab.kitware.com/cmake/cmake/-/issues/19815#note_636971)
> 
> 1. Launch `cmd.exe` and configure the environment to use the Microsoft toolchain: `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`
Change 64 to 32 if you want to build 32bit.
> 1. Configure: `cmake -S . -B build -G "Ninja Multi-Config" -DCMAKE_WIN32_EXECUTABLE=1`
> 1. Build: `cmake --build build --config Debug -t zelda`

One reason to use Ninja is to use clang instead of MSVC (just skip the environment configuration above);
or to use `ccache` for faster builds.

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
