#ifndef OWL_LIST_H
#define OWL_LIST_H

#include <stdbool.h>
#include "owl.h"
#include <stdint.h>

#define rrb_to_list(rrb) owl_tag_as(rrb, LIST)
#define list_to_rrb(list) ((RRB*) owl_extract_ptr(list))

#define RRB_BITS 5
#define RRB_MAX_HEIGHT 6

#define RRB_BRANCHING (1 << RRB_BITS)
#define RRB_MASK (RRB_BRANCHING - 1)

#define RRB_INVARIANT 1
#define RRB_EXTRAS 2

#define GUID_DECLARATION const void *guid;

typedef enum {LEAF_NODE, INTERNAL_NODE} NodeType;

typedef struct TreeNode {
  NodeType type;
  uint32_t len;
  GUID_DECLARATION
} TreeNode;

typedef struct LeafNode {
  NodeType type;
  uint32_t len;
  GUID_DECLARATION
  const void *child[];
} LeafNode;

typedef struct RRBSizeTable {
  GUID_DECLARATION
  uint32_t size[];
} RRBSizeTable;

typedef struct InternalNode {
  NodeType type;
  uint32_t len;
  GUID_DECLARATION
  RRBSizeTable *size_table;
  struct InternalNode *child[];
} InternalNode;

typedef struct RRB {
  uint32_t cnt;
  uint32_t shift;
  uint32_t tail_len;
  LeafNode *tail;
  TreeNode *root;
} RRB;

owl_term owl_list_init();
void owl_list_print(vm_t *vm, owl_term list);
owl_term owl_list_push(vm_t *vm, owl_term list, owl_term elem);
owl_term owl_list_nth(owl_term list, owl_term index);
owl_term owl_list_count(owl_term list);
owl_term owl_list_slice(vm_t *vm, owl_term list, owl_term from, owl_term to);
owl_term owl_list_concat(vm_t *vm, owl_term left, owl_term right);
bool owl_list_eq(owl_term left, owl_term right);

#endif  // OWL_LIST_H
