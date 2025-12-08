if(APPLE)
	# needed for https://gitlab.kitware.com/cmake/cmake/-/commit/7b99c42e57b11afe818827ac537ca020c020addd
	cmake_minimum_required(VERSION 3.29)
else()
	cmake_minimum_required(VERSION 3.27)
endif()
set(CMAKE_CONFIGURATION_TYPES Asan Debug Release RelWithDebInfo Coverage CACHE STRING INTERNAL FORCE)
project (ZeldaClassic HOMEPAGE_URL "https://www.zquestclassic.com")

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FetchContent)
include(CodeCoverage)

option(COPY_RESOURCES "Copy runtime resources to build folder, but only if they do not exist yet" ON)

set(USE_PCH FALSE CACHE BOOL "Use precompiled headers")
set(UNITY_BUILD FALSE CACHE BOOL "Unity build")
set(WANT_SENTRY FALSE CACHE BOOL "Include Sentry for crash dump handling")
set(WANT_WEBSOCKETS TRUE CACHE BOOL "Support websockets in zscript")
set(WANT_NFD TRUE CACHE BOOL "Include nativefiledialog")
set(WANT_FUZZ FALSE CACHE BOOL "Set FUZZ CXX definition")
set(WANT_GIT_HOOKS TRUE CACHE BOOL "Install git hooks")
set(WANT_ZC_TESTS TRUE CACHE BOOL "Build tests")
set(WANT_VENDORED_DUMB TRUE CACHE BOOL "Build the vendored library at third_party/dumb")
set(WANT_VENDORED_LIBJPEG_TURBO TRUE CACHE BOOL "Build the vendored library at third_party/libjpeg_turbo")
set(JIT_BACKEND "default" CACHE STRING "One of: default, x64, wasm, none")
set(ZC_VERSION "" CACHE STRING "Version string")
set(ZC_OFFICIAL "" CACHE BOOL "Official build")
set(WANT_ERROR_CHECKING TRUE CACHE BOOL "")
set(RELEASE_PLATFORM "" CACHE STRING "Release platform. Ex: mac, linux, windows-x86, windows-x64")
set(RELEASE_CHANNEL "" CACHE STRING "Release channel. Used by updater to find new releases")
set(REPO "" CACHE STRING "GitHub org/repo")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(ZC_OFFICIAL)
	if(NOT ZC_VERSION)
		message(FATAL_ERROR "Missing expected ZC_VERSION")
	endif()
	if(NOT RELEASE_CHANNEL)
		message(FATAL_ERROR "Missing expected RELEASE_CHANNEL")
	endif()
	if(NOT REPO)
		message(FATAL_ERROR "Missing expected REPO")
	endif()
endif()

find_program(PYTHON python3 python REQUIRED)

if(MSVC)
	set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} /fsanitize=address /Zi" CACHE STRING "" FORCE)
	set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS_DEBUG} /fsanitize=address /Zi" CACHE STRING "" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_ASAN "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /fsanitize=address /Zi" CACHE STRING "" FORCE)
else()
	set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer" CACHE STRING "" FORCE)
	set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer" CACHE STRING "" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_ASAN "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address" CACHE STRING "" FORCE)
endif()

if(WANT_GIT_HOOKS)
	if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
		if(UNIX)
			configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/commit-msg.in" "${CMAKE_SOURCE_DIR}/.git/commit-msg")
			configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/prepare-commit-msg.in" "${CMAKE_SOURCE_DIR}/.git/prepare-commit-msg")
			file(
				COPY "${CMAKE_SOURCE_DIR}/.git/commit-msg"
				DESTINATION "${CMAKE_SOURCE_DIR}/.git/hooks"
				FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
			)
			file(
				COPY "${CMAKE_SOURCE_DIR}/.git/prepare-commit-msg"
				DESTINATION "${CMAKE_SOURCE_DIR}/.git/hooks"
				FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
			)
		else()
			configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/commit-msg.in" "${CMAKE_SOURCE_DIR}/.git/hooks/commit-msg")
			configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/prepare-commit-msg.in" "${CMAKE_SOURCE_DIR}/.git/hooks/prepare-commit-msg")
		endif()
	endif()
endif()

# This allows outputs to be run where they are by putting their libaries in
# the same folder. 
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Hide various test-related targets created by GME cmake so they
# don't clutter up the Solution file. https://gitlab.kitware.com/cmake/cmake/-/issues/21730
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

set(BUILD_TESTING OFF CACHE BOOL "")

