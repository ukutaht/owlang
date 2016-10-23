#include <stdlib.h>
#include "term.h"
#include "alloc.h"
#include "std/owl_function.h"
#include "std/owl_string.h"

Function* owl_function_init(const char* name, uint64_t location) {
  Function* function = malloc(sizeof(Function));
  function->location = location;
  function->name = name;
  function->n_upvalues = 0;

  return function;
}

Function* owl_anon_function_init(vm_t *vm, uint64_t location, uint8_t n_upvalues) {
  // Anonymous functions are subject to garbage collection, hence using `owl_alloc`
  Function* function = owl_alloc(vm, sizeof(Function) + n_upvalues * sizeof(owl_term));
  function->location = location;
  function->name = "Anonymous";
  function->n_upvalues = n_upvalues;

  return function;
}

void owl_function_set_upvalue(Function* fun, uint8_t index, owl_term value) {
  fun->upvalues[index] = value;
}

owl_term owl_function_get_upvalue(Function* fun, uint8_t index) {
  return fun->upvalues[index];
}

owl_term owl_function_name(owl_term function) {
  Function* fun = owl_term_to_function(function);
  return owl_string_from(fun->name);
}
