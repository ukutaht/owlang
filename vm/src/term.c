#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "alloc.h"
#include "term.h"
#include "std/owl_list.h"


owl_term owl_int_from(uint64_t val) {
  return (val << 3) | INT;
}

owl_term owl_string_from(const char* val) {
  return ((uint64_t) val << 3) | STRING;
}

owl_term owl_function_from(uint64_t instruction) {
  return (instruction << 3) | FUNCTION;
}

owl_term owl_concat(owl_term left, owl_term right) {
  owl_tag left_tag = owl_tag_of(left);
  owl_tag right_tag = owl_tag_of(right);

  if (left_tag != right_tag) {
    puts("Type error in concat");
    exit(1);
  }

  if (left_tag == STRING) {
    const char *left_str = owl_extract_ptr(left);
    const char *right_str = owl_extract_ptr(right);

    size_t left_len = strlen(left_str);
    size_t total_len = left_len + strlen(right_str);
    char *result = owl_alloc(total_len);
    strcpy(result, left_str);
    strcpy(result + left_len, right_str);

    return owl_string_from(result);
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
  switch(term) {
    case OWL_TRUE:
      puts("true");
      return;
    case OWL_FALSE:
      puts("false");
      return;
    case OWL_NIL:
      puts("nil");
      return;
  }

  switch(owl_tag_of(term)) {
    case INT:
      printf("%llu\n", int_from_owl_int(term));
      return;
    case TUPLE:
    {
      owl_term *ary = owl_extract_ptr(term);
      uint8_t size = ary[0];
      for(uint8_t i = 1; i <= size; i++) {
        owl_term_print(ary[i]);
      }
      return;
    }
    case LIST:
    {
      owl_list_print(term);
      return;
    }
    case STRING:
      printf("\"");
      printf("%s", (char*) owl_extract_ptr(term));
      printf("\"");
      return;
    default:
      return;
  }
}
