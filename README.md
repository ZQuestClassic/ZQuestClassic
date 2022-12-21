# Zelda Classic
Zelda Classic is a game and editing tool that lets you create custom Legend of Zelda quests.

Homepage:
http://zeldaclassic.com

The homepage contains information about the latest release, links to the latest binaries, and a repository of custom quests for you to enjoy. 

[Latest Downloads](http://zeldaclassic.com/downloads), [Our Discord](https://discord.gg/ddk2dk4guq)

This repository contains the Zelda Classic source code, for use by the Zelda Classic developers and advanced users wanting to port ZC to new platforms. You do not need to download or compile the source if you want to play Zelda Classic or make quests on Windows, OS X, or (Debian) Linux.

## Building the Source

Because of Zelda Classic's extended development history and dependency on legacy libraries, the build process is somewhat involved. The instructions here are not comprehensive, but are intended to help you get started.

### General Comments

**1))** Zelda Classic uses CMake to build the source. Before getting started, you will need to install CMake (version 3.5 or later.) The scripts have been tested on the following operating systems and toolchains:

- **Windows 10 with MSVC 2019**

You may need to tweak the configuration settings for other platforms.

**2))** The Zelda Classic project includes two executables and one library:
 - the zcsound library, which is used by all the other binaries;
 - zelda, the Zelda Classic player;
 - zquest, the Zelda Classic level editor;
 
The included build scripts will build all targets.

**3))** Included in this repository are pre-built libraries for many of ZC's dependencies, including several Allegro add-on libraries for handling sound. These binaries are included for the most common platforms for convenience of the developers, but if you are using a different operating system or toolchain you may need to rebuild these binaries from source. There are (currently) no automated scripts for doing this, but the source packages are included in the `/other` directory.

Zelda Classic builds Allegro 5 from source. The code base still uses Allegro 4 APIs, but uses [Allegro-Legacy](https://github.com/NewCreature/Allegro-Legacy) to translate into Allegro 5.

**4))** Zelda Classic works **only** when compiled for a 32-bit architecture (but the compiled binaries will run fine on 64-bit operating systems.) This means that to successfully compile ZC, you must set your compiler to generate 32-bit code, and you must **obtain 32-bit versions of all external libraries** (or build them yourself from source). For example, on 64-bit Ubuntu you may need to install the packages g++-multilib, libx11-dev:i386, libxext-dev:i386, libxcursor-dev:i386, libxxf86vm-dev:i386, libxpm-dev:i386, libasound2-dev:i386, and possibly others. If you are getting linker errors, check carefully for messages about binary incompatibility with the external libraries.

**5))** The ZScript parser included with ZQuest uses Flex and Bison to auto-generate its source code. You must download and install Flex and Bison to compile ZQuest with ZScript support. If you do not have these tools, compilation will fail.

For Windows user, win-flex and win-bison will work fine. Be sure to add the folder containing the binaries (e.g. `win-flex.exe`) to your PATH environment variable. You can use this to install with [choco](https://chocolatey.org/install): `choco install winflexbison3`.

If flex-bison is not found, CMake will fail to build.

**6))**

The compiled binaries require a number of resource files to be available at runtime. By default, building the project will copy those files to the correct location. They will only be copied if they do not already exist–so you can modify these files (like the various `.cfg` files) in your build directory to your liking. To completely refresh the resources file, build the `copy_resources` target (ex: `cmake --build build -t copy_resources`)

To debug any issues, open `allegro.log` after opening a binary.

### Quick-start: Windows with MSVC

Download CMake and run the CMake GUI. It will prompt your for the location of the source code, and the location in which to build the binaries. Specify the root (the folder containing this file) for the former and the `build` folder for the latter.

When it asks you for compiler settings, be sure to set MSVC 2019, with `32-bit` target.

Click "Generate." This will create a Visual Studio project file for you in the build directory. You can then open up the project file in MSVC and do editing/compilation/debugging in MSVC. You do not need to touch CMake again unless you want to change project configuration options or add/remove source files.

### Quick-start: Linux with gcc

See `docs/building.md`.

## Contributing to Zelda Classic

We encourage third-party submission of patches and new features! If you're interesting in contributing to Zelda Classic's development, please read the CONTRIBUTE file.

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


