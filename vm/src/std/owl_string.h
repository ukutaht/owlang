#include "term.h"


#ifndef OWL_STRING_H
#define OWL_STRING_H 1

#define owl_string_from(str) owl_tag_as(str, STRING)

owl_term owl_string_slice(owl_term string, owl_term from, owl_term to);
owl_term owl_string_concat(owl_term left, owl_term right);

#endif  // OWL_STRING_H
