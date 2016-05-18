#include <stdbool.h>

#include "term.h"

#ifndef LIST_H
#define LIST_H 1

owl_term list_init();
owl_term list_push(owl_term list, owl_term elem);
bool list_eq(owl_term left, owl_term right);

#endif  // LIST_H
