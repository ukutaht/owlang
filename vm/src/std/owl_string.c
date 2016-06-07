#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "std/owl_string.h"
#include "term.h"
#include "alloc.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

owl_term owl_string_slice(owl_term string, owl_term from, owl_term to) {
  char *the_string = owl_extract_ptr(string);

  int from_int = int_from_owl_int(from);
  // Cap to make sure we don't go past the end
  int to_int = MIN(int_from_owl_int(to), strlen(the_string));
  int slice_size = to_int - from_int;

  if (slice_size < 1) {
    printf("Slice size must be more over 0");
    exit(1);
  }
  char *sliced = owl_alloc(slice_size);
  memcpy(sliced, the_string + from_int, slice_size);

  return owl_string_from(sliced);
}

owl_term owl_string_concat(owl_term left, owl_term right) {
  const char *left_str = owl_extract_ptr(left);
  const char *right_str = owl_extract_ptr(right);

  size_t left_len = strlen(left_str);
  size_t total_len = left_len + strlen(right_str);
  char *result = owl_alloc(total_len);
  strcpy(result, left_str);
  strcpy(result + left_len, right_str);

  return owl_string_from(result);
}

owl_term owl_string_count(owl_term string) {
  const char *str = owl_extract_ptr(string);

  return owl_int_from(strlen(str));
}

owl_term owl_string_contains(owl_term string, owl_term substr) {
  const char *str = owl_extract_ptr(string);
  const char *subs = owl_extract_ptr(substr);

  if (strstr(str, subs) != NULL) {
    return OWL_TRUE;
  } else {
    return OWL_FALSE;
  }
}
