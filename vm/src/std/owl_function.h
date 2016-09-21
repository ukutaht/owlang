#include <stdint.h>

#ifndef OWL_FUNCTION_H
#define OWL_FUNCTION_H 1

#define owl_function_from(val) owl_tag_as(val, FUNCTION)
#define owl_term_to_function(term) ((Function*) (term >> 3))

typedef struct Function {
  uint64_t location;
  const char* name;
  owl_term upvalues[128];
} Function;

Function* owl_function_init(const char* name, uint64_t location);
Function* owl_anon_function_init(uint64_t location);
void owl_function_set_upvalue(Function* fun, uint8_t index, owl_term value);
owl_term owl_function_get_upvalue(Function* fun, uint8_t index);
owl_term owl_function_name(owl_term function);

#endif  // OWL_FUNCTION_H
