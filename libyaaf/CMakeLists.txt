################################################################################
# libYAAF Settings
################################################################################

set(YAAF_LIB_NAME YAAF)
set(YAAF_VERSION_MAJOR 1)
set(YAAF_VERSION_MINOR 0)
set(YAAF_VERSION_PATCH 0)

set(CMAKE_C_FLAGS "${YAAF_C_WARNING_FLAGS} ${YAAF_C_COMPILE_FLAGS} ${YAFF_C_WERROR}")

if(${YAAF_BUILD_SHARED_LIB})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${YAAF_SHARED_C_FLAGS}")
set(YAAF_LIB_MODE SHARED)
else()
set(YAAF_LIB_MODE STATIC)
endif()

set(YAAF_CONFIG_HEADER_DIR ${PROJECT_BINARY_DIR}/build_config)
set(YAAF_CONFIG_FILE "${YAAF_CONFIG_HEADER_DIR}/YAAF_Config.h")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/YAAF_Config.cmake"
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
  src/YAAF_Hash.h
  src/YAAF_Hash_xxhash.h
)


set(YAAF_SRC
  src/YAAF_Archive.c
  src/YAAF_File.c
  src/YAAF_Internal.c
  src/YAAF_MemFile.c
  src/YAAF_TLS.c
  src/YAAF_Compression.c
  src/YAAF_Compression_lz4.c
  src/YAAF_Hash_xxhash.c
)

add_definitions("-DYAAF_BUILDING_LIBRARY")

add_library(${YAAF_LIB_NAME} ${YAAF_LIB_MODE} ${YAAF_SRC} ${YAAF_HDR} ${YAAF_INTERNAL_HDR})

#set soname
if(${YAAF_BUILD_SHARED_LIB})
set_target_properties(${YAAF_LIB_NAME} PROPERTIES SOVERSION ${YAAF_VERSION_MAJOR})
add_library(${YAAF_LIB_NAME}-internal STATIC ${YAAF_SRC})
set(YAAF_LIBRARIES_INTERNAL ${YAAF_LIB_NAME}-internal ${YAAF_LZ4_LIBRARY} PARENT_SCOPE)
endif()

set(YAAF_LIBRARIES ${YAAF_LIB_NAME} ${YAAF_LZ4_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

set(YAAF_LIBRARIES ${YAAF_LIBRARIES} PARENT_SCOPE)
set(YAAF_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include ${YAAF_CONFIG_HEADER_DIR} PARENT_SCOPE)
set(YAAF_INCLUDE_DIR_INTERNAL ${CMAKE_CURRENT_SOURCE_DIR}/src ${YAAF_LZ4_INCLUDE_DIR} PARENT_SCOPE)


target_link_libraries(${YAAF_LIB_NAME} ${YAAF_LZ4_LIBRARY})

add_dependencies(${YAAF_LIB_NAME} ${YAAF_LZ4_LIBRARY})

install(TARGETS ${YAAF_LIB_NAME} DESTINATION lib)
install(FILES ${YAAF_HDR} DESTINATION include/YAAF)
