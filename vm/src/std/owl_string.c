#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
