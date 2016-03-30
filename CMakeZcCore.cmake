# Set up some common directories.

set(ZC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(ZC_INCLUDE_DIR "${ZC_ROOT_DIR}/include")
set(ZC_SRC_DIR "${ZC_ROOT_DIR}/src")
set(ZC_ALLEGRO_DIR "${ZC_ROOT_DIR}/allegro")

# Common names.
set(ZC_ZCLASSIC_NAME "Zelda Classic")
set(ZC_ZQUEST_NAME "ZQuest")

# Set up our own libraries for searching for later.
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}" "${ZC_CMAKE_DIR}/Modules/")

# Ensure folder support is available.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Check for standard headers.
include(CheckIncludeFiles)
check_include_files(assert.h HAVE_ASSERT_H)
check_include_files(ctype.h HAVE_CTYPE_H)
check_include_files(float.h HAVE_FLOAT_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(math.h HAVE_MATH_H)
check_include_files(stdarg.h HAVE_STDARG_H)
check_include_files(stdio.h HAVE_STDIO_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(string.h HAVE_STRING_H)

check_include_files(sys/time.h HAVE_SYS_TIME_H)

if (HAVE_STDLIB_H AND HAVE_STDARG_H AND HAVE_STRING_H AND HAVE_FLOAT_H)
	set(STDC_HEADERS 1)
endif()

include(CheckFunctionExists)
check_function_exists(log2 HAVE_LOG2)

include(CheckSymbolExists)

include(CheckCXXSymbolExists)

# Double check the defined types. Be prepared to define standard ones.
include(CheckTypeSize)

# Boost is required for this project.
find_package(boost)


