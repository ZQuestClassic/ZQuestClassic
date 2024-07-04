cmake_minimum_required(VERSION 3.28)

#############################################################
# Sound Library
#############################################################

FetchContent_Declare(
        gme_external
        GIT_REPOSITORY https://github.com/libgme/game-music-emu
        GIT_TAG 6b676192d98302e698ac78fe3c00833eae6a74e5
        EXCLUDE_FROM_ALL
)
set(GME_YM2612_EMU "Nuked" CACHE STRING "Which YM2612 emulator to use: \"Nuked\" (LGPLv2.1+), \"MAME\" (GPLv2+), or \"GENS\" (LGPLv2.1+)")
option(ENABLE_UBSAN "" OFF)
FetchContent_MakeAvailable(gme_external)
add_library(gme_headers INTERFACE)
target_include_directories(gme_headers INTERFACE ${gme_external_SOURCE_DIR}/gme)
target_link_libraries(gme_headers INTERFACE ${ZLIB_LIBRARIES})
target_compile_definitions(gme_headers INTERFACE -DVGM_YM2612_NUKED)

option(BUILD_EXAMPLES "" OFF)
set(ALLEGRO_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/allegro_legacy/include)
set(ALLEGRO_LIBRARY allegro_with_legacy)
FetchContent_Declare(
        dumb_external
        GIT_REPOSITORY https://github.com/kode54/dumb.git
        GIT_TAG 396caa4d31859045ccb5ef943fd430ca4026cce8
        PATCH_COMMAND git restore . && git apply ${CMAKE_SOURCE_DIR}/third_party/aldumb.patch
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(dumb_external)
add_library(dumb_headers INTERFACE)
target_include_directories(dumb_headers INTERFACE ${dumb_external_SOURCE_DIR}/include)

set(SOUNDLIBS gme gme_headers aldmb dumb dumb_headers)

if (MSVC AND (MSVC_VERSION GREATER 1600))
    set(ZCSOUNDLIBSEXTRA legacy_stdio_definitions)
endif ()
set(ZCSOUNDSOURCES
        zcmusic.cpp
        zcmixer.cpp
)
add_library(zcsound STATIC ${ZCSOUNDSOURCES})
target_link_libraries(zcsound PUBLIC allegro_with_legacy zcbase ${SOUNDLIBS} ${ZCSOUNDLIBSEXTRA})

target_compile_definitions(zcsound PRIVATE SOUND_LIBS_BUILT_FROM_SOURCE)