if(LINUX)
	# https://stackoverflow.com/a/71321163/2788187
	SET(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
endif()

if(APPLE)
	find_program(DSYMUTIL_PROGRAM dsymutil)
	if (DSYMUTIL_PROGRAM)
		foreach(lang C CXX)
			foreach(var LINK_EXECUTABLE CREATE_SHARED_LIBRARY)
				set(CMAKE_${lang}_${var} "${CMAKE_${lang}_${var}}" "${DSYMUTIL_PROGRAM} <TARGET>")
			endforeach()
		endforeach()
	endif()
endif()

if(LINUX)
	find_program(EU_STRIP_PROGRAM eu-strip)
	if (EU_STRIP_PROGRAM)
		foreach(lang C CXX)
			foreach(var LINK_EXECUTABLE CREATE_SHARED_LIBRARY)
				set(CMAKE_${lang}_${var} "${CMAKE_${lang}_${var}}" "${EU_STRIP_PROGRAM} --strip-debug -f <TARGET>.debug <TARGET>")
			endforeach()
		endforeach()
	endif()
endif()

function(enable_unity_build UB_SUFFIX SOURCE_VARIABLE_NAME)
  set(files ${${SOURCE_VARIABLE_NAME}})
  # Generate a unique filename for the unity build translation unit
  set(unit_build_file ${CMAKE_CURRENT_BINARY_DIR}/UB_${UB_SUFFIX}.cpp)
  # Exclude all translation units from compilation
  set_source_files_properties(${files} PROPERTIES HEADER_FILE_ONLY true)
  # Open the ub file
  FILE(WRITE ${unit_build_file} "// Unity Build generated by CMake\n")
  # Add include statement for each translation unit
  foreach(source_file ${files} )
	FILE( APPEND ${unit_build_file} "#include <${CMAKE_CURRENT_SOURCE_DIR}/${source_file}>\n")
  endforeach(source_file)
  # Complement list of translation units with the name of ub
  set(${SOURCE_VARIABLE_NAME} ${${SOURCE_VARIABLE_NAME}} ${unit_build_file} PARENT_SCOPE)
endfunction(enable_unity_build)

set(ZC_INSTALL_TARGETS "")

#############################################################
# deps
#############################################################

set(BUILD_SHARED_LIBS ON)

add_subdirectory(third_party)

# Local windows development is for some reason more finnicky to setup with the very latest
# zlib/png. For now, use older versions locally.
if(MSVC AND NOT (DEFINED ENV{CI} OR ZC_OFFICIAL) AND NOT ZLIB_LIBRARIES)
	FetchContent_Declare(
		zlib
		GIT_REPOSITORY
			https://github.com/Mizux/zlib.git
		GIT_TAG
			19d7d6b32ee3af4ed4bdec415f42f518c93b5025
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(zlib)

	if(NOT ZLIB_LIBRARIES)
		set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
		set(ZLIB_LIBRARY ZLIB::ZLIB)
		set(ZLIB_LIBRARIES ZLIB::ZLIB)
	endif()

	FetchContent_Declare(
		png
		GIT_REPOSITORY
			https://github.com/glennrp/libpng.git
		GIT_TAG
			v1.6.38
		EXCLUDE_FROM_ALL
	)
	set(PNG_BUILD_ZLIB ON CACHE BOOL "")
	set(PNG_STATIC OFF CACHE BOOL "")
	set(PNG_TESTS OFF CACHE BOOL "")
	FetchContent_MakeAvailable(png)

	if(NOT PNG_LIBRARIES)
		set(PNG_LIBRARIES png)
		set(PNG_INCLUDE_DIRS "${png_SOURCE_DIR}" "${png_BINARY_DIR}")
		set(PNG_INCLUDE_DIR "${png_SOURCE_DIR}" "${png_BINARY_DIR}")
	endif()

	list(APPEND ZC_INSTALL_TARGETS ${PNG_LIBRARIES})
elseif(NOT EMSCRIPTEN)
	FetchContent_Declare(
		zlib
		GIT_REPOSITORY
			https://github.com/madler/zlib.git
		GIT_TAG
			v1.3.1
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(zlib)

	FetchContent_Declare(
		png
		GIT_REPOSITORY
			https://github.com/pnggroup/libpng
		GIT_TAG
			v1.6.47
		EXCLUDE_FROM_ALL
	)
	set(ZLIB_ROOT "${zlib_SOURCE_DIR}" CACHE INTERNAL "Root for the zlib library, used by libpng")
	set(PNG_STATIC OFF CACHE INTERNAL BOOL "")
	set(PNG_TESTS OFF CACHE INTERNAL BOOL "")
	FetchContent_MakeAvailable(png)

	if(NOT PNG_LIBRARIES)
		set(PNG_LIBRARIES png_shared)
		set(PNG_INCLUDE_DIRS "${png_SOURCE_DIR}" "${png_BINARY_DIR}")
		set(PNG_INCLUDE_DIR "${png_SOURCE_DIR}" "${png_BINARY_DIR}")
	endif()

	list(APPEND ZC_INSTALL_TARGETS ${ZLIB_LIBRARIES} ${PNG_LIBRARIES})
endif()

FetchContent_Declare(json
	URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
	EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(json)

if (NOT EMSCRIPTEN)
	FetchContent_Declare(
		stduuid
		GIT_REPOSITORY
			https://github.com/mariusbancila/stduuid
		GIT_TAG
			3afe7193facd5d674de709fccc44d5055e144d7a
		PATCH_COMMAND  git restore . && git apply ${CMAKE_SOURCE_DIR}/third_party/stduuid.patch
		EXCLUDE_FROM_ALL
	)
	set(UUID_SYSTEM_GENERATOR ON CACHE INTERNAL "")
	FetchContent_MakeAvailable(stduuid)
endif()

set(USE_NFD OFF)
if (WANT_NFD)
	if (EMSCRIPTEN)
		message(WARNING "WANT_NFD is set, but can't support web, so ignoring")
	else()
		FetchContent_Declare(
			nativefiledialog
			GIT_REPOSITORY
				https://github.com/btzy/nativefiledialog-extended
			GIT_TAG
				17b6e8ce219c0677f94b63636abb9296b28841ca
			EXCLUDE_FROM_ALL
		)
		set(BUILD_SHARED_LIBS_PREV ${BUILD_SHARED_LIBS})
		set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
		set(NFD_INSTALL OFF CACHE BOOL "" FORCE)
		FetchContent_MakeAvailable(nativefiledialog) # exposed as nfd
		set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_PREV} CACHE BOOL "" FORCE)
		set(USE_NFD ON)
	endif()
endif()

find_package(CURL)

#############################################################
# Allegro
#############################################################

# SDL WIP:
#    cmake .. -DALLEGRO_SDL=on -DWANT_NATIVE_DIALOG=off
# https://github.com/liballeg/allegro5/issues/1335

if(APPLE)
	# OSX 12
	add_definitions(-DMAC_OS_X_VERSION_MIN_REQUIRED=120000)
else()
	add_definitions(-DALLEGRO_NO_MAGIC_MAIN)
endif()
add_definitions(-DALLEGRO_KCM_AUDIO_SRC)
# Needed to build on Mac, otherwise get this error:
#    aintern_bitmap.h:154:33: error: unknown type name 'ALLEGRO_BITMAP_WRAP'
add_definitions(-DALLEGRO_UNSTABLE)
add_compile_definitions("$<$<CONFIG:DEBUG>:DEBUGMODE>")
add_compile_definitions("$<$<CONFIG:ASAN>:DEBUGMODE>")
option(SHARED "" ON)

set(MINIMP3_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/minimp3)

FetchContent_Declare(
  allegro5
  GIT_REPOSITORY https://github.com/connorjclark/allegro5.git
  GIT_TAG        97c84670907b2547f742924d571f2c617d557ed0
  EXCLUDE_FROM_ALL
)
set(WANT_TESTS "" OFF CACHE BOOL "" FORCE)
set(WANT_EXAMPLES "" OFF CACHE BOOL "" FORCE)
set(WANT_DEMO "" OFF CACHE BOOL "" FORCE)
set(WANT_DOCS "" OFF CACHE BOOL "" FORCE)
set(WANT_IMAGE_WEBP "" OFF CACHE BOOL "" FORCE)
if(WANT_VENDORED_LIBJPEG_TURBO)
	set(WANT_NATIVE_IMAGE_LOADER OFF CACHE BOOL "" FORCE)
else()
	set(WANT_NATIVE_IMAGE_LOADER ON CACHE BOOL "" FORCE)
endif()
set(WANT_OPUS "" OFF CACHE BOOL "" FORCE)
set(WANT_DUMB "" ON CACHE BOOL "" FORCE)
set(WANT_FLAC "" OFF CACHE BOOL "" FORCE)
set(WANT_VIDEO "" OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(allegro5)
# Allegro 5 CMakeLists sets CMAKE_CONFIGURATION_TYPES in a way that doesn't work for us,
# so set it back to our value.
set(CMAKE_CONFIGURATION_TYPES Asan Debug Release RelWithDebInfo Coverage CACHE STRING INTERNAL FORCE)

# These include files are typically copied into the correct places via allegro's install
# target, but we do it manually because we are compiling from source.
file(COPY ${allegro5_SOURCE_DIR}/addons/font/allegro5/allegro_font.h
	DESTINATION ${allegro5_SOURCE_DIR}/include/allegro5
)
file(COPY ${allegro5_SOURCE_DIR}/addons/primitives/allegro5/allegro_primitives.h
	DESTINATION ${allegro5_SOURCE_DIR}/include/allegro5
)
file(COPY ${allegro5_SOURCE_DIR}/addons/audio/allegro5/allegro_audio.h
	DESTINATION ${allegro5_SOURCE_DIR}/include/allegro5
)
file(COPY ${allegro5_SOURCE_DIR}/addons/audio/allegro5/internal/aintern_audio.h
	DESTINATION ${allegro5_SOURCE_DIR}/include/allegro5/internal
)
file(COPY ${allegro5_SOURCE_DIR}/addons/image/allegro5/allegro_image.h
	DESTINATION ${allegro5_SOURCE_DIR}/include/allegro5
)
file(COPY ${allegro5_SOURCE_DIR}/addons/acodec/allegro5/allegro_acodec.h
	DESTINATION ${allegro5_SOURCE_DIR}/include/allegro5
)
file(COPY ${allegro5_SOURCE_DIR}/addons/native_dialog/allegro5/allegro_native_dialog.h
	DESTINATION ${allegro5_SOURCE_DIR}/include/allegro5
)

# TODO: figure out how to not use global includes for this
include_directories(${allegro5_SOURCE_DIR}/include)
include_directories(${allegro5_BINARY_DIR}/include)
# Some performance problems with allegro legacy threaded event queues encouraged
# NOT closing them on shutdown. This seemed to work ok on Windows and Mac, but on
# Linux it totally prevents the program from closing.
# TODO: investigate this further - can performance problems with threaded event queues
# be fixed?
if(LINUX AND NOT EMSCRIPTEN)
	set(ALLEGRO_LEGACY_CLOSE_THREADS 1)
endif()
add_subdirectory(third_party/allegro_legacy EXCLUDE_FROM_ALL)

add_library(allegro_with_legacy INTERFACE)
target_include_directories(allegro_with_legacy INTERFACE ${allegro5_SOURCE_DIR}/include ${allegro5_BINARY_DIR}/include)
target_include_directories(allegro_with_legacy INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/allegro_legacy/include ${CMAKE_CURRENT_BINARY_DIR}/third_party/allegro_legacy/include)
target_link_libraries(allegro_with_legacy INTERFACE allegro-legacy allegro_primitives allegro_font allegro_image allegro_audio allegro_acodec allegro_main)

if (NOT EMSCRIPTEN)
	target_link_libraries(allegro_with_legacy INTERFACE allegro_dialog)
endif()

get_target_property(ALLEGRO_LINK_LIBRARIES allegro_with_legacy INTERFACE_LINK_LIBRARIES)
list(APPEND ZC_INSTALL_TARGETS allegro ${ALLEGRO_LINK_LIBRARIES})

if(DEFINED ENV{CI} OR ZC_OFFICIAL)
	get_target_property(ALLEGRO_CODEC_SOURCES allegro_acodec SOURCES)
	message("${ALLEGRO_CODEC_SOURCES}")
	if(NOT "ogg.c" IN_LIST ALLEGRO_CODEC_SOURCES)
		message(FATAL_ERROR "Missing support for OGG")
	endif()
	if(NOT "mp3.c" IN_LIST ALLEGRO_CODEC_SOURCES)
		message(FATAL_ERROR "Missing support for MP3")
	endif()
	# TODO: why not working for web build in CI?
	if(NOT EMSCRIPTEN AND NOT "dumb.c" IN_LIST ALLEGRO_CODEC_SOURCES)
		message(FATAL_ERROR "Missing support for DUMB")
	endif()
endif()

#############################################################
# Header and source file lists
#############################################################

set(ZELDA_MODULES
	${CMAKE_SOURCE_DIR}/modules/zelda/ZeldaCore.txt
	${CMAKE_SOURCE_DIR}/modules/zelda/ZeldaGUI.txt
	${CMAKE_SOURCE_DIR}/modules/zelda/ZeldaSprite.txt
	${CMAKE_SOURCE_DIR}/modules/zelda/ZeldaSubscreen.txt
)

set(ZQUEST_MODULES
	${CMAKE_SOURCE_DIR}/modules/zquest/ZQuestCore.txt
	${CMAKE_SOURCE_DIR}/modules/zquest/ZQuestGUI.txt
	${CMAKE_SOURCE_DIR}/modules/zquest/ZQuestMisc.txt
)

set(ZSCRIPT_MODULES
	${CMAKE_SOURCE_DIR}/modules/zscript/ParserCore.txt
	${CMAKE_SOURCE_DIR}/modules/zscript/ParserSrc.txt
)

set(LAUNCHER_MODULES
	${CMAKE_SOURCE_DIR}/modules/launcher/LauncherCore.txt
	${CMAKE_SOURCE_DIR}/modules/launcher/LauncherGUI.txt
)

foreach(module ${ZELDA_MODULES} ${ZQUEST_MODULES} ${LAUNCHER_MODULES} ${ZSCRIPT_MODULES})
	include(${module})
	set_source_files_properties(${module} PROPERTIES HEADER_FILE_ONLY true)
endforeach()

#############################################################
# Build the ZScript parser
#############################################################

find_package(BISON QUIET)
find_package(FLEX QUIET)

if(${BISON_FOUND} AND ${FLEX_FOUND})
	if(MSVC)
		set(FLEXFLAGS "--wincompat")
	endif()
	FLEX_TARGET(ZScriptLexer ${CMAKE_SOURCE_DIR}/src/parser/ffscript.lpp ${CMAKE_SOURCE_DIR}/src/parser/lex.yy.cpp COMPILE_FLAGS ${FLEXFLAGS})
	BISON_TARGET(ZScriptParser ${CMAKE_SOURCE_DIR}/src/parser/ffscript.ypp ${CMAKE_SOURCE_DIR}/src/parser/y.tab.cpp COMPILE_FLAGS -v)
	add_flex_bison_dependency(ZScriptLexer ZScriptParser)
else()
	message(FATAL_ERROR "Flex and Bison not found. ZQuest cannot be compiled without these.")
endif()


#############################################################
# Global settings for the different platforms
#############################################################


# Windows

if(WIN32)
	# Prevents definition of min and max macros, which conflict with std::min and std::max
	add_definitions(-DNOMINMAX)

	string(REGEX MATCH "MSVC" CMAKE_COMPILER_IS_MSVC "${CMAKE_C_COMPILER_ID}")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.20.2750.8)
		message(FATAL_ERROR "MSVC version ${CMAKE_CXX_COMPILER_VERSION} is too old. Must use at least 2019 version 16.0.0")
	endif()
endif()

# Turn off warnings for all targets that aren't ours (warnings for our targets are set in Misc. section)
if(MSVC)
	add_compile_options(/W0)
else()
	add_compile_options(-Wno-error)
endif()

if(MSVC)
	# Build in parallel - speeds up the build on multicore machines.
	add_compile_options(/MP)

	# Require for fmt.
	add_compile_options(/utf-8)

	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCD /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE /OPT:NOICF")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")

elseif(EMSCRIPTEN)
	add_definitions(-DALLEGRO_LINUX) # Mistakenly used in the source.

	add_compile_options("-m32")
	# TODO: remove?
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32 -static-libgcc -export-dynamic")
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32 -static-libgcc")

elseif(LINUX)
	add_definitions(-DALLEGRO_LINUX) # Mistakenly used in the source.
	add_definitions(-DALLEGRO_LEGACY_NO_FIX_ALIASES)

	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -export-dynamic")
		SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
		SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc")
	endif()
endif()

if(MSVC)
	# REQUIRED or else MSVC throws internal compiler error(!)
	add_compile_options("/fp:fast")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# TODO: GCC w/ these flags don't quite match the behavior of MSVC or clang, as seen in a replay report:
	#    https://zc-replay-compare-3700187020.surge.sh/
	# The difference is trivial (and isolated to just stuff re: Patras, or maybe Sin/Cos, unclear), and actually GCC
	# looks more "correct". Should re-assess fp-rounding to guarentee same playback for any compiler; should either:
	# 1) ignore some frames for purposes of replay assert
	# 2) investigate more how to get exact same behavior from gcc
	# 3) investigate removing the fp flags from all compilers and seeing if results can be made to be consistent;
	#    would require a workaround for the MSVC internal compiler error mentioned above.
	#
	# In the meantime, we just use clang or msvc (never gcc) for replay tests (and for building release), which seems fine long term.

	# https://stackoverflow.com/a/36502370/2788187
	add_compile_options(-funsafe-math-optimizations -ffinite-math-only -fno-math-errno -fno-signaling-nans -fno-rounding-math -fcx-limited-range)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options("-ffp-model=fast")
endif()

# Optimize the math functions always, even for debug mode. This reduces many inconsistencies with replays
# in non-Release builds.
if(MSVC)
	# error D8016: '/O2' and '/RTC1' command-line options are incompatible
	# set_source_files_properties(src/base/zc_math.cpp PROPERTIES COMPILE_FLAGS /O2)
else()
	set_source_files_properties(src/base/zc_math.cpp PROPERTIES COMPILE_FLAGS -O3)
endif()

# Make __FILE__ relative to the source directory.
if(MSVC)
	add_compile_options("/d1trimfile:$(SolutionDir)")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options(-fmacro-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=.)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options(-mssse3)
endif()

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(is_multi_config)
	set(BUILD_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
else()
	set(BUILD_FOLDER "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

#############################################################
# Allegro Graphics Addons
#############################################################

add_library(alpng STATIC
	third_party/al_loadpng/loadpng.c
	third_party/al_loadpng/regpng.c
	third_party/al_loadpng/savepng.c
)
target_link_libraries(alpng PUBLIC
	allegro_with_legacy
	${PNG_LIBRARIES}
)
target_include_directories(alpng PUBLIC
	third_party/al_loadpng
)
target_include_directories(alpng PRIVATE
	${PNG_INCLUDE_DIRS}
)
list(APPEND IMAGELIBS alpng)

add_library(algif STATIC
	third_party/al_gif/src/save_gif.cpp
	third_party/al_gif/src/load_gif.cpp
)
target_include_directories(algif PUBLIC
	third_party/al_gif/include
)
target_link_libraries(algif PUBLIC allegro_with_legacy)
list(APPEND IMAGELIBS algif)

add_library(al5img STATIC
	src/al5_img/src/al5_img.c
)
target_include_directories(al5img PUBLIC
	src/al5_img/include
)
target_link_libraries(al5img PUBLIC allegro_with_legacy algif)
list(APPEND IMAGELIBS al5img)

#############################################################
# Common libraries
#############################################################

# Remove once generally available in all compilers for c++ 20.
add_library(fmt STATIC
	third_party/fmt/src/format.cc
)
target_include_directories(fmt PUBLIC SYSTEM third_party/fmt/include)

set(zcbase_sources
	src/base/about.cpp
	src/base/autocombo.cpp
	src/base/colors.cpp
	src/base/combo.cpp
	src/base/containers.cpp
	src/base/cpool.cpp
	src/base/cpos_info.cpp
	src/base/dmap.cpp
	src/base/files.cpp
	src/base/fonts.cpp
	src/base/general.cpp
	src/base/gui.cpp
	src/base/handles.cpp
	src/base/hotkey.cpp
	src/base/initdata.cpp
	src/base/jwinfsel.cpp
	src/base/mapscr.cpp
	src/base/md5.cpp
	src/base/misctypes.cpp
	src/base/msgstr.cpp
	src/base/new_menu.cpp
	src/base/packfile.cpp
	src/base/pal_tables.cpp
	src/base/process_management.cpp
	src/base/qrs.cpp
	src/base/qst.cpp
	src/base/random.cpp
	src/base/render.cpp
	src/base/scc.cpp
	src/base/sin1.cpp
	src/base/util.cpp
	src/base/version.cpp
	src/base/weapon_data.cpp
	src/base/win32.cpp
	src/base/worker_pool.cpp
	src/base/zapp.cpp
	src/base/zc_alleg.cpp
	src/base/zc_math.cpp
	src/base/zdefs.cpp
	src/base/zfix.cpp
	src/base/zsys.cpp
)
if (EMSCRIPTEN)
	list(APPEND zcbase_sources src/base/emscripten_utils.cpp)
endif()
add_library(zcbase STATIC ${zcbase_sources})
target_link_libraries(zcbase PUBLIC allegro_with_legacy fmt)
target_include_directories(zcbase PUBLIC src third_party/nonstd/include)
if(CURL_FOUND)
	target_sources(zcbase PRIVATE src/base/http.cpp)
	target_link_libraries(zcbase PUBLIC CURL::libcurl)
	target_compile_definitions(zcbase PUBLIC HAS_CURL)
else()
	message(WARNING "libcurl not found - disabling base/http.cpp")
endif()
if(NOT WIN32)
	target_include_directories(zcbase PUBLIC include/pstream)
endif()
if (USE_NFD)
	target_link_libraries(zcbase PUBLIC nfd)
	target_compile_definitions(zcbase PUBLIC HAS_NFD)
	list(APPEND ZC_INSTALL_TARGETS nfd)
endif()

if(WANT_ZC_TESTS)
	add_executable(base_test_runner
		src/test_runner/base_test_runner.cpp
		src/test_runner/test_runner.cpp
		src/base/scc_test.cpp
	)
	target_link_libraries(base_test_runner PRIVATE zcbase)

	if(DEFINED ENV{CI})
		list(APPEND ZC_INSTALL_TARGETS base_test_runner)
	endif()
endif()

# std::execution supplementation.
# For std::execution::par, seq, and par_unseq.
# On compilers with support poolSTL does nothing, else poolSTL aliases poolstl::par to the std::execution policies.
# This way users may write code using standard std::execution and still support platforms lacking support.
# Source: https://github.com/alugowski/poolSTL/blob/main/tests/CMakeLists.txt
FetchContent_Declare(
	poolSTL
	GIT_REPOSITORY https://github.com/alugowski/poolSTL
	GIT_TAG 26d95b90aea7c36732a2df50df1c6fa26c96f93e
	GIT_SHALLOW TRUE
	EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(poolSTL)
add_library(poolSTL_tbb INTERFACE)
target_link_libraries(poolSTL_tbb INTERFACE poolSTL)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # GCC and Clang require TBB for std::execution::par.
    # MinGW will compile std::execution::par without TBB, but performance is sequential. (Verified as of MinGW 13).
    find_package(TBB)
    if (TBB_FOUND)
		message("Linking TBB")
        target_link_libraries(poolSTL_tbb INTERFACE TBB::tbb)
    else()
        message("No TBB - using poolSTL fallback")
        # Prevent including <execution>, as doing that requires linking against TBB on newer GCC/Clang.
        # User code must not #include <execution> either.
        target_compile_definitions(poolSTL_tbb INTERFACE POOLSTL_STD_SUPPLEMENT_NO_INCLUDE)
		target_compile_definitions(poolSTL_tbb INTERFACE POOLSTL_STD_SUPPLEMENT_FORCE)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_link_options(zcbase PRIVATE "/WHOLEARCHIVE")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_link_options(zcbase PRIVATE "-force_load")
endif()

add_library(zalleg STATIC src/zalleg/zalleg.cpp)
target_link_libraries(zalleg PUBLIC allegro_with_legacy zcbase ${IMAGELIBS})

add_library(zcgui STATIC
	src/gui/buildutil.cpp
	src/gui/button.cpp
	src/gui/checkbox_qr.cpp
	src/gui/checkbox.cpp
	src/gui/colorsel.cpp
	src/gui/common.cpp
	src/gui/ddpanel.cpp
	src/gui/dialog_ref.cpp
	src/gui/dialog_runner.cpp
	src/gui/drop_down_list.cpp
	src/gui/editbox.cpp
	src/gui/EditboxModel.cpp
	src/gui/EditboxView.cpp
	src/gui/frame.cpp
	src/gui/grid.cpp
	src/gui/jwin_a5.cpp
	src/gui/jwin.cpp
	src/gui/label.cpp
	src/gui/list_data.cpp
	src/gui/list.cpp
	src/gui/menu.cpp
	src/gui/radio.cpp
	src/gui/radioset.cpp
	src/gui/scrolling_pane.cpp
	src/gui/size.cpp
	src/gui/switcher.cpp
	src/gui/tabpanel.cpp
	src/gui/tabref.cpp
	src/gui/text_field.cpp
	src/gui/top_level.cpp
	src/gui/widget.cpp
	src/gui/window.cpp
	src/gui/slider.cpp
	src/gui/ditherprev.cpp
	src/gui/separator.cpp
)
target_link_libraries(zcgui PUBLIC allegro_with_legacy zcbase)
target_include_directories(zcgui PRIVATE src)

add_library(zasm STATIC
	src/zasm/serialize.cpp
	src/zasm/table.cpp
)
target_link_libraries(zasm PUBLIC allegro_with_legacy zcbase fmt)

#############################################################
# ZConsole
#############################################################

if(WIN32)
	add_executable(zconsole src/zconsole/ConsoleLoggerHelper.cpp)
	target_link_options(zconsole PRIVATE "/SUBSYSTEM:CONSOLE")
	list(APPEND ZC_INSTALL_TARGETS zconsole)
endif()

add_library(zconsolelogger STATIC src/zconsole/ConsoleLogger.cpp)
target_link_libraries(zconsolelogger PUBLIC allegro_with_legacy zcbase)
target_include_directories(zconsolelogger PUBLIC src)

target_link_libraries(zcbase PUBLIC zconsolelogger)

#############################################################
# Sound Library
#############################################################

FetchContent_Declare(
	gme_external
	GIT_REPOSITORY https://github.com/libgme/game-music-emu
	GIT_TAG        05a2aa29e8eae29316804fdd28ceaa96c74a1531
	EXCLUDE_FROM_ALL
)
set(GME_YM2612_EMU "Nuked" CACHE STRING "Which YM2612 emulator to use: \"Nuked\" (LGPLv2.1+), \"MAME\" (GPLv2+), or \"GENS\" (LGPLv2.1+)")
FetchContent_MakeAvailable(gme_external)
add_library(gme_headers INTERFACE)
target_include_directories(gme_headers INTERFACE ${gme_external_SOURCE_DIR}/gme)
target_link_libraries(gme_headers INTERFACE ${ZLIB_LIBRARIES})
target_compile_definitions(gme_headers INTERFACE -DVGM_YM2612_NUKED)

set(SOUNDLIBS gme gme_headers)
list(APPEND ZC_INSTALL_TARGETS gme)

if(MSVC AND (MSVC_VERSION GREATER 1600))
	set(ZCSOUNDLIBSEXTRA legacy_stdio_definitions)
endif()
set(ZCSOUNDSOURCES
	src/sound/zcmusic.cpp
	src/sound/zcmixer.cpp
)
add_library(zcsound SHARED ${ZCSOUNDSOURCES})
target_link_libraries(zcsound PUBLIC allegro_with_legacy zcbase ${SOUNDLIBS} ${ZCSOUNDLIBSEXTRA})

target_compile_definitions(zcsound PRIVATE SOUND_LIBS_BUILT_FROM_SOURCE)

#############################################################
# Networking
#############################################################

if(NOT WANT_WEBSOCKETS)
	add_library(websocket INTERFACE)
	target_sources(websocket INTERFACE src/zc/websocket_pool_unsupported.cpp)
elseif(EMSCRIPTEN)
	add_library(websocket INTERFACE)
	target_sources(websocket INTERFACE src/zc/websocket_pool_web.cpp)
else()
	find_package(OpenSSL)

	FetchContent_Declare(
		asio
		GIT_REPOSITORY
			https://github.com/chriskohlhoff/asio.git
		GIT_TAG
			ed5db1b50136bace796062c1a6eab0df9a74f8fa
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(asio)
	add_library(asio INTERFACE)
	target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)

	FetchContent_Declare(
		websocketpp
		GIT_REPOSITORY
			https://github.com/zaphoyd/websocketpp.git
		GIT_TAG
			b9aeec6eaf3d5610503439b4fae3581d9aff08e8
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(websocketpp)

	add_library(websocket INTERFACE)
	target_include_directories(websocket INTERFACE ${websocketpp_SOURCE_DIR})
	target_compile_definitions(websocket INTERFACE _WEBSOCKETPP_CPP11_STL_ ASIO_STANDALONE)
	target_link_libraries(websocket INTERFACE asio)
	if (OpenSSL_FOUND)
		target_link_libraries(websocket INTERFACE OpenSSL::SSL)
		target_compile_definitions(websocket INTERFACE HAS_SSL)
	endif()

	target_sources(websocket INTERFACE src/zc/websocket_pool_native.cpp)
	# websocketpp cmake stomps on ours ...
	set(CMAKE_CONFIGURATION_TYPES Asan Debug Release RelWithDebInfo Coverage CACHE STRING INTERNAL FORCE)
endif()

#############################################################
# Zelda
#############################################################

if(MSVC)
	list(APPEND ZELDAEXTRASOURCES icons/zc_icon.rc)

elseif(LINUX)
	list(APPEND ZELDAEXTRASOURCES icons/zc_icon.c)
endif()

if(UNITY_BUILD)
	enable_unity_build(ZeldaCore ZELDA_CORE_SOURCES)
	enable_unity_build(ZeldaGUI ZELDA_GUI_SOURCES)
	enable_unity_build(ZeldaSprite ZELDA_SPRITE_SOURCES)
	enable_unity_build(ZeldaSubscreen ZELDA_SUBSCREEN_SOURCES)
endif()

add_executable(zplayer ${ZELDA_CORE_SOURCES} ${ZELDA_GUI_SOURCES} ${ZELDA_SPRITE_SOURCES} ${ZELDA_SUBSCREEN_SOURCES} ${ZELDA_SCRIPTING_SOURCES} ${ZELDAEXTRASOURCES} ${ZELDA_MODULES})
target_compile_definitions(zplayer PRIVATE IS_PLAYER)
set_property(TARGET zplayer PROPERTY VS_DPI_AWARE "PerMonitor")
target_include_directories(zplayer PRIVATE include/xxhash)
target_link_libraries(zplayer PRIVATE allegro_with_legacy zasm nlohmann_json::nlohmann_json poolSTL_tbb)
if (NOT EMSCRIPTEN)
	target_link_libraries(zplayer PRIVATE stduuid)
	target_compile_definitions(zplayer PRIVATE HAS_UUID)
endif()
if(CURL_FOUND)
	target_sources(zplayer PRIVATE src/zc/replay_upload.cpp)
else()
	message(WARNING "libcurl not found - disabling zc/replay_upload.cpp")
endif()

if(WIN32)
	target_link_libraries(zplayer PRIVATE zalleg zconsolelogger zcbase zcgui zcsound websocket winmm ${IMAGELIBS} ${ZELDALIBSEXTRA})
elseif(LINUX)
	target_link_libraries(zplayer PRIVATE zalleg zconsolelogger zcbase zcgui zcsound websocket ${IMAGELIBS} ${ZELDALIBSEXTRA})
elseif(APPLE)
	target_link_libraries(zplayer PRIVATE zalleg zconsolelogger zcbase zcgui zcsound websocket ${IMAGELIBS} ${ZELDALIBSEXTRA})
endif()

if(JIT_BACKEND STREQUAL "default")
	if(EMSCRIPTEN)
		set(JIT_BACKEND "wasm")
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(JIT_BACKEND "x64")
	else()
		set(JIT_BACKEND "none")
	endif()
endif()

if(JIT_BACKEND STREQUAL "none")
	target_sources(zplayer PUBLIC src/zc/jit_none.cpp)
	# This is defined on zcbase b/c currently qst.cpp relies on ffscript.h
	target_compile_definitions(zcbase PUBLIC ZC_JIT_BACKEND=0)
elseif(JIT_BACKEND STREQUAL "x64")
	target_sources(zplayer PUBLIC src/zc/jit_x64.cpp)
	target_compile_definitions(zcbase PUBLIC ZC_JIT_BACKEND=1)

	FetchContent_Declare(
		asmjit
		GIT_REPOSITORY
			https://github.com/connorjclark/asmjit.git
		GIT_TAG
			9e9494808f1d9101e71e75046d75e762b5ece21c
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(asmjit)
	target_link_libraries(zplayer PRIVATE asmjit)
	list(APPEND ZC_INSTALL_TARGETS asmjit)

	get_target_property(asm_compile_options asmjit COMPILE_OPTIONS)
	string(REPLACE "$<NOT:$<CONFIG:Debug>>" "$<NOT:$<CONFIG:Debug,Asan>>" asm_compile_options "${asm_compile_options}")
	set_target_properties(asmjit PROPERTIES COMPILE_OPTIONS "${asm_compile_options}")
elseif(JIT_BACKEND STREQUAL "wasm")
	target_sources(zplayer PUBLIC src/zc/jit_wasm.cpp)
	target_compile_definitions(zcbase PUBLIC ZC_JIT_BACKEND=2)
else()
	message(FATAL_ERROR "Invalid value for JIT_BACKEND")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND WIN32)
	target_compile_definitions(zplayer PRIVATE ZC_WIN_32)
endif()

# For some reason compilation fails in CI, but only for ubuntu clang job...
#   Linking CXX executable RelWithDebInfo/zscript
#   /usr/bin/ld: /usr/bin/ld: DWARF error: invalid or unhandled FORM value: 0x23
#   RelWithDebInfo/libzcbase.a(zsys.cpp.o): in function `temp_name(char*)':
if(NOT EMSCRIPTEN AND NOT (DEFINED ENV{CI} AND CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	file(GLOB_RECURSE zscript_binding_files "resources/include/bindings/*.zh")
	list(REMOVE_ITEM zscript_binding_files "${CMAKE_CURRENT_SOURCE_DIR}/resources/include/bindings/qrs.zh")
	add_custom_command(
		COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_zasm_context_strings.py --build_folder "${BUILD_FOLDER}"
		OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/zc/scripting/context_strings.cpp
		DEPENDS scripts/generate_zasm_context_strings.py ${zscript_binding_files}
	)
	add_custom_target(gen_zscript_context_strings DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/zc/scripting/context_strings.cpp zscript)
	add_dependencies(zplayer gen_zscript_context_strings)
endif()

# qr generation
set(gen_zscript_qr_enum scripts/generate_qr_constants.py)
add_custom_command(
	COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_qr_constants.py --build_folder "${BUILD_FOLDER}"
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/resources/include/bindings/qrs.zh
	DEPENDS ${gen_zscript_qr_enum}
)
add_custom_target(gen_zscript_qr_enum DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/include/bindings/qrs.zh zeditor)


#############################################################
# ZQuest
#############################################################

if(MSVC)
	list(APPEND ZQUESTEXTRASOURCES icons/zq_icon.rc)
elseif(LINUX)
	list(APPEND ZQUESTEXTRASOURCES icons/zq_icon.c)
endif()

if(UNITY_BUILD)
	enable_unity_build(ZQuestCore ZQUEST_CORE_SOURCES)
	enable_unity_build(ZQuestGUI ZQUEST_GUI_SOURCES)
	enable_unity_build(ZQuestMisc ZQUEST_MISC_SOURCES)
	enable_unity_build(ZQuestZScript PARSER_SOURCES)
endif()

add_executable(zeditor ${ZQUEST_CORE_SOURCES} ${ZQUEST_GUI_SOURCES} ${ZQUEST_MISC_SOURCES} ${ZQUESTEXTRASOURCES} ${ZQUEST_MODULES})
set_property(TARGET zeditor PROPERTY VS_DPI_AWARE "PerMonitor")

target_link_libraries(zeditor PUBLIC zalleg zconsolelogger zcbase zcgui zcsound zasm ${IMAGELIBS} ${ZQUESTLIBSEXTRA} nlohmann_json::nlohmann_json)
target_include_directories(zeditor PRIVATE src)

if(MSVC)
	target_compile_definitions(zcsound PRIVATE ZCM_DLL)
	target_compile_definitions(zplayer PRIVATE ZCM_DLL_IMPORT)
	target_compile_definitions(zeditor PRIVATE ZCM_DLL_IMPORT)
endif()
target_compile_definitions(zeditor PRIVATE IS_EDITOR)

if(WIN32)
	set_source_files_properties(
		src/dialog/itemeditor.cpp
		src/dialog/init_data.cpp
		src/dialog/comboeditor.cpp
		src/dialog/combowizard.cpp
		src/dialog/subscr_props.cpp
		PROPERTIES COMPILE_FLAGS "/bigobj"
	)
endif()

#############################################################
# ZScript parser
#############################################################

if(${BISON_FOUND} AND ${FLEX_FOUND})
	add_executable(zscript ${ZSCRIPT_CORE_SOURCES} ${PARSER_SOURCES} ${BISON_ZScriptParser_OUTPUTS} ${FLEX_ZScriptLexer_OUTPUTS} ${ZSCRIPT_MODULES})
	target_include_directories(zscript PRIVATE src)
	target_link_libraries(zscript PRIVATE fmt zcbase zconsolelogger zasm ${ZSCRIPTLIBSEXTRA} nlohmann_json::nlohmann_json)
	target_compile_definitions(zscript PRIVATE IS_PARSER)
endif()

#############################################################
# Launcher
#############################################################

if(MSVC)
	list(APPEND LAUNCHEREXTRASOURCES icons/zl_icon.rc)
elseif(LINUX)
	list(APPEND LAUNCHEREXTRASOURCES icons/zl_icon.c)
endif()

add_executable(zlauncher ${LAUNCHER_SOURCES} ${LAUNCHER_GUI_SOURCES} ${LAUNCHEREXTRASOURCES})
target_compile_definitions(zlauncher PRIVATE IS_LAUNCHER)
set_property(TARGET zlauncher PROPERTY VS_DPI_AWARE "PerMonitor")
target_link_libraries(zlauncher PRIVATE zalleg zcbase zcgui zconsolelogger ${LAUNCHERLIBSEXTRA})

#############################################################
# Standalone
#############################################################

if(WIN32)
	add_executable(zstandalone src/standalone/standalone.cpp)
	target_link_options(zstandalone PRIVATE "/SUBSYSTEM:WINDOWS")
	add_dependencies(zeditor zstandalone)
	list(APPEND ZC_INSTALL_TARGETS zstandalone)
endif()

#############################################################
# ZUpdater
#############################################################

# TODO give zupdater its own icon.
if(MSVC)
	list(APPEND UPDATEREXTRASOURCES icons/zl_icon.rc)
elseif(LINUX)
	list(APPEND UPDATEREXTRASOURCES icons/zl_icon.c)
endif()

add_executable(zupdater src/zupdater/zupdater.cpp src/zconfig.cpp ${UPDATEREXTRASOURCES})
target_compile_definitions(zupdater PRIVATE ZUPDATER)
target_link_libraries(zupdater PRIVATE allegro_dialog fmt zcbase)
target_include_directories(zupdater PRIVATE src)
if(WIN32)
	target_link_options(zupdater PRIVATE "/SUBSYSTEM:WINDOWS")
endif()

if(CURL_FOUND AND WIN32)
	message(STATUS "Using curl for zupdater")

	target_link_libraries(zupdater PRIVATE CURL::libcurl)

	# add_library(json INTERFACE third_party/json/json/json.h)
	# target_include_directories(json INTERFACE third_party/json)
	target_link_libraries(zupdater PRIVATE nlohmann_json::nlohmann_json)

	FetchContent_Declare(
		miniz
		GIT_REPOSITORY
			https://github.com/richgel999/miniz.git
		GIT_TAG
			9ae305f6e109f8f1fbd2130458f1ee6197269b3b
		EXCLUDE_FROM_ALL
	)
	set(BUILD_SHARED_LIBS_PREV ${BUILD_SHARED_LIBS})
	set(BUILD_SHARED_LIBS OFF)
	FetchContent_MakeAvailable(miniz)
	set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_PREV} CACHE BOOL "" FORCE)
	target_link_libraries(zupdater PRIVATE miniz)
else()
	message(WARNING "Using Python for zupdater")

	target_compile_definitions(zupdater PRIVATE UPDATER_USES_PYTHON)
	target_compile_definitions(zlauncher PRIVATE UPDATER_USES_PYTHON)
endif()

#############################################################
# Emscripten
#############################################################

if (EMSCRIPTEN)
	target_link_options(zplayer PRIVATE "SHELL:-s EXPORTED_FUNCTIONS=_main,_get_shareable_url,_create_synthetic_key_event,_em_do_commands,_em_get_register,_em_set_register,_em_runtime_script_debug,_em_log_error -s EXPORTED_RUNTIME_METHODS=cwrap,wasmMemory,HEAPU8,HEAP32")
	target_link_options(zeditor PRIVATE "SHELL:-s EXPORTED_FUNCTIONS=_main,_get_shareable_url,_create_synthetic_key_event,_open_test_mode")
	target_link_options(zscript PRIVATE "SHELL:-no-pthread -s EXPORTED_FUNCTIONS=_main,_compile_script -s EXPORTED_RUNTIME_METHODS=cwrap,FS,PROXYFS")
	set_target_properties(zscript PROPERTIES SUFFIX ".mjs")
endif()

#############################################################
# Misc.
#############################################################

set(zc_targets zeditor zplayer zlauncher zalleg zcbase zcgui zcsound zscript zconsolelogger zupdater)
set(zc_clang_errors
	-Werror=array-bounds
	-Werror=format
	-Werror=inconsistent-missing-override
	-Werror=return-type
	-Werror=type-limits
	-Werror=unreachable-code
	# -Werror=reorder
	# -Werror=unused
	-Wno-write-strings
	-Wno-switch
)
set(zc_gcc_errors
	-Werror=array-bounds
	-Werror=format
	# GCC currently gives some false positives for format-overflow.
	# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108091
	-Wno-error=format-overflow
	-Werror=return-type
	-Werror=unreachable-code
	# -Werror=unused
	-Wno-write-strings
	-Wno-switch
	# TODO
	# -Wunused-result
)
set(zc_msvc_errors
	# /we5038
	# /we4265
)

foreach(target IN LISTS zc_targets)
	target_compile_definitions(${target} PRIVATE ZC_APP_NAME="${target}")
	target_compile_definitions(${target} PRIVATE "$<$<CONFIG:DEBUG>:DCHECK_IS_ON>")
	target_compile_definitions(${target} PRIVATE "$<$<CONFIG:ASAN>:DCHECK_IS_ON>")
endforeach()

if(WANT_SENTRY)
	set(SENTRY_VERSION 0.6.6)
	set(SENTRY_BACKEND crashpad)
	set(SENTRY_BUILD_SHARED_LIBS OFF)
	FetchContent_Declare(sentry
		URL https://github.com/getsentry/sentry-native/releases/download/${SENTRY_VERSION}/sentry-native.zip
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(sentry)

	foreach(target IN LISTS zc_targets)
		target_link_libraries(${target} PRIVATE sentry)
		target_compile_definitions(${target} PRIVATE HAS_SENTRY)
	endforeach()

	list(APPEND ZC_INSTALL_TARGETS crashpad_handler)
endif()

# See docs/versioning.md
set(gen_version_header_deps scripts/generate_version_header.py)
find_program(GIT_EXECUTABLE NAMES git)
if(GIT_EXECUTABLE)
	execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --git-dir
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
	list(APPEND gen_version_header_deps
		"${CMAKE_CURRENT_SOURCE_DIR}/${GIT_DIR}/HEAD"
		"${CMAKE_CURRENT_SOURCE_DIR}/${GIT_DIR}/refs/tags"
        "${CMAKE_CURRENT_SOURCE_DIR}/${GIT_DIR}/index"
	)
endif()
add_custom_command(
	COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_version_header.py --version "${ZC_VERSION}"
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/base/version_header.h
	DEPENDS ${gen_version_header_deps}
	COMMENT "Generating src/base/version_header.h ..."
)
add_custom_target(generate_version_header DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/base/version_header.h)
add_dependencies(zcbase generate_version_header)

if(RELEASE_PLATFORM)
	foreach(target IN LISTS zc_targets)
		target_compile_definitions(${target} PRIVATE RELEASE_PLATFORM="${RELEASE_PLATFORM}")
	endforeach()
endif()

if(RELEASE_CHANNEL)
	foreach(target IN LISTS zc_targets)
		target_compile_definitions(${target} PRIVATE RELEASE_CHANNEL="${RELEASE_CHANNEL}")
	endforeach()
endif()

if(REPO)
	foreach(target IN LISTS zc_targets)
		target_compile_definitions(${target} PRIVATE REPO="${REPO}")
	endforeach()
endif()

if(WANT_FUZZ)
	foreach(target IN LISTS zc_targets)
		target_compile_definitions(${target} PRIVATE ZC_FUZZ)
	endforeach()
endif()

if (WIN32)
	list(APPEND zc_targets zconsole zstandalone)
	add_dependencies(zplayer zconsole)
endif()

if (MSVC)
	foreach(target IN LISTS zc_targets)
		# Yes MSVC, we want C++ 20 features. https://devblogs.microsoft.com/cppblog/announcing-full-support-for-a-c-c-conformant-preprocessor-in-msvc/
		target_compile_options(${target} PRIVATE /Zc:preprocessor)
	endforeach()
endif()

if(USE_PCH)
	foreach(target IN LISTS zc_targets)
		target_precompile_headers(${target}
			PRIVATE
				[["base/zc_alleg.h"]]
				<cassert>
				<cctype>
				<cmath>
				<cstdarg>
				<cstdint>
				<cstdio>
				<cstdlib>
				<cstring>
				<map>
				<set>
				<string>
				<vector>
		)
	endforeach()
endif()

foreach(target IN LISTS zc_targets)
	if(MSVC)
		target_compile_definitions(${target} PRIVATE __STDC_LIMIT_MACROS)
		if(WANT_ERROR_CHECKING)
			# TODO: set /W2 higher.
			target_compile_options(${target} PRIVATE /W2 /WX ${zc_msvc_errors})
		endif()
	endif()

	if(WANT_ERROR_CHECKING AND CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT EMSCRIPTEN)
		target_compile_options(${target} PRIVATE -Wall)
		# Uncomment to autofix. Only works for clang. When done, comment this line out again.
		# target_compile_options(${target} PRIVATE -ferror-limit=10000 -Wno-everything ${zc_clang_errors} "SHELL:-Xclang -fix-what-you-can" "SHELL:-Xclang -fixit-recompile")
		target_compile_options(${target} PRIVATE ${zc_clang_errors})
	endif()
	if(WANT_ERROR_CHECKING AND CMAKE_C_COMPILER_ID MATCHES "GNU")
		target_compile_options(${target} PRIVATE -Wall ${zc_gcc_errors})
	endif()
endforeach()

# Disable warnings for generated file.
if(WIN32)
	set_source_files_properties(src/parser/lex.yy.cpp PROPERTIES COMPILE_FLAGS /W0)
else()
	set_source_files_properties(src/parser/lex.yy.cpp PROPERTIES COMPILE_FLAGS -Wno-everything)
endif()

# Add some targets for running replay tests.
# TODO: Visual Studio can't cancel custom commands from CMake
# https://gitlab.kitware.com/cmake/cmake/-/issues/25183
# https://stackoverflow.com/q/43364914
function (create_replay_tests_target target_name)
	list(SUBLIST ARGV 1 -1 command_args)
	add_custom_command(
		USES_TERMINAL
		COMMAND ${PYTHON} -Xutf8 ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_replay_tests.py --no-emoji --build_folder "${BUILD_FOLDER}" ${command_args}
		OUTPUT ${target_name}.stamp
	)
	add_custom_target(${target_name} DEPENDS ${target_name}.stamp zplayer)
	set_source_files_properties(${target_name}.stamp PROPERTIES SYMBOLIC 1)
endfunction()

# Run everything.
create_replay_tests_target(run_replay_tests_all --assert)
# Run just the short ones.
create_replay_tests_target(run_replay_tests_basic --assert --filter playground --filter classic_1st.zplay --filter dark)
# Don't use jit, just because there is lots of overhead to compilation that will make it a waste when
# running just a few seconds.
create_replay_tests_target(run_replay_tests_all_1s --assert --no-jit --max_duration 1)
create_replay_tests_target(run_replay_tests_all_10s --assert --no-jit --max_duration 10)
create_replay_tests_target(run_replay_tests_all_100s --assert --no-jit --max_duration 100)

if(EMSCRIPTEN)
	add_subdirectory(web)
endif()

add_subdirectory(packaging)

#############################################################
# Visual Studio
#############################################################

if(CMAKE_GENERATOR MATCHES "Visual Studio")
	# "Edit and Continue" functionality
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")

	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT zeditor)

	# Configure "Debug" option in VS to use the correct working directory, otherwise it fails to load zc configuration files
	set_property(TARGET ${zc_targets} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(ProjectDir)$(Configuration)")

	# Organize into folders
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	set_target_properties(zplayer zeditor zscript zlauncher zupdater package_zc PROPERTIES FOLDER "")
	set_target_properties(run_replay_tests_all run_replay_tests_basic run_replay_tests_all_1s run_replay_tests_all_10s run_replay_tests_all_100s PROPERTIES FOLDER "Replay Tests")
	set_target_properties(zcbase zalleg zcgui zcsound PROPERTIES FOLDER "Our Libraries")
	set_target_properties(gme PROPERTIES FOLDER Sound)
	set_target_properties(allegro allegro_audio allegro_acodec allegro_image allegro_font allegro_main allegro_primitives allegro_dialog allegro-legacy al5img algif alpng PROPERTIES FOLDER Allegro)
	set_target_properties(zconsole zconsolelogger PROPERTIES FOLDER ZConsole)
	if(WANT_SENTRY)
		set_target_properties(crashpad_client crashpad_compat crashpad_getopt crashpad_handler crashpad_handler_lib crashpad_minidump crashpad_snapshot crashpad_tools crashpad_util crashpad_zlib mini_chromium sentry PROPERTIES FOLDER Sentry)
	endif()

	# Put everything else in "Other" folder
	function(get_all_targets var)
		set(targets)
		get_all_targets_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
		set(${var} ${targets} PARENT_SCOPE)
	endfunction()

	macro(get_all_targets_recursive targets dir)
		get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
		foreach(subdir ${subdirectories})
			get_all_targets_recursive(${targets} ${subdir})
		endforeach()

		get_property(current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
		list(APPEND ${targets} ${current_targets})
	endmacro()

	get_all_targets(all_targets)
	foreach(target IN LISTS all_targets)
		get_property(folder_set TARGET ${target} PROPERTY FOLDER SET)
		if(NOT folder_set)
			set_target_properties(${target} PROPERTIES FOLDER Other)
		endif()
	endforeach()

	# The sound system has a known memory issue, so for now set the default debugging args to disable sound.
	set_property(TARGET ${zc_targets} PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "$<$<CONFIG:ASAN>:-s>")
endif()
