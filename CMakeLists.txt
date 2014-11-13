################################################################################
# YAAF - Yet Another Archive Format
################################################################################
project(YAAF)
cmake_minimum_required(VERSION 2.8)

################################################################################
# Compiler flags
################################################################################
if(NOT MSVC)
set(YAFF_C_WERROR "-Werror")
set(YAAF_C_WARNING_FLAGS "-Wall -Wextra -Winit-self -Wuninitialized -Wmissing-declarations -pedantic -fvisibility=hidden")
set(YAAF_CXX_WARNING_FLAGS "-Woverloaded-virtual")

set(YAAF_C_COMPILE_FLAGS "-std=c99")
set(YAAF_CXX_COMPILE_FLAGS "-std=c++11 -fno-exceptions -fno-rtti")

set(YAAF_C_COMPILE_FLAGS_DEBUG "-g -O0 -fno-inline -DYAAF_DEBUG")
set(YAAF_CXX_COMPILE_FLAGS_DEBUG "-g -O0 -fno-inline -DYAAF_DEBUG")

set(YAAF_C_COMPILE_FLAGS_RELEASE "-O3 -g")
set(YAAF_CXX_COMPILE_FLAGS_RELEASE "-O3 -g")

else()
set(YAFF_C_WERROR )
set(YAAF_C_WARNING_FLAGS )
set(YAAF_CXX_WARNING_FLAGS )

set(YAAF_C_COMPILE_FLAGS )
set(YAAF_CXX_COMPILE_FLAGS )

set(YAAF_C_COMPILE_FLAGS_DEBUG "/DYAAF_DEBUG")
set(YAAF_CXX_COMPILE_FLAGS_DEBUG "/DYAAF_DEBUG")

set(YAAF_C_COMPILE_FLAGS_RELEASE )
set(YAAF_CXX_COMPILE_FLAGS_RELEASE )
endif()

set(CMAKE_C_FLAGS "${YAAF_C_WARNING_FLAGS} ${YAAF_C_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${YAAF_C_WARNING_FLAGS} ${YAAF_CXX_WARNING_FLAGS} ${YAAF_CXX_COMPILE_FLAGS}")

set(CMAKE_C_FLAGS_RELEASE "${YAAF_C_COMPILE_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELEASE "${YAAF_CXX_COMPILE_FLAGS_RELEASE}")

set(CMAKE_C_FLAGS_DEBUG "${YAAF_C_COMPILE_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG "${YAAF_CXX_COMPILE_FLAGS_DEBUG}")

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

################################################################################
# LIB Settings
################################################################################
set(CMAKE_C_FLAGS "${YAAF_C_WARNING_FLAGS} ${YAAF_C_COMPILE_FLAGS} ${YAFF_C_WERROR}")

set(YAAF_LIB_NAME YAAF)
set(YAAF_CONFIG_HEADER_DIR ${PROJECT_BINARY_DIR}/build_config)
set(YAAF_CONFIG_FILE "${YAAF_CONFIG_HEADER_DIR}/YAAF_Config.h")

configure_file("${PROJECT_SOURCE_DIR}/cmake/YAAF_Config.cmake"
               "${YAAF_CONFIG_FILE}")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${YAAF_CONFIG_HEADER_DIR}
  ${YAAF_LZ4_INCLUDE_DIR}
)



################################################################################
# Sources
################################################################################
set(YAAF_HDR
  include/YAAF_Setup.h
  include/YAAF.h
  ${YAAF_CONFIG_FILE}
)

set(YAAF_INTERNAL_HDR
  src/YAAF_Archive.h
  src/YAAF_File.h
  src/YAAF_Internal.h
  src/YAAF_MemFile.h
  src/YAAF_Compression.h
  src/YAAF_Compression_lz4.h
)


set(YAAF_SRC
  src/YAAF_Archive.c
  src/YAAF_File.c
  src/YAAF_Internal.c
  src/YAAF_MemFile.c
  src/YAAF_TLS.c
  src/YAAF_Compression.c
  src/YAAF_Compression_lz4.c
)

add_library(${YAAF_LIB_NAME} ${YAAF_SRC} ${YAAF_HDR} ${YAAF_INTERNAL_HDR})

set(YAAF_LIBRARIES ${YAAF_LIB_NAME} ${YAAF_LZ4_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(${YAAF_LIB_NAME} ${YAAF_LZ4_LIBRARY})

install(TARGETS ${YAAF_LIB_NAME} DESTINATION lib)
install(FILES ${YAAF_HDR} DESTINATION include/YAAF)

if(YAAF_BUILD_TESTS)
add_subdirectory(test)
endif()


if(NOT YAAF_EXCLUDE_TOOLS)
add_subdirectory(tools/yaafcl)
endif()

