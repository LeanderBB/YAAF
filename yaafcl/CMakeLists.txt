################################################################################
# YAAFCL - CMD line tool
################################################################################


if(UNIX AND NOT APPLE AND CMAKE_COMPILER_IS_GNUCC)
add_definitions("-D_GNU_SOURCE")
endif()

if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    if((UNIX) AND (NOT APPLE))
        add_definitions("-D_GNU_SOURCE")
    endif()
endif()

include_directories(${YAAF_INCLUDE_DIR} ${YAAF_INCLUDE_DIR_INTERNAL})

add_executable(yaafcl
  YAAFCL.c
  YAAFCL.h
  YAAFCL_DirUtils.c
  YAAFCL_DirUtils.h
  YAAFCL_StrUtil.c
  YAAFCL_StrUtil.h
  YAAFCL_Job.h
  YAAFCL_Job.c
)
target_link_libraries(yaafcl ${YAAF_LIBRARIES} ${YAAF_LIBRARIES_INTERNAL})

install(TARGETS yaafcl DESTINATION bin)
