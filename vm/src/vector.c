#include <rrb.h>
#include "vector.h"
#include "vm.h"

owl_term vector_init() {
  const RRB *rrb = rrb_create();
  owl_term vector = (owl_term) rrb;

  return (vector << 3) | VECTOR;
}

owl_term vector_push(owl_term vec, owl_term elem) {
  const RRB *rrb = (RRB*) owl_extract_ptr(vec);
  owl_term modified = (owl_term) rrb_push(rrb, (void*) elem);

  return (modified << 3) | VECTOR;
}

bool vector_eq(owl_term left_vec, owl_term right_vec) {
  if (left_vec == right_vec) return true;

  const RRB *left = (RRB*) owl_extract_ptr(left_vec);
  const RRB *right = (RRB*) owl_extract_ptr(right_vec);

  uint32_t left_count = rrb_count(left);
  uint32_t right_count = rrb_count(right);

  if (left_count != right_count) return false;

  for (uint32_t i = 0; i < left_count; i++) {
    owl_term left_elem  = (owl_term) rrb_nth(left, i);
    owl_term right_elem = (owl_term) rrb_nth(left, i);

    if (!owl_terms_eq(left_elem, right_elem)) return false;
  }

  return true;
}
