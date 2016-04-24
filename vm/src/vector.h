#include <stdbool.h>

#include "term.h"

#ifndef VECTOR_H
#define VECTOR_H 1

owl_term vector_init();
owl_term vector_push(owl_term vec, owl_term elem);
bool vector_eq(owl_term left_vec, owl_term right_vec);

#endif  // VECTOR_H
