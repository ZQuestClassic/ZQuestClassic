# XXX build chm files
# XXX build devhelp files
# XXX resources (keyboard/language) - pretty useless except on DOS
# XXX install grabber.txt?
# XXX png plugin for grabber
# XXX what is fixbundle?
# XXX applehelp bundle?
# XXX projectbuilder application template?
# XXX allegro.m4?

# Ports that used to work which aren't supported yet:
#   Haiku/BeOS port
#   djgpp port
#   QNX port
#   BCC32
#   Watcom
#   DMC
#   PSP port?

#-----------------------------------------------------------------------------#
#
# CMake setup
#

cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are:
        None Debug Release RelWithDebInfo MinSizeRel Profile."
      FORCE)
endif()
# Restrict configuration types to the selected build type.
# Note: This needs to be done before the project command
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" CACHE INTERNAL "internal")

#-----------------------------------------------------------------------------#
#
# Build options
#

# Set the project name.
project(ALLEGRO C CXX)

if(WIN32)
    if(MINGW)
        # As of CMake 2.6.2, support for building .rc files with mingw seems to
        # be broken (uses wrong command line syntax) so we have to set this
        # variable manually.
        set(CMAKE_RC_COMPILE_OBJECT
            "<CMAKE_RC_COMPILER> <FLAGS> -O coff -o <OBJECT> -i <SOURCE>")
        if(NOT CMAKE_RC_COMPILER OR "${CMAKE_RC_COMPILER}" STREQUAL "")
            string(REGEX REPLACE "gcc(.exe)?$" "windres"
                GUESS_RC_COMPILER ${CMAKE_C_COMPILER})
            find_program(CMAKE_RC_COMPILER ${GUESS_RC_COMPILER})
        endif()
    endif(MINGW)
    enable_language(RC OPTIONAL)
    message(STATUS "CMAKE_RC_COMPILER: ${CMAKE_RC_COMPILER}")
    if(NOT CMAKE_RC_COMPILER)
        message(FATAL_ERROR "Please set CMAKE_RC_COMPILER manually, e.g. *-windres.")
    endif()
endif(WIN32)

set(ALLEGRO_VERSION 4.4.2)
string(REGEX MATCH "^[0-9]+[.][0-9]+" ALLEGRO_SOVERSION ${ALLEGRO_VERSION})

# Mac OS X -compatibility_version.
# XXX doesn't seem to work
set(COMPAT_VERSION 4.4.0)

set(ALLEGRO_DLL_SHORTVER 44)

# Search in the `cmake' directory for additional CMake modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Search for C header files in these directories.
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)

# Put libraries into `lib'.
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# Lists of all the source files.
include(FileList)

# Our own CMake macros and functions.
include(Common)

#-----------------------------------------------------------------------------#
#
# Build options
#

option(SHARED "Build shared library for main Allegro library" on)
set(BUILD_SHARED_LIBS ${SHARED})    # actual CMake variable

# On some 64-bit platforms, libraries should be installed into `lib64'
# instead of `lib'.  Set this to 64 to do that.
set(LIB_SUFFIX "" CACHE STRING "Suffix for 'lib' directories, e.g. '64'")

option(MAGIC_MAIN "Enable magic main (Unix)" off)

option(WANT_FRAMEWORKS "Want frameworks on Mac OS X" off)
option(WANT_EMBED
    "Make frameworks embeddable in application bundles (Mac OS X)" on)

set(FRAMEWORK_INSTALL_PREFIX "/Library/Frameworks" CACHE STRING
    "Directory in which to install Mac OS X frameworks")

#-----------------------------------------------------------------------------#
#
# Unix platform checks
#

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(FindPkgConfig)
include(TestBigEndian)

if(UNIX)
    test_big_endian(ALLEGRO_BIG_ENDIAN)
    if(NOT ALLEGRO_BIG_ENDIAN)
        set(ALLEGRO_LITTLE_ENDIAN 1)
    endif(NOT ALLEGRO_BIG_ENDIAN)

    check_include_files(dirent.h ALLEGRO_HAVE_DIRENT_H)
    check_include_files(inttypes.h ALLEGRO_HAVE_INTTYPES_H)

    check_include_files(linux/awe_voice.h ALLEGRO_HAVE_LINUX_AWE_VOICE_H)
    check_include_files(linux/input.h ALLEGRO_HAVE_LINUX_INPUT_H)
    # On some systems including linux/joystick.h without sys/types.h results
    # in conflicting definitions of fd_set.
    check_include_files("sys/types.h;linux/joystick.h" ALLEGRO_HAVE_LINUX_JOYSTICK_H)
    check_include_files(linux/soundcard.h ALLEGRO_HAVE_LINUX_SOUNDCARD_H)
    check_include_files(machine/soundcard.h ALLEGRO_HAVE_MACHINE_SOUNDCARD_H)
    check_include_files(soundcard.h ALLEGRO_HAVE_SOUNDCARD_H)
    check_include_files(stdint.h ALLEGRO_HAVE_STDINT_H)
    check_include_files(sys/io.h ALLEGRO_HAVE_SYS_IO_H)
    check_include_files(sys/stat.h ALLEGRO_HAVE_SYS_STAT_H)
    check_include_files(sys/time.h ALLEGRO_HAVE_SYS_TIME_H)
    check_include_files(sys/soundcard.h ALLEGRO_HAVE_SYS_SOUNDCARD_H)
    check_include_files(sys/utsname.h ALLEGRO_HAVE_SYS_UTSNAME_H)

    check_function_exists(getexecname ALLEGRO_HAVE_GETEXECNAME)
    check_function_exists(memcmp ALLEGRO_HAVE_MEMCMP)
    check_function_exists(mkstemp ALLEGRO_HAVE_MKSTEMP)
    check_function_exists(mmap ALLEGRO_HAVE_MMAP)
    check_function_exists(mprotect ALLEGRO_HAVE_MPROTECT)
    check_function_exists(sched_yield ALLEGRO_HAVE_SCHED_YIELD)
    check_function_exists(stricmp ALLEGRO_HAVE_STRICMP)
    check_function_exists(strlwr ALLEGRO_HAVE_STRLWR)
    check_function_exists(strupr ALLEGRO_HAVE_STRUPR)
    check_function_exists(sysconf ALLEGRO_HAVE_SYSCONF)

    check_c_source_compiles("
        #include <sys/procfs.h>
        #include <sys/ioctl.h>
        int main(void) {
            struct prpsinfo psinfo;
            ioctl(0, PIOCPSINFO, &psinfo);
            return 0;
        }"
        ALLEGRO_HAVE_SV_PROCFS_H
        )
    check_c_source_compiles("
        #include <sys/procfs.h>
        int main(void) {
            struct prpsinfo psinfo;
            psinfo.pr_argc = 0;
            return 0;
        }"
        ALLEGRO_HAVE_PROCFS_ARGCV
        )

    check_c_source_compiles("
        #include <unistd.h>
        #include <sys/mman.h>
        int main(void) {
            void *x = MAP_FAILED;
        }"
        MAP_FAILED_DEFINED)
    if(NOT MAP_FAILED_DEFINED)
        set(MAP_FAILED "((void *) -1)")
    endif()

    check_c_source_runs("
        static int notsupported = 1;
        void test_ctor (void) __attribute__((constructor));
        void test_ctor (void) { notsupported = 0; }
        int main(void) { return (notsupported); }
        "
        ALLEGRO_USE_CONSTRUCTOR)

    find_library(RT_LIBRARY rt)
    check_c_source_compiles("
        #include <time.h>
        int main(void) {
            struct timespec new_time_ns;
            clock_gettime(CLOCK_MONOTONIC, &new_time_ns);
            return 0;
        }"
        ALLEGRO_HAVE_POSIX_MONOTONIC_CLOCK
        )

    if(MAGIC_MAIN)
        set(ALLEGRO_WITH_MAGIC_MAIN 1)
    endif(MAGIC_MAIN)

    # XXX const
    # XXX inline
    # XXX size_t
endif(UNIX)

#-----------------------------------------------------------------------------#

# Not sure if we want to support disabling these any more.
set(ALLEGRO_COLOR8 1)
set(ALLEGRO_COLOR16 1)
set(ALLEGRO_COLOR24 1)
set(ALLEGRO_COLOR32 1)

set(ALLEGRO_NO_ASM 1)
# ALLEGRO_MMX left undefined
# ALLEGRO_SSE left undefined

#-----------------------------------------------------------------------------#
#
# Platform drivers
#

set(PLATFORM_SOURCES)
set(PLATFORM_LIBS)              # always link with these libraries
set(PLATFORM_LIBS_NON_MODULES)  # only link with these if modules disabled
set(PLATFORM_LIBS_MODULES)      # only link with these if modules enabled

# -- Unix --

option(WANT_OSS "Build OSS support" on)
option(WANT_ALSA "Build ALSA support" on)
option(WANT_JACK "Build JACK support" on)
option(WANT_SGIAUDIO "Build SGI audio support" on)

if(ALLEGRO_UNIX) # not MACOSX
    find_package(Threads)
    if(NOT CMAKE_USE_PTHREADS_INIT)
        message(FATAL_ERROR "Unix port requires pthreads support.")
    endif()
    set(ALLEGRO_HAVE_LIBPTHREAD 1)
    list(APPEND PLATFORM_LIBS m ${CMAKE_THREAD_LIBS_INIT})
    list(APPEND PLATFORM_SOURCES ${ALLEGRO_SRC_UNIX_FILES})

    find_library(DLOPEN_LIBRARY dl)
    mark_as_advanced(DLOPEN_LIBRARY)
    if(DLOPEN_LIBRARY)
        list(APPEND PLATFORM_LIBS_MODULES ${DLOPEN_LIBRARY})
    endif()

    if(ALLEGRO_HAVE_POSIX_MONOTONIC_CLOCK)
        list(APPEND PLATFORM_LIBS ${RT_LIBRARY})
    endif(ALLEGRO_HAVE_POSIX_MONOTONIC_CLOCK)

    if(WANT_OSS)
        include(AllegroFindOSS)
        if(OSS_FOUND)
            set(ALLEGRO_WITH_OSSDIGI 1)
            set(ALLEGRO_WITH_OSSMIDI 1)
            include_directories(SYSTEM ${OSS_INCLUDE_DIR})
        endif(OSS_FOUND)
    endif(WANT_OSS)

    if(WANT_ALSA)
        pkg_check_modules(ALSA alsa)
        if(ALSA_FOUND)
            # ALSA 0.5 is beyond obsolete.
            set(ALLEGRO_ALSA_VERSION 9)
            set(ALLEGRO_WITH_ALSADIGI 1)
            set(ALLEGRO_WITH_ALSAMIDI 1)
            include_directories(SYSTEM ${ALSA_INCLUDE_DIRS})
            list(APPEND PLATFORM_LIBS_NON_MODULES ${ALSA_LIBRARIES})
            add_our_module(alleg-alsadigi src/unix/alsa9.c ${ALSA_LIBRARIES})
            add_our_module(alleg-alsamidi src/unix/alsamidi.c ${ALSA_LIBRARIES})
        endif(ALSA_FOUND)
    endif(WANT_ALSA)

    if(WANT_JACK)
        pkg_check_modules(JACK jack)
        if(JACK_FOUND)
            set(ALLEGRO_WITH_JACKDIGI 1)
            include_directories(SYSTEM ${JACK_INCLUDE_DIRS})
            list(APPEND PLATFORM_LIBS_NON_MODULES ${JACK_LIBRARIES})
            add_our_module(alleg-jack src/unix/jack.c ${JACK_LIBRARIES})
        endif(JACK_FOUND)
    endif(WANT_JACK)

    # This is untested.
    if(WANT_SGIAUDIO)
        check_library_exists(audio alOpenPort "" SGIAUDIO_FOUND)
        if(SGIAUDIO_FOUND)
            set(ALLEGRO_WITH_SGIALDIGI 1)
            list(APPEND PLATFORM_LIBS_NON_MODULES audio)
            add_our_module(alleg-sgialdigi src/unix/sgial.c audio)
        endif(SGIAUDIO_FOUND)
    endif(WANT_SGIAUDIO)

    # aRts is obsolete
    # esd is obsolete
endif(ALLEGRO_UNIX)

# -- X11 --

option(WANT_X11 "Want X11 support (Unix)" on)

if(ALLEGRO_UNIX AND WANT_X11)
    find_package(X11)
    if(X11_FOUND)
        set(ALLEGRO_WITH_XWINDOWS 1)
    endif()
endif()

if(ALLEGRO_WITH_XWINDOWS)
    include_directories(SYSTEM ${X11_INCLUDE_DIR})
    list(APPEND PLATFORM_SOURCES ${ALLEGRO_SRC_X_FILES})
    list(APPEND PLATFORM_LIBS ${X11_LIBRARIES})

    if(X11_XShm_FOUND)
        set(ALLEGRO_XWINDOWS_WITH_SHM 1)
        list(APPEND PLATFORM_LIBS ${X11_Xext_LIB})
    endif()

    if(X11_Xcursor_FOUND)
        set(ALLEGRO_XWINDOWS_WITH_XCURSOR 1)
        list(APPEND PLATFORM_LIBS ${X11_Xcursor_LIB})
    endif()

    if(X11_Xcursor_FOUND)
        set(ALLEGRO_XWINDOWS_WITH_XCURSOR 1)
        list(APPEND PLATFORM_LIBS ${X11_Xcursor_LIB})
    endif()

    if(X11_Xpm_FOUND)
        set(ALLEGRO_XWINDOWS_WITH_XPM 1)
        list(APPEND PLATFORM_LIBS ${X11_Xpm_LIB})
    endif()

    find_library(X11_Xxf86vm_LIB Xxf86vm ${X11_LIB_SEARCH_PATH})
    mark_as_advanced(X11_Xxf86vm_LIB)
    if(X11_xf86vmode_FOUND)
        set(ALLEGRO_XWINDOWS_WITH_XF86VIDMODE 1)
        list(APPEND PLATFORM_LIBS ${X11_Xxf86vm_LIB})
    endif()

    check_library_exists(X11 XOpenIM "${X11_LIB_SEARCH_PATH}" XIM_FOUND)
    if(XIM_FOUND)
        set(ALLEGRO_XWINDOWS_WITH_XIM 1)
    endif(XIM_FOUND)

    check_library_exists(Xxf86dga XDGAQueryExtension
        "${X11_LIB_SEARCH_PATH}" XDGA_FOUND)
    if(XDGA_FOUND)
        set(ALLEGRO_XWINDOWS_WITH_XF86DGA2 1)
        list(APPEND PLATFORM_LIBS_NON_MODULES Xxf86dga ${X11_LIBRARIES})
        add_our_module(alleg-dga2 src/x/xdga2.c Xxf86dga ${X11_LIBRARIES})
    endif()
endif(ALLEGRO_WITH_XWINDOWS)

# -- Linux console --

option(WANT_LINUX_CONSOLE "Want Linux console support (Linux)" off)
option(WANT_LINUX_VGA "Want Linux console VGA driver (Linux)" on)
option(WANT_LINUX_FBCON "Want Linux console fbcon driver (Linux)" on)
option(WANT_LINUX_SVGALIB "Want Linux console SVGAlib driver (Linux)" on)

if(WANT_LINUX_CONSOLE AND CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(ALLEGRO_LINUX 1)
    list(APPEND PLATFORM_SOURCES ${ALLEGRO_SRC_LINUX_FILES})

    if(WANT_LINUX_VGA)
        set(ALLEGRO_LINUX_VGA 1)
        add_our_module(alleg-vga src/misc/vga.c)
    endif()

    if(WANT_LINUX_FBCON)
        check_c_source_compiles("
            #include <linux/fb.h>
            int main(void) {
                int x = FB_SYNC_ON_GREEN;
                return 0;
            }"
            CAN_FBCON)
        if(CAN_FBCON)
            set(ALLEGRO_LINUX_FBCON 1)
            add_our_module(alleg-fbcon src/linux/fbcon.c)
        endif()
    endif()

    if(WANT_LINUX_SVGALIB)
        find_package(Svgalib)
        if(SVGALIB_FOUND)
            set(ALLEGRO_LINUX_SVGALIB 1)
            include_directories(SYSTEM ${SVGALIB_INCLUDE_DIR})

            set(CMAKE_REQUIRED_LIBRARIES ${SVGALIB_LIBRARY})
            check_c_source_compiles("
                #include <vga.h>
                int main(void) {
                    int x = vga_version;
                    return 0;
                }"
                ALLEGRO_LINUX_SVGALIB_HAVE_VGA_VERSION)

            list(APPEND PLATFORM_LIBS_NON_MODULES ${SVGALIB_LIBRARY})
            add_our_module(alleg-svgalib src/linux/svgalib.c
                ${SVGALIB_LIBRARY})
        endif()
    endif()
endif(WANT_LINUX_CONSOLE AND CMAKE_SYSTEM_NAME STREQUAL Linux)

#-----------------------------------------------------------------------------#
#
# Generate and install headers
#

if(UNIX)
    configure_file(
        ${CMAKE_SOURCE_DIR}/include/allegro/platform/alunixac.h.cmake
        ${CMAKE_SOURCE_DIR}/extra/alunixac.h
        )
endif(UNIX)
