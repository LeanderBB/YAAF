################################################################################
# YAAF - Test programs
################################################################################
include_directories(${YAAF_INCLUDE_DIR})
include_directories(${YAAF_INCLUDE_DIR_INTERNAL})
add_executable(YAAF_TestCompression YAAF_TestCompression.c)
target_link_libraries(YAAF_TestCompression ${YAAF_LIBRARIES})

add_executable(YAAF_TestHashMap YAAF_TestHashMap.c)
target_link_libraries(YAAF_TestHashMap ${YAAF_LIBRARIES})
