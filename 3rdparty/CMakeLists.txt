################################################################################
# 3rdparty libraries used by YAAF
################################################################################

include(ExternalProject)


if(${YAAF_USE_HASH_XXHASH} EQUAL 1 OR ${YAAF_USE_COMPRESSION_LZ4} EQUAL 1)

if(${YAAF_BUILD_SHARED_LIB})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${YAAF_SHARED_C_FLAGS}")
endif()

set(YAAF_LZ4_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lib PARENT_SCOPE)
if(${YAAF_USE_COMPRESSION_LZ4} EQUAL 1)
set(YAAF_LZ4_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lib/lz4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lib/lz4hc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lib/xxhash.c
)
else()
set(YAAF_LZ4_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/lz4/xxhash.c
)
endif()

set(YAAF_LZ4_LIBRARY YAAF-lz4-xxhash PARENT_SCOPE)
add_library(YAAF-lz4-xxhash ${YAAF_LZ4_SRC})

install(TARGETS YAAF-lz4-xxhash DESTINATION lib)

endif()

