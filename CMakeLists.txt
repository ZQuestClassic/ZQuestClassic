cmake_minimum_required(VERSION 3.2.0)

project(ZeldaClassic)

# Have most of the cmake definitions in a different file
# to allow easier cross compiling.
include(CMakeZcCore.cmake)

# Where our source files are stored.
add_subdirectory(src)
