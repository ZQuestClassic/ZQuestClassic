# https://github.com/NewCreature/Allegro-Legacy
# Local changes
# - CMAKE_SOURCE_DIR -> CMAKE_CURRENT_SOURCE_DIR, same for CMAKE_BINARY_DIR
# - ctrl+f local edit

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
# local edit
if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" CACHE INTERNAL "internal")
endif()

#-----------------------------------------------------------------------------#
#
# Build options
#

# Set the project name.
project(ALLEGRO_LEGACY C CXX)

set(ALLEGRO_LEGACY_VERSION 4.4.2)
string(REGEX MATCH "^[0-9]+[.][0-9]+" ALLEGRO_LEGACY_SOVERSION ${ALLEGRO_LEGACY_VERSION})

# Mac OS X -compatibility_version.
# XXX doesn't seem to work
set(COMPAT_VERSION 4.4.0)

set(ALLEGRO_LEGACY_DLL_SHORTVER 44)

# Search in the `cmake' directory for additional CMake modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Search for C header files in these directories.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
include_directories(/usr/local/include)

# Put libraries into `lib'.
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)

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

if(MSVC)
    option(STATIC_RUNTIME "Use the static MSVC runtime" on)
endif(MSVC)

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
    test_big_endian(ALLEGRO_LEGACY_BIG_ENDIAN)
    if(NOT ALLEGRO_LEGACY_BIG_ENDIAN)
        set(ALLEGRO_LEGACY_LITTLE_ENDIAN 1)
    endif(NOT ALLEGRO_LEGACY_BIG_ENDIAN)

    check_include_files(dirent.h ALLEGRO_LEGACY_HAVE_DIRENT_H)
    check_include_files(inttypes.h ALLEGRO_LEGACY_HAVE_INTTYPES_H)

    check_include_files(linux/awe_voice.h ALLEGRO_LEGACY_HAVE_LINUX_AWE_VOICE_H)
    check_include_files(linux/input.h ALLEGRO_LEGACY_HAVE_LINUX_INPUT_H)
    # On some systems including linux/joystick.h without sys/types.h results
    # in conflicting definitions of fd_set.
    check_include_files("sys/types.h;linux/joystick.h" ALLEGRO_LEGACY_HAVE_LINUX_JOYSTICK_H)
    check_include_files(linux/soundcard.h ALLEGRO_LEGACY_HAVE_LINUX_SOUNDCARD_H)
    check_include_files(machine/soundcard.h ALLEGRO_LEGACY_HAVE_MACHINE_SOUNDCARD_H)
    check_include_files(soundcard.h ALLEGRO_LEGACY_HAVE_SOUNDCARD_H)
    check_include_files(stdint.h ALLEGRO_LEGACY_HAVE_STDINT_H)
    check_include_files(sys/io.h ALLEGRO_LEGACY_HAVE_SYS_IO_H)
    check_include_files(sys/stat.h ALLEGRO_LEGACY_HAVE_SYS_STAT_H)
    check_include_files(sys/time.h ALLEGRO_LEGACY_HAVE_SYS_TIME_H)
    check_include_files(sys/soundcard.h ALLEGRO_LEGACY_HAVE_SYS_SOUNDCARD_H)
    check_include_files(sys/utsname.h ALLEGRO_LEGACY_HAVE_SYS_UTSNAME_H)

    check_function_exists(getexecname ALLEGRO_LEGACY_HAVE_GETEXECNAME)
    check_function_exists(memcmp ALLEGRO_LEGACY_HAVE_MEMCMP)
    check_function_exists(mkstemp ALLEGRO_LEGACY_HAVE_MKSTEMP)
    check_function_exists(mmap ALLEGRO_LEGACY_HAVE_MMAP)
    check_function_exists(mprotect ALLEGRO_LEGACY_HAVE_MPROTECT)
    check_function_exists(sched_yield ALLEGRO_LEGACY_HAVE_SCHED_YIELD)
    check_function_exists(stricmp ALLEGRO_LEGACY_HAVE_STRICMP)
    check_function_exists(strlwr ALLEGRO_LEGACY_HAVE_STRLWR)
    check_function_exists(strupr ALLEGRO_LEGACY_HAVE_STRUPR)
    check_function_exists(sysconf ALLEGRO_LEGACY_HAVE_SYSCONF)

    check_c_source_compiles("
        #include <sys/procfs.h>
        #include <sys/ioctl.h>
        int main(void) {
            struct prpsinfo psinfo;
            ioctl(0, PIOCPSINFO, &psinfo);
            return 0;
        }"
        ALLEGRO_LEGACY_HAVE_SV_PROCFS_H
        )
    check_c_source_compiles("
        #include <sys/procfs.h>
        int main(void) {
            struct prpsinfo psinfo;
            psinfo.pr_argc = 0;
            return 0;
        }"
        ALLEGRO_LEGACY_HAVE_PROCFS_ARGCV
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
        ALLEGRO_LEGACY_USE_CONSTRUCTOR)

    find_library(RT_LIBRARY rt)
    check_c_source_compiles("
        #include <time.h>
        int main(void) {
            struct timespec new_time_ns;
            clock_gettime(CLOCK_MONOTONIC, &new_time_ns);
            return 0;
        }"
        ALLEGRO_LEGACY_HAVE_POSIX_MONOTONIC_CLOCK
        )

    if(MAGIC_MAIN)
        set(ALLEGRO_LEGACY_WITH_MAGIC_MAIN 1)
    endif(MAGIC_MAIN)

    # XXX const
    # XXX inline
    # XXX size_t
