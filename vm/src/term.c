#include <stdint.h>
#include <stdarg.h>

#include "term.h"

owl_tag owl_tag_of(owl_term term) {
  return term & 0xf;
}

owl_term* owl_extract_ptr(owl_term val) {
  return (owl_term*) int_from_owl_int(val);
}

owl_term owl_int_from(uint64_t val) {
  return (val << 3) | INT;
}

uint64_t int_from_owl_int(owl_term val) {
  return val >> 3;
}
