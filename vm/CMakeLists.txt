cmake_minimum_required (VERSION 3.4)
project (owlang)

SET (CMAKE_C_FLAGS "-Wall -Wextra -pedantic -std=c99 -fsanitize=address")
link_directories(lib/target/lib/)
include_directories(lib/target/include/)

include_directories ("${PROJECT_SOURCE_DIR}/src")
add_executable(vm src/main.c src/vm.c src/opcodes.c src/term.c src/alloc.c src/util/file.c src/std/owl_list.c src/std/owl_file.c src/std/owl_string.c src/std/owl_code.c src/std/owl_function.c src/stack.c)
target_link_libraries(vm intern /Users/ukutaht/dev/owlang/compiler/target/debug/libowlc.a)
