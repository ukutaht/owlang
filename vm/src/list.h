#include <stdbool.h>

#include "term.h"

#ifndef LIST_H
#define LIST_H 1

owl_term list_init();
void list_print(owl_term list);
owl_term list_push(owl_term list, owl_term elem);
owl_term list_nth(owl_term list, owl_term index);
bool list_eq(owl_term left, owl_term right);

#endif  // LIST_H
