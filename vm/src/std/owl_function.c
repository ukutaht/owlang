#include "term.h"
#include "alloc.h"
#include "std/owl_function.h"
#include "std/owl_string.h"

Function* owl_function_init(const char* name, uint64_t location) {
  Function* function = malloc(sizeof(Function));
  function->location = location;
  function->name = name;

  return function;
}

Function* owl_anon_function_init(uint64_t location) {
  // Anonymous functions are subject to garbage collection, hence using `owl_alloc`
  Function* function = owl_alloc(sizeof(Function));
  function->location = location;
  function->name = "Anonymous";

  return function;
}

owl_term owl_function_name(owl_term function) {
  Function* fun = owl_term_to_function(function);
  return owl_string_from(fun->name);
}
