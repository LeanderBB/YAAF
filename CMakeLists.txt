################################################################################
# YAAF - Yet Another Archive Format
################################################################################
project(YAAF C)
cmake_minimum_required(VERSION 2.8)


################################################################################
# Options
################################################################################

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

set(YAFF_C_WERROR )

set(YAAF_C_WARNING_FLAGS )

set(YAAF_C_COMPILE_FLAGS "${YAAF_C_COMPILER_EXTRA}")

set(YAAF_C_COMPILE_FLAGS_DEBUG "/DYAAF_DEBUG")

set(YAAF_C_COMPILE_FLAGS_RELEASE )
endif()

set(CMAKE_C_FLAGS "${YAAF_C_WARNING_FLAGS} ${YAAF_C_COMPILE_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${YAAF_C_COMPILE_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_DEBUG "${YAAF_C_COMPILE_FLAGS_DEBUG}")

################################################################################
# Header Tests
################################################################################

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)

#check string functions
check_include_files(strings.h YAAF_HAVE_STRINGS_H)
if(${YAAF_HAVE_STRINGS_H})
set(CMAKE_REQUIRED_INCLUDES string.h strings.h)
else()
set(CMAKE_REQUIRED_INCLUDES string.h)
endif()
check_function_exists(strcasecmp YAAF_HAVE_STRCASECMP)
check_function_exists(stricmp YAAF_HAVE_STRICMP)
unset(CMAKE_REQUIRED_INCLUDES)

# check dirent
check_include_files(dirent.h YAAF_HAVE_DIRENT_H)

#check inttypes
check_include_files(inttypes.h YAAF_HAVE_INTTYPES_H)

# check malloc.h
check_include_files(malloc.h YAAF_HAVE_MALLOC_H)

# check mmap.h
check_include_files(sys/mman.h YAAF_HAVE_MMAN_H)

# check pthread.h
check_include_files(pthread.h YAAF_HAVE_PTHREAD_H)

# check windows.h
check_include_files(windows.h YAAF_HAVE_WINDOWS_H)

# find threads
if(UNIX)
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
endif()

find_package(Threads REQUIRED)

################################################################################
# 3rd party libs
################################################################################

add_subdirectory(3rdparty)


add_subdirectory(libyaaf)

if(YAAF_BUILD_TEST)
add_subdirectory(test)
endif()


if(NOT YAAF_EXCLUDE_YAAFCL)
add_subdirectory(yaafcl)
endif()

