#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "alloc.h"
#include "term.h"
#include "std/owl_list.h"
#include "std/owl_string.h"

#define INT_MAX_DIGITS 20

owl_term owl_concat(owl_term left, owl_term right) {
  owl_tag left_tag = owl_tag_of(left);
  owl_tag right_tag = owl_tag_of(right);

  if (left_tag != right_tag) {
    puts("Type error in concat");
    exit(1);
  }

  if (left_tag == STRING) {
    return owl_string_concat(left, right);
  } else if (left_tag == LIST) {
    return owl_list_concat(left, right);
  } else {
    puts("Concat not defined for this type");
    exit(1);
  }
}

owl_term owl_tuple_nth(owl_term tuple, uint8_t index) {
  owl_term *ary = owl_extract_ptr(tuple);
  return ary[index + 1];
}

owl_term owl_type_of(owl_term term) {
  switch(owl_tag_of(term)) {
    case POINTER:
      return owl_string_from("Pointer");
    case INT:
      return owl_string_from("Int");
    case TUPLE:
      return owl_string_from("Tuple");
    case LIST:
      return owl_string_from("List");
    case STRING:
      return owl_string_from("String");
    case FUNCTION:
      return owl_string_from("Function");
    default:
      return owl_string_from("Unknown");
  }
}

owl_term owl_term_to_string(owl_term term) {
  switch(term) {
  case OWL_TRUE:
    return owl_string_from("true");
  case OWL_FALSE:
    return owl_string_from("false");
  case OWL_NIL:
    return owl_string_from("nil");
  }

  switch(owl_tag_of(term)) {
    case INT:
    {
      char *buf = owl_alloc(INT_MAX_DIGITS + 1);
      sprintf(buf, "%llu", int_from_owl_int(term));
      return owl_string_from(buf);
    }
    case TUPLE:
    {
      owl_term buffer = owl_string_from("");

      owl_term *ary = owl_extract_ptr(term);
      uint8_t size = ary[0];
      for(uint8_t i = 1; i <= size; i++) {
        buffer = owl_concat(buffer, owl_term_to_string(ary[i]));
      }
      return buffer;
    }
    case LIST:
    {
      owl_term buffer = owl_string_from("");
      uint64_t count = int_from_owl_int(owl_list_count(term));
      for(uint64_t i = 0; i < count; i++) {
        buffer = owl_string_concat(buffer, owl_term_to_string(owl_list_nth(term, owl_int_from(i))));
      }
      return buffer;
    }
    case STRING:
      return term;
    default:
      puts("Unable to convert to string");
      exit(1);
  }
}

bool owl_terms_eq(owl_term left, owl_term right) {
  // This cathes booleans, nils, ints and interned strings all at once
  if (left == right) return true;

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
    case LIST:
      return owl_list_eq(left, right);
    case STRING: // Comparing non-interned strings
    {
      const char *left_str = owl_extract_ptr(left);
      const char *right_str = owl_extract_ptr(right);

      return strcmp(left_str, right_str) == 0;
    }
    default:
      return false;
  }
}

void owl_term_print(owl_term term) {
  owl_term string = owl_term_to_string(term);
  fputs(owl_extract_ptr(string), stdout);
}
