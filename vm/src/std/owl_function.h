#include <stdint.h>

#ifndef OWL_FUNCTION_H
#define OWL_FUNCTION_H 1

#define owl_function_from(val) owl_tag_as(val, FUNCTION)
#define owl_term_to_function(term) ((Function*) (term >> 3))

typedef struct Function {
  uint64_t location;
  const char* name;
} Function;

Function* owl_function_init(const char* name, uint64_t location);
owl_term owl_function_name(owl_term function);

#endif  // OWL_FUNCTION_H
