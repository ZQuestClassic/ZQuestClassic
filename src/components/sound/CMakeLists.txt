FetchContent_Declare(
	gme_external
	GIT_REPOSITORY https://github.com/libgme/game-music-emu
	GIT_TAG        05a2aa29e8eae29316804fdd28ceaa96c74a1531
	PATCH_COMMAND  git restore . && git apply ${CMAKE_SOURCE_DIR}/third_party/gme.patch
	EXCLUDE_FROM_ALL
)
set(GME_YM2612_EMU "Nuked" CACHE STRING "Which YM2612 emulator to use: \"Nuked\" (LGPLv2.1+), \"MAME\" (GPLv2+), or \"GENS\" (LGPLv2.1+)")
if(EMSCRIPTEN)
	# gme caches this option from BUILD_SHARED_LIBS at first configure, so force
	# it: a real .so in the link makes emscripten >= 6.0 imply -sMAIN_MODULE.
	set(GME_BUILD_SHARED OFF CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(gme_external)
# gme links its shared library with -Wl,-no-undefined (on Linux). A sanitized
# shared library leaves the sanitizer runtime's symbols undefined on purpose
# (they resolve against the executable at load time), so that flag fails the
# Asan link. Move it out of the plain LINK_FLAGS string into a per-config option.
get_target_property(gme_link_flags gme LINK_FLAGS)
if(gme_link_flags MATCHES "-Wl,-no-undefined")
	string(REPLACE " -Wl,-no-undefined" "" gme_link_flags "${gme_link_flags}")
	set_target_properties(gme PROPERTIES LINK_FLAGS "${gme_link_flags}")
	target_link_options(gme PRIVATE "$<$<NOT:$<CONFIG:Asan>>:-Wl,-no-undefined>")
endif()
add_library(gme_headers INTERFACE)
target_include_directories(gme_headers INTERFACE ${gme_external_SOURCE_DIR}/gme)
target_link_libraries(gme_headers INTERFACE ZLIB::ZLIB)
target_compile_definitions(gme_headers INTERFACE -DVGM_YM2612_NUKED)

set(INSTALL_TARGETS "${ZC_INSTALL_TARGETS}")
list(APPEND INSTALL_TARGETS gme)
set(ZC_INSTALL_TARGETS "${INSTALL_TARGETS}" PARENT_SCOPE)

if(MSVC AND (MSVC_VERSION GREATER 1600))
	set(ZCSOUNDLIBSEXTRA legacy_stdio_definitions)
endif()
set(ZCSOUNDSOURCES
	midi.cpp
	zcmixer.cpp
	zcmusic.cpp
)
if(EMSCRIPTEN)
	add_library(zcsound STATIC ${ZCSOUNDSOURCES})
else()
	add_library(zcsound SHARED ${ZCSOUNDSOURCES})
endif()
target_link_libraries(zcsound PUBLIC base allegro_with_legacy gme gme_headers ${ZCSOUNDLIBSEXTRA})

target_compile_definitions(zcsound PRIVATE SOUND_LIBS_BUILT_FROM_SOURCE)