endif(UNIX)

#-----------------------------------------------------------------------------#
#
# Compiler and platform setup
#

option(STRICT_WARN "Halt at warnings" off)

set(INSTALL_PREFIX "")

if(CMAKE_COMPILER_IS_GNUCC)
    set(COMPILER_GCC 1)
    set(ALLEGRO_LEGACY_GCC 1)
    set(WFLAGS "-W -Wall -Wno-unused-parameter")
    set(WFLAGS_C_ONLY "-Wdeclaration-after-statement")
    if(STRICT_WARN)
        set(WFLAGS "${WFLAGS} -Werror -Wpointer-arith")
        set(WFLAGS_C_ONLY "${WFLAGS_C_ONLY} -Wmissing-declarations")
        set(WFLAGS_C_ONLY "${WFLAGS_C_ONLY} -Wstrict-prototypes")
    endif(STRICT_WARN)
endif(CMAKE_COMPILER_IS_GNUCC)

if(MINGW)
    set(ALLEGRO_LEGACY_MINGW32 1)

    # Guess MINGDIR from the value of CMAKE_C_COMPILER if it's not set.
    if("$ENV{MINGDIR}" STREQUAL "")
        string(REGEX REPLACE "/bin/[^/]*$" "" MINGDIR "${CMAKE_C_COMPILER}")
        message(STATUS "Guessed MinGW directory: ${MINGDIR}")
    else("$ENV{MINGDIR}" STREQUAL "")
        file(TO_CMAKE_PATH "$ENV{MINGDIR}" MINGDIR)
        message(STATUS "Using MINGDIR: ${MINGDIR}")
    endif("$ENV{MINGDIR}" STREQUAL "")

    # Search in MINGDIR for headers and libraries.
    set(CMAKE_PREFIX_PATH "${MINGDIR}")

    # Install to MINGDIR
    if(INSTALL_PREFIX STREQUAL "")
        set(CMAKE_INSTALL_PREFIX ${MINGDIR})
    else(INSTALL_PREFIX STREQUAL "")
        set(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX})
    endif(INSTALL_PREFIX STREQUAL "")

    message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endif(MINGW)

if(MSVC)
    set(COMPILER_MSVC 1)
    set(ALLEGRO_LEGACY_MSVC 1)
    set(WFLAGS "/W3 -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE")
endif(MSVC)

if(WIN32)
    set(ALLEGRO_LEGACY_WINDOWS 1)
endif()

