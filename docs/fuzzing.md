# Fuzzing

Instructions for how to fuzz with [AFL++](https://github.com/AFLplusplus/AFLplusplus).

Initial fuzzing (just overnight) on the quest loading code found 619 bad `.qst` inputs, leading to 18 different bug fixes.

Get and start the Docker image:

```sh
docker pull aflplusplus/aflplusplus
docker run --name zc-fuzz -ti -v /path/to/ZC/root:/src aflplusplus/aflplusplus
```

Install a bunch of stuff:
```sh
apt update
apt install -y ccache ninja-build build-essential gcc-multilib g++-multilib libx11-dev libglu1-mesa-dev libcurl4-gnutls-dev freeglut3-dev mesa-common-dev libxcursor1 libasound2-dev libgtk-3-dev flex bison elfutils xvfb
apt clean all
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
apt update
apt remove --purge --auto-remove cmake
apt install -y cmake
cmake --version
```

Build:
```sh
cd /src
CC=afl-cc CCX=afl-cc++ cmake -B build_fuzz -S . -DWANT_FUZZ=ON
cmake --build build_fuzz -t zelda
cd build_fuzz
cp -r RelWithDebInfo/* .
mkdir inputs
cp modules/classic/classic_1st.qst inputs/
export CI=1
```

Finally, run the fuzzer:

```sh
xvfb-run --auto-servernum afl-fuzz -M $HOSTNAME-main -m 512 -t 3000 -i inputs -o results -- ./zelda -load-and-quit @@
```

Bump up your Docker resources first to however much memory / cores make sense. You can run multiple fuzzers (each on a dedicated core) by running:

```sh
docker exec -it zc-fuzz bash
cd /src/build_fuzz
# Change `variant-1` to a unique name for each additional fuzzer.
xvfb-run --auto-servernum afl-fuzz -S variant-1 -m 512 -t 3000 -i inputs -o results -- ./zelda -load-and-quit @@
```

When crashes are found the input file that caused them will be saved to `build_fuzz/results`. You can debug it (outside of Docker):

```sh
./zelda -load-and-quit /path/to/ZC/root/build_fuzz/results/ebd8aa410fae-main/crashes/id:000000,sig:11,src:000000,time:14618,execs:26,op:havoc,rep:2
```

You likely want to enable ASAN to help debug:
```sh
cmake -G 'Ninja Multi-Config' -B build_asan -S . -DWANT_ASAN=ON -DWANT_FUZZ=ON
cmake --build build_asan --config Debug -t zelda
```

This script is helpful for 1) getting a stack trace of the program crashing with the bad inputs and 2) verifying changes no longer result in a crash:

```sh
python scripts/process_fuzz_results.py --build_folder build_asan/Debug --fuzz_results_folder build_fuzz
```

Each crashing input will generate a file in `.tmp/fuzz/outputs` that looks something like this:

```cpp
build_asan/Debug/zelda -load-and-quit /Users/connorclark/code/ZeldaClassic-secondary/build_fuzz/results/ebd8aa410fae-main/hangs/id:000080,src:000787+000076,time:26346082,execs:78378,op:splice,rep:11

=================================================================
==10193==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x700003bff22e at pc 0x00010c6489f5 bp 0x700003bfd790 sp 0x700003bfcf50
READ of size 31 at 0x700003bff22e thread T3
    #0 0x10c6489f4 in wrap_strlen+0x184 (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x1b9f4)
    #1 0x10311e7a7 in readheader(PACKFILE*, zquestheader*, bool, unsigned char) qst.cpp:2605
    #2 0x1031eecf4 in _lq_int(char const*, zquestheader*, miscQdata*, zctune*, bool, bool, unsigned char const*, unsigned char) qst.cpp:21630
    #3 0x1031f5d14 in loadquest(char const*, zquestheader*, miscQdata*, zctune*, bool, bool, unsigned char*, unsigned char, bool, unsigned char) qst.cpp:22516
    #4 0x103091f14 in do_load_and_quit_command(char const*) zelda.cpp:4435
    #5 0x103093103 in _al_mangled_main zelda.cpp:4449
    #6 0x10c391b7d in call_user_main osx_app_delegate.m:217
    #7 0x10c391b58 in +[AllegroAppDelegate app_main:] osx_app_delegate.m:228
    #8 0x7ff81abe6993 in __NSThread__start__+0x3f0 (Foundation:x86_64+0x58993)
    #9 0x7ff819ccb4e0 in _pthread_start+0x7c (libsystem_pthread.dylib:x86_64+0x64e0)
    #10 0x7ff819cc6f6a in thread_start+0xe (libsystem_pthread.dylib:x86_64+0x1f6a)
```

Just run the above script, inspect the stack traces saved to `.tmp/fuzz/outputs`, fix, re-build, repeat until all test inputs no longer crash.

## Future work

Fuzz more things: ZScript parser, ZASM, the JIT compiler.
