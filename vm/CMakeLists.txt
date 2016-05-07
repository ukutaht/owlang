cmake_minimum_required (VERSION 3.4)
project (owlang)

SET (CMAKE_C_FLAGS "-Wall -Wextra -pedantic -std=c99")

INCLUDE (CheckIncludeFiles)
check_include_files (gc.h HAVE_GC)
check_include_files (intern/strings.h HAVE_STRING_INTERNS)
check_include_files (rrb.h HAVE_STRING_INTERNS)

include_directories ("${PROJECT_SOURCE_DIR}/src")
add_executable(vm src/main.c src/vm.c src/opcodes.c src/term.c src/alloc.c src/vector.c src/util/file.c)
target_link_libraries(vm gc rrb intern)
