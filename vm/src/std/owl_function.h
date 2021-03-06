#ifndef OWL_FUNCTION_H
#define OWL_FUNCTION_H

#include "owl.h"

#define owl_function_from(val) owl_tag_as(val, FUNCTION)
#define owl_term_to_function(term) ((Function*) (term >> 3))

Function* owl_function_init(const char* name, uint64_t location);
Function* owl_anon_function_init(vm_t *vm, uint64_t location, uint8_t n_upvalues);
void owl_function_set_upvalue(Function* fun, uint8_t index, owl_term value);
owl_term owl_function_get_upvalue(Function* fun, uint8_t index);
owl_term owl_function_name(owl_term function);

#endif  // OWL_FUNCTION_H
