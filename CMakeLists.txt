################################################################################
# YAAF - Yet Another Archive Format
################################################################################
project(YAAF C)
cmake_minimum_required(VERSION 2.8)


################################################################################
# Options
################################################################################

# Set configuration types do Debug and Release
set(CMAKE_CONFIGURATION_TYPES Debug Release)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
message(FATAL_ERROR "Please use either Release or Debug for CMAKE_BUILD_TYPE")
endif()

# use LZ4 as defualt compression algorithm (can't be changed atm)
set(YAAF_USE_COMPRESSION_LZ4 1)

# use XXHASH as default hashing algorithm (can't be changed atm)
set(YAAF_USE_HASH_XXHASH 1)

option(YAAF_BUILD_SHARED_LIB "Build the shared library version of YAAF" NO)

################################################################################
# Compiler flags
################################################################################

if(NOT MSVC)
set(YAAF_SHARED_C_FLAGS "-fPIC")

set(YAFF_C_WERROR "-Werror")

set(YAAF_C_WARNING_FLAGS "-Wall -Wextra -Winit-self -Wuninitialized -Wmissing-declarations -pedantic -fvisibility=hidden")

set(YAAF_C_COMPILE_FLAGS "-std=c99 ${YAAF_C_COMPILER_EXTRA}")

set(YAAF_C_COMPILE_FLAGS_DEBUG "-g -O0 -fno-inline -DYAAF_DEBUG")

set(YAAF_C_COMPILE_FLAGS_RELEASE "-O3 -g")

else()

add_definitions( "/D WIN32" "/D _WINDOWS")

set(YAFF_C_WERROR "/WX")

set(YAAF_C_WARNING_FLAGS "/W3")

set(YAAF_C_COMPILE_FLAGS "${YAAF_C_COMPILER_EXTRA}")

set(YAAF_C_COMPILE_FLAGS_DEBUG "/DYAAF_DEBUG /D _DEBUG /Oy- /Od /Ob0 /Gm- /RTC1 /Zi")

set(YAAF_C_COMPILE_FLAGS_RELEASE "/O2 /Oy- /Gm- /GS /MD /Ob2 /Gd  /D NDEBUG")
endif()

set(CMAKE_C_FLAGS "${YAAF_C_WARNING_FLAGS} ${YAAF_C_COMPILE_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${YAAF_C_COMPILE_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_DEBUG "${YAAF_C_COMPILE_FLAGS_DEBUG}")

################################################################################
# Includes
################################################################################

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)

################################################################################
# 3rd party libs
################################################################################

add_subdirectory(3rdparty)

################################################################################
# libyaaf
################################################################################
add_subdirectory(libyaaf)

################################################################################
# Small compression test
################################################################################
if(YAAF_BUILD_TEST)
add_subdirectory(test)
endif()

################################################################################
# yaafcl
################################################################################
if(NOT YAAF_EXCLUDE_YAAFCL)
add_subdirectory(yaafcl)
endif()

