cmake_minimum_required (VERSION 3.4)
project (owlang)

SET (CMAKE_C_FLAGS "-Wall -Wextra -pedantic -std=c99")
link_directories(lib/target/lib/)
include_directories(lib/target/include/)

INCLUDE (CheckIncludeFiles)
check_include_files (gc.h HAVE_GC)

if (NOT HAVE_GC)
  message(FATAL_ERROR "gc.h not found")
endif()

include_directories ("${PROJECT_SOURCE_DIR}/src")
add_executable(vm src/main.c src/vm.c src/opcodes.c src/term.c src/alloc.c src/util/file.c src/std/owl_list.c src/std/owl_file.c src/std/owl_string.c src/std/owl_code.c)
target_link_libraries(vm gc rrb intern /Users/ukutaht/dev/owlang/compiler/target/debug/libowlc.a)
