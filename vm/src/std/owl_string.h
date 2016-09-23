#ifndef OWL_STRING_H
#define OWL_STRING_H

#include "term.h"

#define owl_string_from(str) owl_tag_as(str, STRING)

owl_term owl_string_slice(owl_term string, owl_term from, owl_term to);
owl_term owl_string_concat(owl_term left, owl_term right);
owl_term owl_string_count(owl_term string);
owl_term owl_string_contains(owl_term string, owl_term substr);

#endif  // OWL_STRING_H
