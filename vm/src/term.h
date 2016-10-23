#ifndef TERM_H
#define TERM_H

#include "owl.h"

#define owl_extract_ptr(term) ((void*) (term >> 3))
#define owl_tag_of(term) ((owl_tag) (term & 0x7))
#define owl_tag_as(term, tag) ((((owl_term) term) << 3) | tag)

#define owl_int_from(val) owl_tag_as(val, INT)
#define int_from_owl_int(term) (term >> 3)

#define owl_term_falsey(term) (term == OWL_FALSE || term == OWL_NIL)
#define owl_term_truthy(term) (!owl_term_falsey(term))
#define owl_negate(term) (owl_term_truthy(term) ? OWL_FALSE : OWL_TRUE)
#define owl_bool(val) (val ? OWL_TRUE : OWL_FALSE)

/*
Using concrete values for true, false and nil. Abusing the fact that low memory
addressses are reserved for kernel stuff. We just don't expect any pointers to
have memory addresses 0, 1, 2 etc. The values grow in increments of two because
in owl terms the lowest bits are reserved for tagging. By making these divisible
by two, they look like pointers when the vm inspects the tag. This ensures that
values like FALSE and TRUE cannot be confused with integers etc.
*/
#define OWL_FALSE 2
#define OWL_TRUE  4
#define OWL_NIL  6

owl_term owl_concat(vm_t *vm, owl_term left, owl_term right);
owl_term owl_term_to_string(vm_t *vm, owl_term term);
void owl_term_print(vm_t *vm, owl_term term);
bool owl_terms_eq(owl_term left, owl_term right);
owl_term owl_type_of(owl_term term);
owl_term owl_tuple_nth(owl_term tuple, uint8_t index);

#endif  // TERM_H
