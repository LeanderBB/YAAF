################################################################################
# 3rdparty libraries used by YAAF
################################################################################

include(ExternalProject)


if(${YAAF_USE_HASH_XXHASH} EQUAL 1 OR ${YAAF_USE_COMPRESSION_LZ4} EQUAL 1)

if(${YAAF_BUILD_SHARED_LIB})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${YAAF_SHARED_C_FLAGS}")
endif()

set(YAAF_LZ4_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lz4 PARENT_SCOPE)
if(${YAAF_USE_COMPRESSION_LZ4} EQUAL 1)
set(YAAF_LZ4_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lz4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lz4hc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/xxhash.c
)
else()
set(YAAF_LZ4_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/xxhash.c
)
endif()

set(YAAF_LZ4_LIBRARY lz4-xxhash-yaaf PARENT_SCOPE)
add_library(lz4-xxhash-yaaf ${YAAF_LZ4_SRC})

install(TARGETS ${YAAF_LZ4_LIBRARY} DESTINATION lib)

endif()

