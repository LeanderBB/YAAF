################################################################################
# YAAFCL - CMD line tool
################################################################################


if(UNIX AND NOT APPLE AND CMAKE_COMPILER_IS_GNUCC)
add_definitions("-D_GNU_SOURCE")
endif()

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
target_link_libraries(yaafcl ${YAAF_LIBRARIES})

install(TARGETS yaafcl DESTINATION bin)
