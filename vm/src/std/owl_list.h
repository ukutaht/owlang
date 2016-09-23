#ifndef OWL_LIST_H
#define OWL_LIST_H

#include <stdbool.h>
#include "term.h"

owl_term owl_list_init();
void owl_list_print(owl_term list);
owl_term owl_list_push(owl_term list, owl_term elem);
owl_term owl_list_nth(owl_term list, owl_term index);
owl_term owl_list_count(owl_term list);
owl_term owl_list_slice(owl_term list, owl_term from, owl_term to);
owl_term owl_list_concat(owl_term left, owl_term right);
bool owl_list_eq(owl_term left, owl_term right);

#endif  // OWL_LIST_H
