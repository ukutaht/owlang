#include <stdint.h>

#include "term.h"
#include "vector.h"

owl_tag owl_tag_of(owl_term term) {
  return term & 0x7;
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

owl_term owl_tuple_nth(owl_term tuple, uint8_t index) {
  owl_term *ary = owl_extract_ptr(tuple);
  return ary[index + 1];
}

bool owl_terms_eq(owl_term left, owl_term right) {
  owl_tag left_tag  = owl_tag_of(left);
  owl_tag right_tag = owl_tag_of(right);

  if (left_tag != right_tag) {
    return false;
  }

  switch(left_tag) {
    case INT:
      return left == right;
    case TUPLE:
    {
      owl_term *left_ary = owl_extract_ptr(left);
      owl_term *right_ary = owl_extract_ptr(right);

      uint8_t left_size = left_ary[0];
      uint8_t right_size = right_ary[0];

      if (left_size != right_size) {
        return false;
      }

      for(uint8_t i = 1; i <= left_size; i++) {
        if (!owl_terms_eq(left_ary[i], right_ary[i])) {
          return false;
        }
      }
      return true;
    }
    case VECTOR:
      return vector_eq(left, right);
    default:
      return false;
  }
}
