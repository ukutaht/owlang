#include <rrb.h>
#include "vector.h"
#include "vm.h"

#define rrb_to_vec(rrb) ((((owl_term) rrb) << 3) | VECTOR)
#define vec_to_rrb(vec) ((RRB*) owl_extract_ptr(vec))

owl_term vector_init() {
  const RRB *rrb = rrb_create();
  return rrb_to_vec(rrb);
}

owl_term vector_push(owl_term vec, owl_term elem) {
  const RRB *rrb = vec_to_rrb(vec);
  rrb = rrb_push(rrb, (void*) elem);
  return rrb_to_vec(rrb);
}

bool vector_eq(owl_term left_vec, owl_term right_vec) {
  if (left_vec == right_vec) return true;

  const RRB *left = vec_to_rrb(left_vec);
  const RRB *right = vec_to_rrb(right_vec);

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
