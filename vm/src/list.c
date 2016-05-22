#include <rrb/rrb.h>
#include "list.h"
#include "vm.h"

#define rrb_to_list(rrb) ((((owl_term) rrb) << 3) | LIST)
#define list_to_rrb(list) ((RRB*) owl_extract_ptr(list))

owl_term list_init() {
  const RRB *rrb = rrb_create();
  return rrb_to_list(rrb);
}

owl_term list_push(owl_term list, owl_term elem) {
  const RRB *rrb = list_to_rrb(list);
  rrb = rrb_push(rrb, (void*) elem);
  return rrb_to_list(rrb);
}

bool list_eq(owl_term left_list, owl_term right_list) {
  const RRB *left = list_to_rrb(left_list);
  const RRB *right = list_to_rrb(right_list);

  uint32_t left_count = rrb_count(left);
  uint32_t right_count = rrb_count(right);

  if (left_count != right_count) return false;

  for (uint32_t i = 0; i < left_count; i++) {
    owl_term left_elem  = (owl_term) rrb_nth(left, i);
    owl_term right_elem = (owl_term) rrb_nth(right, i);

    if (!owl_terms_eq(left_elem, right_elem)) return false;
  }

  return true;
}
