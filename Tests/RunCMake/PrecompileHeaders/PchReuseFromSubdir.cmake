set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

enable_language(C)

add_library(empty empty.c)
target_precompile_headers(empty PUBLIC
  <stdio.h>
  <string.h>
)
target_include_directories(empty PUBLIC include)

add_library(foo foo.c)
target_include_directories(foo PUBLIC include)
target_precompile_headers(foo REUSE_FROM empty)

subdirs(subdir)

enable_testing()
add_test(NAME foobar COMMAND foobar)