if(APPLE)
    set(ALLEGRO_LEGACY_MACOSX 1)
    set(ALLEGRO_LEGACY_DARWIN 1)

    # Hack to deal with Mac OS X 10.6.  NSQuickDrawView is not defined by
    # NSQuickDrawView.h when compiling in 64-bit mode, and 64-bit mode is the
    # default when compiling on Snow Leopard.
    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL i386)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch i386")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch i386")
    endif()

    # The Mac port simply uses too many deprecated things.
    if(COMPILER_GCC)
        set(WFLAGS "${WFLAGS} -Wno-deprecated-declarations")
    endif(COMPILER_GCC)
endif(APPLE)

if(BEOS)
    if(CMAKE_SYSTEM_NAME STREQUAL Haiku)
        set(ALLEGRO_LEGACY_HAIKU 1)
    else(CMAKE_SYSTEM_NAME STREQUAL Haiku)
        set(ALLEGRO_LEGACY_BEOS 1)
    endif(CMAKE_SYSTEM_NAME STREQUAL Haiku)
    set(WFLAGS "-W -Wall -Wno-unused -Wno-multichar")
    set(WFLAGS_C_ONLY "")
    set(WFLAGS_CXX_ONLY "-Wno-ctor-dtor-privacy")
endif(BEOS)

if(UNIX AND NOT APPLE AND NOT BEOS)
    set(ALLEGRO_LEGACY_UNIX 1)
endif()

if(PSP)
    set(ALLEGRO_LEGACY_PSP 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -G0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -G0 -fno-exceptions -fno-rtti")
    set(CMAKE_INSTALL_PREFIX ${PSP_PREFIX})
endif(PSP)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WFLAGS} ${WFLAGS_C_ONLY}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WFLAGS} ${WFLAGS_CXX_ONLY}")

if(NOT BUILD_SHARED_LIBS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DALLEGRO_LEGACY_STATICLINK")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DALLEGRO_LEGACY_STATICLINK")
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUGMODE=1")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUGMODE=1")

list(APPEND CMAKE_BUILD_TYPES Profile)
mark_as_advanced(
    CMAKE_C_FLAGS_PROFILE
    CMAKE_CXX_FLAGS_PROFILE
    CMAKE_EXE_LINKER_FLAGS_PROFILE
    )
if(COMPILER_GCC)
    set(CMAKE_C_FLAGS_PROFILE "-pg"
        CACHE STRING "profiling flags")
    set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE}"
        CACHE STRING "profiling flags")
    set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-pg"
        CACHE STRING "profiling flags")
    set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "-pg"
        CACHE STRING "profiling flags")
endif(COMPILER_GCC)
if(COMPILER_MSVC)
    set(CMAKE_C_FLAGS_PROFILE "-Gd -Ox"
        CACHE STRING "profiling flags")
    set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE}"
        CACHE STRING "profiling flags")
    set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-profile"
        CACHE STRING "profiling flags")


    if(STATIC_RUNTIME)
        set(CompilerFlags
            CMAKE_CXX_FLAGS
	        CMAKE_CXX_FLAGS_DEBUG
	        CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
        	CMAKE_C_FLAGS
	        CMAKE_C_FLAGS_DEBUG
        	CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_RELWITHDEBINFO
        	)
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
            string(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
        endforeach()
    endif(STATIC_RUNTIME)
endif(COMPILER_MSVC)

#-----------------------------------------------------------------------------#

# Not sure if we want to support disabling these any more.
set(ALLEGRO_LEGACY_COLOR8 1)
set(ALLEGRO_LEGACY_COLOR16 1)
set(ALLEGRO_LEGACY_COLOR24 1)
set(ALLEGRO_LEGACY_COLOR32 1)

set(ALLEGRO_LEGACY_NO_ASM 1)
# ALLEGRO_LEGACY_MMX left undefined
# ALLEGRO_LEGACY_SSE left undefined

# local edit
# find_library(ALLEGRO_5_LIBRARY NAMES allegro allegro-static REQUIRED)
# find_library(ALLEGRO_5_AUDIO_LIBRARY NAMES allegro_audio allegro_audio-static)
# find_library(ALLEGRO_5_MAIN_LIBRARY NAMES allegro_main allegro_main-static)
set(ALLEGRO_5_LIBRARY allegro)
set(ALLEGRO_5_AUDIO_LIBRARY allegro_audio)
set(ALLEGRO_5_MAIN_LIBRARY allegro_main)

#-----------------------------------------------------------------------------#
#
# Unix modules
#

option(WANT_MODULES "Use modules for drivers (Unix)" on)

if(WANT_MODULES AND ALLEGRO_LEGACY_UNIX)
    set(ALLEGRO_LEGACY_WITH_MODULES 1)
    set(ALLEGRO_LEGACY_MODULES_PATH
        ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/allegro/${ALLEGRO_LEGACY_VERSION})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modules.lst
            DESTINATION "${ALLEGRO_LEGACY_MODULES_PATH}"
            )
