#ifndef OWL_CODE_H
#define OWL_CODE_H 1

/*
 Defined in compiler/src/compiler.rs. These functions are
 exported using rust's ffi library to be used at runtime.
*/

typedef struct compiled_module_t {
  size_t size;
  uint8_t *bytecode;
} compiled_module_t;

compiled_module_t* compile_file_to_memory(const char *input);
void free_module(compiled_module_t *module);

#include "term.h"

owl_term owl_code_load(vm_t *vm, owl_term filename);
owl_term owl_load_module(vm_t *vm, uint8_t *bytecode, size_t size);

#endif  // OWL_CODE_H
