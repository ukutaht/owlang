#include <stdint.h>
#include <stdbool.h>

#ifndef TERM_H
#define TERM_H 1

typedef uint64_t owl_term;

// pointer: 000
// int:     001
// tuple:   010
// vector:  011
typedef enum owl_tag {
    POINTER = 0,
    INT,
    TUPLE,
    VECTOR
} owl_tag;


owl_tag owl_tag_of(owl_term term);

owl_term* owl_extract_ptr(owl_term val);

owl_term owl_int_from(uint64_t val);
uint64_t int_from_owl_int(owl_term val);

owl_term owl_tuple_nth(owl_term tuple, uint8_t index);
bool owl_terms_eq(owl_term left, owl_term right);

#endif  // TERM_H