endif(WANT_MODULES AND ALLEGRO_LEGACY_UNIX)

function(add_our_module nm files)
    if(ALLEGRO_LEGACY_WITH_MODULES)
        add_library(${nm} MODULE ${files})
        set_target_properties(${nm}
            PROPERTIES
            PREFIX ""
            COMPILE_FLAGS -DALLEGRO_LEGACY_MODULE
            )
        target_link_libraries(${nm} ${ARGN})
        install(TARGETS ${nm}
                DESTINATION "${ALLEGRO_LEGACY_MODULES_PATH}"
                )
    endif(ALLEGRO_LEGACY_WITH_MODULES)
endfunction(add_our_module)

#-----------------------------------------------------------------------------#
#
# Platform drivers
#

set(PLATFORM_SOURCES)
set(PLATFORM_LIBS)              # always link with these libraries
set(PLATFORM_LIBS_NON_MODULES)  # only link with these if modules disabled
set(PLATFORM_LIBS_MODULES)      # only link with these if modules enabled

list(APPEND PLATFORM_SOURCES ${ALLEGRO_LEGACY_SRC_A5_FILES})

list(APPEND PLATFORM_SOURCES ${ALLEGRO_LEGACY_SRC_MIDIA5_FILES})
if(ALLEGRO_LEGACY_MACOSX)
    list(APPEND PLATFORM_SOURCES ${ALLEGRO_LEGACY_SRC_MIDIA5_MACOS_FILES})
endif(ALLEGRO_LEGACY_MACOSX)
if(ALLEGRO_LEGACY_UNIX)
    # local edit
    if (ALLEGRO_SDL)
        list(APPEND PLATFORM_SOURCES ${ALLEGRO_LEGACY_SRC_MIDIA5_SDL_FILES})
    else()
        list(APPEND PLATFORM_SOURCES ${ALLEGRO_LEGACY_SRC_MIDIA5_UNIX_FILES})
    endif()
endif(ALLEGRO_LEGACY_UNIX)
if(ALLEGRO_LEGACY_WINDOWS)
    list(APPEND PLATFORM_SOURCES ${ALLEGRO_LEGACY_SRC_MIDIA5_WINDOWS_FILES})
endif(ALLEGRO_LEGACY_WINDOWS)
list(APPEND PLATFORM_LIBS ${ALLEGRO_5_LIBRARY} ${ALLEGRO_5_AUDIO_LIBRARY})
if(APPLE)
	list(APPEND PLATFORM_LIBS ${ALLEGRO_5_MAIN_LIBRARY})
	list(APPEND PLATFORM_LIBS "-framework OpenGL -framework OpenAL -framework AudioToolbox -framework Cocoa -framework IOKit -framework CoreVideo")
endif()

#-----------------------------------------------------------------------------#
#
# Generate and install headers
#

set(ALLEGRO_LEGACY_PUBLIC_HEADERS)

macro(add_headers location)
    list(APPEND ALLEGRO_LEGACY_PUBLIC_HEADERS ${ARGN})
    # We reuse MACOSX_PACKAGE_LOCATION on non-Mac platforms as well.
    set_source_files_properties(${ARGN}
	PROPERTIES
	MACOSX_PACKAGE_LOCATION Headers/${location}
	)
