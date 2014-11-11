################################################################################
# 3rdparty libraries used by YAAF
################################################################################

set(YAAF_LZ4_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lz4 PARENT_SCOPE)
set(YAAF_LZ4_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lz4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lz4hc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lz4frame.c
)
set(YAAFL_LZ4_LIBRARY lz4-yaaf PARENT_SCOPE)
add_library(lz4-yaaf ${YAAF_LZ4_SRC})

