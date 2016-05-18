#include <stdint.h>

#include "term.h"
#include "list.h"


owl_term owl_int_from(uint64_t val) {
  return (val << 3) | INT;
}

owl_term owl_bool(bool val) {
  if (val) {
    return OWL_TRUE;
  } else {
    return OWL_FALSE;
  }
}

owl_term owl_tuple_nth(owl_term tuple, uint8_t index) {
  owl_term *ary = owl_extract_ptr(tuple);
  return ary[index + 1];
}

owl_term owl_negate(owl_term value) {
  if (owl_term_truthy(value)) {
    return OWL_FALSE;
  } else {
    return OWL_TRUE;
  }
}

bool owl_terms_eq(owl_term left, owl_term right) {
  if (left == right) return true;

  owl_tag left_tag  = owl_tag_of(left);
  owl_tag right_tag = owl_tag_of(right);

  if (left_tag != right_tag) {
    return false;
  }

  // Booleans need not to be handled here because they are caught by strict equality earlier
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
    case LIST:
      return list_eq(left, right);
    default:
      return false;
  }
}