endmacro(add_headers)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/allegro/platform/alplatf.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/allegro/platform/alplatf.h
    @ONLY
    )
add_headers(allegro/platform
    ${CMAKE_CURRENT_BINARY_DIR}/include/allegro/platform/alplatf.h
    )

if(UNIX)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/allegro/platform/alunixac.h.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/include/allegro/platform/alunixac.h
        )
    add_headers(allegro/platform
        ${CMAKE_CURRENT_BINARY_DIR}/include/allegro/platform/alunixac.h
	)
endif(UNIX)

add_headers(""               include/allegro.h)
add_headers(allegro          ${ALLEGRO_LEGACY_INCLUDE_ALLEGRO_LEGACY_FILES})
add_headers(allegro/inline   ${ALLEGRO_LEGACY_INCLUDE_ALLEGRO_LEGACY_INLINE_FILES})
add_headers(allegro/internal ${ALLEGRO_LEGACY_INCLUDE_ALLEGRO_LEGACY_INTERNAL_FILES})
add_headers(allegro/platform ${ALLEGRO_LEGACY_INCLUDE_ALLEGRO_LEGACY_PLATFORM_FILES})

# local edit
if(ALLEGRO_LEGACY_CLOSE_THREADS)
    add_definitions(-DALLEGRO_LEGACY_CLOSE_THREADS)
endif()

# Platform-specific top-level headers.
if(ALLEGRO_LEGACY_LINUX)
    add_headers("" include/linalleg.h)
endif(ALLEGRO_LEGACY_LINUX)
if(APPLE)
    add_headers("" include/osxalleg.h)
endif(APPLE)
if(WIN32)
    add_headers("" include/winalleg.h)
endif(WIN32)
if(ALLEGRO_LEGACY_WITH_XWINDOWS)
    add_headers("" include/xalleg.h)
endif(ALLEGRO_LEGACY_WITH_XWINDOWS)
if(BEOS)
    add_headers("" include/bealleg.h)
endif(BEOS)
add_headers("" include/a5alleg.h)

#-----------------------------------------------------------------------------#
#
# Main library
#

# On Unix/Mac we don't ever use a -static postfix.
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(BUILD_TYPE_SUFFIX -debug)
elseif(CMAKE_BUILD_TYPE STREQUAL Profile)
    set(BUILD_TYPE_SUFFIX -profile)
else()
    set(BUILD_TYPE_SUFFIX "")
endif()

if(ALLEGRO_LEGACY_MODULES_PATH)
    set(ALLEGRO_LEGACY_MODULES_PATH_FLAG
        "-DALLEGRO_LEGACY_MODULES_PATH='\"${ALLEGRO_LEGACY_MODULES_PATH}\"'")
endif(ALLEGRO_LEGACY_MODULES_PATH)

if(APPLE)
    # CMake 2.6.2 onwards passes these flags automatically.
    if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} STREQUAL 2.6 AND
	    ${CMAKE_PATCH_VERSION} LESS 2)
	set(ALLEGRO_LEGACY_LINK_FLAGS
	    "-compatibility_version ${COMPAT_VERSION} -current_version ${ALLEGRO_LEGACY_VERSION}")
    endif()
endif(APPLE)

add_our_library(allegro-legacy
    ${ALLEGRO_LEGACY_SRC_FILES}
    ${ALLEGRO_LEGACY_SRC_C_FILES}
    ${PLATFORM_SOURCES}
    ${ALLEGRO_LEGACY_PUBLIC_HEADERS})

if(WIN32 AND SHARED)
    set(allegro_OUTPUT_NAME alleg${ALLEGRO_LEGACY_DLL_SHORTVER})
else()
    set(allegro_OUTPUT_NAME alleg)
endif()

set_target_properties(allegro-legacy
    PROPERTIES
    COMPILE_FLAGS "-DALLEGRO_LEGACY_SRC ${ALLEGRO_LEGACY_MODULES_PATH_FLAG}"
    LINK_FLAGS "${ALLEGRO_LEGACY_LINK_FLAGS}"
    OUTPUT_NAME ${allegro_OUTPUT_NAME}
    VERSION ${ALLEGRO_LEGACY_VERSION}
    SOVERSION ${ALLEGRO_LEGACY_SOVERSION}
    )

# local edit
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(allegro-legacy PRIVATE $<$<CONFIG:Debug>:/O2>)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(allegro-legacy PRIVATE $<$<CONFIG:Debug>:-O3>)
endif()

STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

# local edit
if(APPLE)
    set_target_properties(allegro-legacy PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    set_target_properties(allegro-legacy PROPERTIES INSTALL_NAME_DIR "@rpath")
endif()

if(WIN32)
    if(SHARED)
        # Make mingw not add lib prefix to DLL, to match MSVC.
        set_target_properties(allegro-legacy
            PROPERTIES
            PREFIX ""
            )
    else(SHARED)
        # Add a -static postfix on the main library for Windows.
        # In earlier versions of Allegro this was "_s".
        set_target_properties(allegro-legacy
            PROPERTIES
            POSTFIX "-static"
            DEBUG_POSTFIX "-debug-static"
            PROFILE_POSTFIX "-profile-static"
            )
    endif(SHARED)
endif(WIN32)

set_our_framework_properties(allegro-legacy Allegro)

set(link_with ${PLATFORM_LIBS})
if(ALLEGRO_LEGACY_WITH_MODULES)
    list(APPEND link_with ${PLATFORM_LIBS_MODULES})
else()
    list(APPEND link_with ${PLATFORM_LIBS_NON_MODULES})
endif()
target_link_libraries(allegro-legacy ${link_with})

sanitize_cmake_link_flags(static_link_with ${link_with})
set_target_properties(allegro-legacy
    PROPERTIES
    static_link_with "${static_link_with}"
    )

# install_our_library(allegro)
# if(NOT WANT_FRAMEWORKS)
#     install_our_headers(${ALLEGRO_LEGACY_PUBLIC_HEADERS})
# endif(NOT WANT_FRAMEWORKS)

if(PSP)
    add_our_library(alleg-main STATIC
	src/psp/pmain.c
	)
    set_target_properties(alleg-main
	PROPERTIES
	OUTPUT_NAME alleg-main${BUILD_TYPE_SUFFIX}
	COMPILE_FLAGS -DALLEGRO_LEGACY_SRC
	)
    target_link_libraries(alleg-main pspuser pspkernel)
    target_link_libraries(allegro-legacy alleg-main)
    install_our_library(alleg-main)
endif(PSP)

#-----------------------------------------------------------------------------#
#
# allegro-config script
#

if(UNIX) # including MACOSX
    set(prefix "${CMAKE_INSTALL_PREFIX}")
    set(INCLUDE_PREFIX "${CMAKE_INSTALL_PREFIX}") # not sure what this is
    if(BUILD_SHARED_LIBS)
        set(LINK_WITH_STATIC_LIBS no)
    else()
        set(LINK_WITH_STATIC_LIBS yes)
    endif()
    set(LIB_TO_LINK alleg${BUILD_TYPE_SUFFIX})
    set(LDFLAGS "")
    sanitize_cmake_link_flags(LIBS
        ${PLATFORM_LIBS}
        ${PLATFORM_LIBS_NON_MODULES}
        ${PLATFORM_LIBS_MODULES}
        )

    if(APPLE)
        # XXX untested
        set(ACCEPT_FRAMEWORKS yes)
        set(FRAMEWORKS "-framework OpenGL -framework OpenAL -framework Cocoa -framework IOKit -framework AudioToolbox -framework CoreVideo")
    else()
        set(ACCEPT_FRAMEWORKS no)
        set(FRAMEWORKS "")
    endif()

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/misc/allegro-config.in
        ${CMAKE_CURRENT_BINARY_DIR}/allegro-config
        @ONLY
        )

    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/allegro-config
            DESTINATION bin
            )
endif(UNIX)

#-----------------------------------------------------------------------------#
# vim: set sts=4 sw=4 et:
