#include "term.h"
#include "alloc.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "std/owl_list.h"

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

// Typical stuff
#define RRB_SHIFT(rrb) (rrb->shift)
#define INC_SHIFT(shift) (shift + (uint32_t) RRB_BITS)
#define DEC_SHIFT(shift) (shift - (uint32_t) RRB_BITS)
#define LEAF_NODE_SHIFT ((uint32_t) 0)

// Abusing allocated pointers being unique to create GUIDs: using a single
// malloc to create a guid.

static LeafNode EMPTY_LEAF = {.type = LEAF_NODE, .len = 0};
static const RRB EMPTY_RRB = {.cnt = 0, .shift = 0, .root = NULL,
                              .tail_len = 0, .tail = &EMPTY_LEAF};

static RRBSizeTable* size_table_create(vm_t *vm, uint32_t len);
static RRBSizeTable* size_table_clone(vm_t *vm, const RRBSizeTable* original, uint32_t len);
static RRBSizeTable* size_table_inc(vm_t *vm, const RRBSizeTable *original, uint32_t len);

static InternalNode* concat_sub_tree(vm_t *vm, TreeNode *left_node, uint32_t left_shift,
                                     TreeNode *right_node, uint32_t right_shift,
                                     char is_top);
static InternalNode* rebalance(vm_t *vm, InternalNode *left, InternalNode *centre,
                               InternalNode *right, uint32_t shift,
                               char is_top);
static uint32_t* create_concat_plan(vm_t *vm, InternalNode *all, uint32_t *top_len);
static InternalNode* execute_concat_plan(vm_t *vm, InternalNode *all, uint32_t *node_sizes,
                                         uint32_t slen, uint32_t shift);
static uint32_t find_shift(TreeNode *node);
static InternalNode* set_sizes(vm_t *vm, InternalNode *node, uint32_t shift);
static uint32_t size_sub_trie(TreeNode *node, uint32_t parent_shift);
static uint32_t sized_pos(const InternalNode *node, uint32_t *index,
                          uint32_t sp);
static const InternalNode* sized(const InternalNode *node, uint32_t *index,
                                 uint32_t sp);

static LeafNode* leaf_node_clone(vm_t *vm, const LeafNode *original);
static LeafNode* leaf_node_inc(vm_t *vm, const LeafNode *original);
static LeafNode* leaf_node_dec(vm_t *vm, const LeafNode *original);
static LeafNode* leaf_node_create(vm_t *vm, uint32_t size);
static LeafNode* leaf_node_merge(vm_t *vm, LeafNode *left_leaf, LeafNode *right_leaf);

static InternalNode* internal_node_create(vm_t *vm, uint32_t len);
static InternalNode* internal_node_clone(vm_t *vm, const InternalNode *original);
static InternalNode* internal_node_inc(vm_t *vm, const InternalNode *original);
static InternalNode* internal_node_dec(vm_t *vm, const InternalNode *original);
static InternalNode* internal_node_merge(vm_t *vm, InternalNode *left, InternalNode *centre,
                                         InternalNode *right);
static InternalNode* internal_node_copy(vm_t *vm, InternalNode *original, uint32_t start,
                                        uint32_t len);
static InternalNode* internal_node_new_above1(vm_t *vm, InternalNode *child);
static InternalNode* internal_node_new_above(vm_t *vm, InternalNode *left, InternalNode *right);

static RRB* slice_right(vm_t *vm, const RRB *rrb, const uint32_t right);
static TreeNode* slice_right_rec(vm_t *vm, uint32_t *total_shift, const TreeNode *root,
                                  uint32_t right, uint32_t shift,
                                  char has_left);
static const RRB* slice_left(vm_t *vm, RRB *rrb, uint32_t left);
static TreeNode* slice_left_rec(vm_t *vm, uint32_t *total_shift, const TreeNode *root,
                                uint32_t left, uint32_t shift,
                                char has_right);

static RRB* rrb_head_clone(vm_t *vm, const RRB *original);

static RRB* push_down_tail(vm_t *vm, const RRB *restrict rrb, RRB *restrict new_rrb,
                           LeafNode *restrict new_tail);
static void promote_rightmost_leaf(vm_t *vm, RRB *new_rrb);



static RRBSizeTable* size_table_create(vm_t* vm, uint32_t size) {
  RRBSizeTable *table = owl_alloc(vm, sizeof(RRBSizeTable) + size * sizeof(uint32_t));
  return table;
}

static RRBSizeTable* size_table_clone(vm_t *vm, const RRBSizeTable *original, uint32_t len) {
  RRBSizeTable *clone = owl_alloc(vm, sizeof(RRBSizeTable) + len * sizeof(uint32_t));
  memcpy(&clone->size, &original->size, sizeof(uint32_t) * len);
  return clone;
}

static inline RRBSizeTable* size_table_inc(vm_t *vm, const RRBSizeTable *original, uint32_t len) {
  RRBSizeTable *incr = owl_alloc(vm, sizeof(RRBSizeTable) + (len + 1) * sizeof(uint32_t));
  memcpy(&incr->size, &original->size, sizeof(uint32_t) * len);
  return incr;
}

static RRB* rrb_head_clone(vm_t *vm, const RRB* original) {
  RRB *clone = owl_alloc(vm, sizeof(RRB));
  memcpy(clone, original, sizeof(RRB));
  return clone;
}

const RRB* rrb_create() {
  return &EMPTY_RRB;
}

static RRB* rrb_mutable_create(vm_t *vm) {
  RRB *rrb = owl_alloc(vm, sizeof(RRB));
  return rrb;
}

const RRB* rrb_concat(vm_t *vm, const RRB *left, const RRB *right) {
  if (left->cnt == 0) {
    return right;
  }
  else if (right->cnt == 0) {
    return left;
  }
  else {
    if (right->root == NULL) {
      // merge left and right tail, if possible
      RRB *new_rrb = rrb_head_clone(vm, left);
      new_rrb->cnt += right->cnt;

      // skip merging if left tail is full.
      if (left->tail_len == RRB_BRANCHING) {
        new_rrb->tail_len = right->tail_len;
        return push_down_tail(vm, left, new_rrb, right->tail);
      }
      // We can merge both tails into a single tail.
      else if (left->tail_len + right->tail_len <= RRB_BRANCHING) {
        const uint32_t new_tail_len = left->tail_len + right->tail_len;
        LeafNode *new_tail = leaf_node_merge(vm, left->tail, right->tail);
        new_rrb->tail = new_tail;
        new_rrb->tail_len = new_tail_len;
        return new_rrb;
      }
      else { // must push down something, and will have elements remaining in
             // the right tail
        LeafNode *push_down = leaf_node_create(vm, RRB_BRANCHING);
        memcpy(&push_down->child[0], &left->tail->child[0],
               left->tail_len * sizeof(void *));
        const uint32_t right_cut = RRB_BRANCHING - left->tail_len;
        memcpy(&push_down->child[left->tail_len], &right->tail->child[0],
               right_cut * sizeof(void *));

        // this will be strictly positive.
        const uint32_t new_tail_len = right->tail_len - right_cut;
        LeafNode *new_tail = leaf_node_create(vm, new_tail_len);

        memcpy(&new_tail->child[0], &right->tail->child[right_cut],
               new_tail_len * sizeof(void *));

        new_rrb->tail = push_down;
        new_rrb->tail_len = new_tail_len;

        // This is an imitation, so that push_down_tail works as we intend it
        // to: Whenever the height has to be increased, it calculates the size
        // table based upon the old rrb's size, minus the old tail. However,
        // since we manipulate the old tail to be longer than it actually was,
        // we have to reflect those changes in the cnt variable.
        RRB left_imitation;
        memcpy(&left_imitation, left, sizeof(RRB));
        left_imitation.cnt = new_rrb->cnt - new_tail_len;

        return push_down_tail(vm, &left_imitation, new_rrb, new_tail);
      }
    }
    left = push_down_tail(vm, left, rrb_head_clone(vm, left), NULL);
    RRB *new_rrb = rrb_mutable_create(vm);
    new_rrb->cnt = left->cnt + right->cnt;

    InternalNode *root_candidate = concat_sub_tree(vm, left->root, RRB_SHIFT(left),
                                                   right->root, RRB_SHIFT(right),
                                                   true);

    new_rrb->shift = find_shift((TreeNode *) root_candidate);
    // must be done before we set sizes.
    new_rrb->root = (TreeNode *) set_sizes(vm, root_candidate, RRB_SHIFT(new_rrb));
    new_rrb->tail = right->tail;
    new_rrb->tail_len = right->tail_len;
    return new_rrb;
  }
}

static InternalNode* concat_sub_tree(vm_t *vm, TreeNode *left_node, uint32_t left_shift,
                                     TreeNode *right_node, uint32_t right_shift,
                                     char is_top) {
  if (left_shift > right_shift) {
    // Left tree is higher than right tree
    InternalNode *left_internal = (InternalNode *) left_node;
    InternalNode *centre_node =
      concat_sub_tree(vm, (TreeNode *) left_internal->child[left_internal->len - 1],
                      DEC_SHIFT(left_shift),
                      right_node, right_shift,
                      false);
    return rebalance(vm, left_internal, centre_node, NULL, left_shift, is_top);
  }
  else if (left_shift < right_shift) {
    InternalNode *right_internal = (InternalNode *) right_node;
    InternalNode *centre_node =
      concat_sub_tree(vm, left_node, left_shift,
                      (TreeNode *) right_internal->child[0],
                      DEC_SHIFT(right_shift),
                      false);
    return rebalance(vm, NULL, centre_node, right_internal, right_shift, is_top);
  }
  else { // we have same height
    if (left_shift == LEAF_NODE_SHIFT) { // We're dealing with leaf nodes
      LeafNode *left_leaf = (LeafNode *) left_node;
      LeafNode *right_leaf = (LeafNode *) right_node;
      // We don't do this if we're not at top, as we'd have to zip stuff above
      // as well.
      if (is_top && (left_leaf->len + right_leaf->len) <= RRB_BRANCHING) {
        // Can put them in a single node
        LeafNode *merged = leaf_node_merge(vm, left_leaf, right_leaf);
        return internal_node_new_above1(vm, (InternalNode *) merged);
      }
      else {
        InternalNode *left_internal = (InternalNode *) left_node;
        InternalNode *right_internal = (InternalNode *) right_node;
        return internal_node_new_above(vm, left_internal, right_internal);
      }
    }

    else { // two internal nodes with same height. Move both down
      InternalNode *left_internal = (InternalNode *) left_node;
      InternalNode *right_internal = (InternalNode *) right_node;
      InternalNode *centre_node =
        concat_sub_tree(vm, (TreeNode *) left_internal->child[left_internal->len - 1],
                        DEC_SHIFT(left_shift),
                        (TreeNode *) right_internal->child[0],
                        DEC_SHIFT(right_shift),
                        false);
      // can be optimised: since left_shift == right_shift, we'll end up in this
      // block again.
      return rebalance(vm, left_internal, centre_node, right_internal, left_shift,
                       is_top);
    }
  }
}

static LeafNode* leaf_node_clone(vm_t *vm, const LeafNode *original) {
  size_t size = sizeof(LeafNode) + original->len * sizeof(void *);
  LeafNode *clone = owl_alloc(vm, size);
  memcpy(clone, original, size);
  return clone;
}

static LeafNode* leaf_node_inc(vm_t *vm, const LeafNode *original) {
  size_t size = sizeof(LeafNode) + original->len * sizeof(void *);
  LeafNode *inc = owl_alloc(vm, size + sizeof(void *));
  memcpy(inc, original, size);
  inc->len++;
  return inc;
}

static LeafNode* leaf_node_dec(vm_t *vm, const LeafNode *original) {
  size_t size = sizeof(LeafNode) + (original->len - 1) * sizeof(void *);
  LeafNode *dec = owl_alloc(vm, size); // assumes size > 1
  memcpy(dec, original, size);
  dec->len--;
  return dec;
}


static LeafNode* leaf_node_create(vm_t *vm, uint32_t len) {
  LeafNode *node = owl_alloc(vm, sizeof(LeafNode) + len * sizeof(void *));
  node->type = LEAF_NODE;
  node->len = len;
  return node;
}

static LeafNode* leaf_node_merge(vm_t *vm, LeafNode *left, LeafNode *right) {
  LeafNode *merged = leaf_node_create(vm, left->len + right->len);

  memcpy(&merged->child[0], left->child, left->len * sizeof(void *));
  memcpy(&merged->child[left->len], right->child, right->len * sizeof(void *));
  return merged;
}

static InternalNode* internal_node_create(vm_t *vm, uint32_t len) {
  InternalNode *node = owl_alloc(vm, sizeof(InternalNode) + len * sizeof(InternalNode *));
  node->type = INTERNAL_NODE;
  node->len = len;
  node->size_table = NULL;
  return node;
}

static InternalNode* internal_node_new_above1(vm_t *vm, InternalNode *child) {
  InternalNode *above = internal_node_create(vm, 1);
  above->child[0] = child;
  return above;
}

static InternalNode* internal_node_new_above(vm_t *vm, InternalNode *left, InternalNode *right) {
  InternalNode *above = internal_node_create(vm, 2);
  above->child[0] = left;
  above->child[1] = right;
  return above;
}

static InternalNode* internal_node_merge(vm_t *vm, InternalNode *left, InternalNode *centre,
                                         InternalNode *right) {
  // If internal node is NULL, its size is zero.
  uint32_t left_len = (left == NULL) ? 0 : left->len - 1;
  uint32_t centre_len = (centre == NULL) ? 0 : centre->len;
  uint32_t right_len = (right == NULL) ? 0 : right->len - 1;

  InternalNode *merged = internal_node_create(vm, left_len + centre_len + right_len);
  if (left_len != 0) { // memcpy'ing zero elements from/to NULL is undefined.
    memcpy(&merged->child[0], left->child,
           left_len * sizeof(InternalNode *));
  }
  if (centre_len != 0) { // same goes here
    memcpy(&merged->child[left_len], centre->child,
           centre_len * sizeof(InternalNode *));
  }
  if (right_len != 0) { // and here
    memcpy(&merged->child[left_len + centre_len], &right->child[1],
           right_len * sizeof(InternalNode *));
  }

  return merged;
}

static InternalNode* internal_node_clone(vm_t *vm, const InternalNode *original) {
  size_t size = sizeof(InternalNode) + original->len * sizeof(InternalNode *);
  InternalNode *clone = owl_alloc(vm, size);
  memcpy(clone, original, size);
  return clone;
}

static InternalNode* internal_node_copy(vm_t *vm, InternalNode *original, uint32_t start,
                                        uint32_t len){
  InternalNode *copy = internal_node_create(vm, len);
  memcpy(copy->child, &original->child[start], len * sizeof(InternalNode *));
  return copy;
}

static InternalNode* internal_node_inc(vm_t *vm, const InternalNode *original) {
  size_t size = sizeof(InternalNode) + original->len * sizeof(InternalNode *);
  InternalNode *incr = owl_alloc(vm, size + sizeof(InternalNode *));
  memcpy(incr, original, size);
  // update length
  if (incr->size_table != NULL) {
    incr->size_table = size_table_inc(vm, incr->size_table, incr->len);
  }
  incr->len++;
  return incr;
}

static InternalNode* internal_node_dec(vm_t *vm, const InternalNode *original) {
  size_t size = sizeof(InternalNode) + (original->len - 1) * sizeof(InternalNode *);
  InternalNode *clone = owl_alloc(vm, size);
  memcpy(clone, original, size);
  // update length
  clone->len--;
  // Leaks the size table, but it's okay: Would cost more to actually make a
  // smaller one as its size would be roughly the same size.
  return clone;
}


static InternalNode* rebalance(vm_t *vm, InternalNode *left, InternalNode *centre,
                               InternalNode *right, uint32_t shift,
                               char is_top) {
  InternalNode *all = internal_node_merge(vm, left, centre, right);
  // top_len is children count of the internal node returned.
  uint32_t top_len; // populated through pointer manipulation.

  uint32_t *node_count = create_concat_plan(vm, all, &top_len);

  InternalNode *new_all = execute_concat_plan(vm, all, node_count, top_len, shift);
  if (top_len <= RRB_BRANCHING) {
    if (is_top == false) {
      return internal_node_new_above1(vm, set_sizes(vm, new_all, shift));
    }
    else {
      return new_all;
    }
  }
  else {
    InternalNode *new_left = internal_node_copy(vm, new_all, 0, RRB_BRANCHING);
    InternalNode *new_right = internal_node_copy(vm, new_all, RRB_BRANCHING,
                                                 top_len - RRB_BRANCHING);
    return internal_node_new_above(vm, set_sizes(vm, new_left, shift),
                                   set_sizes(vm, new_right, shift));
  }
}

/**
 * create_concat_plan takes in the large concatenated internal node and a
 * pointer to an uint32_t, which will contain the reduced size of the rebalanced
 * node. It returns a plan as an array of uint32_t's, and modifies the input
 * pointer to contain the length of said array.
 */

static uint32_t* create_concat_plan(vm_t *vm, InternalNode *all, uint32_t *top_len) {
  uint32_t *node_count = owl_alloc(vm, all->len * sizeof(uint32_t));

  uint32_t total_nodes = 0;
  for (uint32_t i = 0; i < all->len; i++) {
    const uint32_t size = all->child[i]->len;
    node_count[i] = size;
    total_nodes += size;
  }

  const uint32_t optimal_slots = ((total_nodes-1) / RRB_BRANCHING) + 1;

  uint32_t shuffled_len = all->len;
  uint32_t i = 0;
  while (optimal_slots + RRB_EXTRAS < shuffled_len) {

    // Skip over all nodes satisfying the invariant.
    while (node_count[i] > RRB_BRANCHING - RRB_INVARIANT) {
      i++;
    }

    // Found short node, so redistribute over the next nodes
    uint32_t remaining_nodes = node_count[i];
    do {
      const uint32_t min_size = MIN(remaining_nodes + node_count[i+1], RRB_BRANCHING);
      node_count[i] = min_size;
      remaining_nodes = remaining_nodes + node_count[i+1] - min_size;
      i++;
    } while (remaining_nodes > 0);

    // Shuffle up remaining node sizes
    for (uint32_t j = i; j < shuffled_len - 1; j++) {
      node_count[j] = node_count[j+1]; // Could use memmove here I guess
    }
    shuffled_len--;
    i--;
  }

  *top_len = shuffled_len;
  return node_count;
}

static InternalNode* execute_concat_plan(vm_t *vm, InternalNode *all, uint32_t *node_size,
                                         uint32_t slen, uint32_t shift) {
  // the all vector doesn't have sizes set yet.

  InternalNode *new_all = internal_node_create(vm, slen);
  // Current old node index to copy from
  uint32_t idx = 0;

  // Offset is how long into the current old node we've already copied from
  uint32_t offset = 0;

  if (shift == INC_SHIFT(LEAF_NODE_SHIFT)) { // handle leaf nodes here
    for (uint32_t i = 0; i < slen; i++) {
      const uint32_t new_size = node_size[i];
      LeafNode *old = (LeafNode *) all->child[idx];

      if (offset == 0 && new_size == old->len) {
        // just pointer copy the node if there is no offset and both have same
        // size
        idx++;
        new_all->child[i] = (InternalNode *) old;
      }
      else {
        LeafNode *new_node = leaf_node_create(vm, new_size);
        uint32_t cur_size = 0;
        // cur_size is the current size of the new node
        // (the amount of elements copied into it so far)

        while (cur_size < new_size /*&& idx < all->len*/) {
          // the commented out check is verified by create_concat_plan --
          // otherwise the implementation is erroneous!
          const LeafNode *old_node = (LeafNode *) all->child[idx];

          if (new_size - cur_size >= old_node->len - offset) {
            // if this node can contain all elements not copied in the old node,
            // copy all of them into this node
            memcpy(&new_node->child[cur_size], &old_node->child[offset],
                   (old_node->len - offset) * sizeof(void *));
            cur_size += old_node->len - offset;
            idx++;
            offset = 0;
          }
          else {
            // if this node can't contain all the elements not copied in the old
            // node, copy as many as we can and pass the old node over to the
            // new node after this one.
            memcpy(&new_node->child[cur_size], &old_node->child[offset],
                   (new_size - cur_size) * sizeof(void *));
            offset += new_size - cur_size;
            cur_size = new_size;
          }
        }

        new_all->child[i] = (InternalNode *) new_node;
      }
    }
  }
  else { // not at lowest non-leaf level
    // this is ALMOST equivalent with the leaf node copying, the only difference
    // is that this is with internal nodes and the fact that they have to create
    // their size tables.

    // As that's the only difference, I won't bother with comments here.
    for (uint32_t i = 0; i < slen; i++) {
      const uint32_t new_size = node_size[i];
      InternalNode *old = all->child[idx];

      if (offset == 0 && new_size == old->len) {
        idx++;
        new_all->child[i] = old;
      }
      else {
        InternalNode *new_node = internal_node_create(vm, new_size);
        uint32_t cur_size = 0;
        while (cur_size < new_size) {
          const InternalNode *old_node = all->child[idx];

          if (new_size - cur_size >= old_node->len - offset) {
            memcpy(&new_node->child[cur_size], &old_node->child[offset],
                   (old_node->len - offset) * sizeof(InternalNode *));
            cur_size += old_node->len - offset;
            idx++;
            offset = 0;
          }
          else {
            memcpy(&new_node->child[cur_size], &old_node->child[offset],
                   (new_size - cur_size) * sizeof(InternalNode *));
            offset += new_size - cur_size;
            cur_size = new_size;
          }
        }
        set_sizes(vm, new_node, DEC_SHIFT(shift)); // This is where we set sizes
        new_all->child[i] = new_node;
      }
    }
  }
  return new_all;
}

// optimize this away?
static uint32_t find_shift(TreeNode *node) {
  if (node->type == LEAF_NODE) {
    return 0;
  }
  else { // must be internal node
    InternalNode *inode = (InternalNode *) node;
    return RRB_BITS + find_shift((TreeNode *) inode->child[0]);
  }
}

static InternalNode* set_sizes(vm_t *vm, InternalNode *node, uint32_t shift) {
  uint32_t sum = 0;
  RRBSizeTable *table = size_table_create(vm, node->len);
  const uint32_t child_shift = DEC_SHIFT(shift);

  for (uint32_t i = 0; i < node->len; i++) {
    sum += size_sub_trie((TreeNode *) node->child[i], child_shift);
    table->size[i] = sum;
  }
  node->size_table = table;
  return node;
}

static uint32_t size_sub_trie(TreeNode *node, uint32_t shift) {
  if (shift > LEAF_NODE_SHIFT) {
    InternalNode *internal = (InternalNode *) node;
    if (internal->size_table == NULL) {
      uint32_t len = internal->len;
      uint32_t child_shift = DEC_SHIFT(shift);
      // TODO: for loopify recursive calls
      /* We're not sure how many are in the last child, so look it up */
      uint32_t last_size =
        size_sub_trie((TreeNode *) internal->child[len - 1], child_shift);
      /* We know all but the last ones are filled, and they have child_shift
         elements in them. */
      return ((len - 1) << shift) + last_size;
    }
    else {
      return internal->size_table->size[internal->len - 1];
    }
  }
  else {
    LeafNode *leaf = (LeafNode *) node;
    return leaf->len;
  }
}

static inline RRB* rrb_tail_push(vm_t *vm, const RRB *restrict rrb, const void *restrict elt);

static inline RRB* rrb_tail_push(vm_t *vm, const RRB *restrict rrb, const void *restrict elt) {
  RRB* new_rrb = rrb_head_clone(vm, rrb);

  LeafNode *new_tail = leaf_node_inc(vm, rrb->tail);

  new_tail->child[new_rrb->tail_len] = elt;
  new_rrb->cnt++;
  new_rrb->tail_len++;
  new_rrb->tail = new_tail;
  return new_rrb;
}

static InternalNode** copy_first_k(vm_t *vm, const RRB *rrb, RRB *new_rrb, const uint32_t k,
                                   const uint32_t tail_size);

static InternalNode** append_empty(vm_t *vm, InternalNode **to_set,
                                   uint32_t empty_height);

const RRB* rrb_push(vm_t *vm, const RRB *restrict rrb, const void *restrict elt) {
  if (rrb->tail_len < RRB_BRANCHING) {
    return rrb_tail_push(vm, rrb, elt);
  }
  RRB *new_rrb = rrb_head_clone(vm, rrb);
  new_rrb->cnt++;

  LeafNode *new_tail = leaf_node_create(vm, 1);
  new_tail->child[0] = elt;
  new_rrb->tail_len = 1;
  RRB* to_return = push_down_tail(vm, rrb, new_rrb, new_tail);
  return to_return;
}


static RRB* push_down_tail(vm_t *vm, const RRB *restrict rrb, RRB *restrict new_rrb,
                           LeafNode *restrict new_tail) {
  const LeafNode *old_tail = new_rrb->tail;
  new_rrb->tail = new_tail;
  if (rrb->cnt <= RRB_BRANCHING) {
    new_rrb->shift = LEAF_NODE_SHIFT;
    new_rrb->root = (TreeNode *) old_tail;
    return new_rrb;
  }
  // Copyable count starts here

  // TODO: Can find last rightmost jump in constant time for pvec subvecs:
  // use the fact that (index & large_mask) == 1 << (RRB_BITS * H) - 1 -> 0 etc.

  uint32_t index = rrb->cnt - 1;

  uint32_t nodes_to_copy = 0;
  uint32_t nodes_visited = 0;
  uint32_t pos = 0; // pos is the position we insert empty nodes in the bottom
                    // copyable node (or the element, if we can copy the leaf)
  const InternalNode *current = (const InternalNode *) rrb->root;

  uint32_t shift = RRB_SHIFT(rrb);

  // checking all non-leaf nodes (or if tail, all but the lowest two levels)
  while (shift > INC_SHIFT(LEAF_NODE_SHIFT)) {
    // calculate child index
    uint32_t child_index;
    if (current->size_table == NULL) {
      // some check here to ensure we're not overflowing the pvec subvec.
      // important to realise that this only needs to be done once in a better
      // impl, the same way the size_table check only has to be done until it's
      // false.
      const uint32_t prev_shift = shift + RRB_BITS;
      if (index >> prev_shift > 0) {
        nodes_visited++; // this could possibly be done earlier in the code.
        goto copyable_count_end;
      }
      child_index = (index >> shift) & RRB_MASK;
      // index filtering is not necessary when the check above is performed at
      // most once.
      index &= ~(RRB_MASK << shift);
    }
    else {
      // no need for sized_pos here, luckily.
      child_index = current->len - 1;
      // Decrement index
      if (child_index != 0) {
        index -= current->size_table->size[child_index-1];
      }
    }
    nodes_visited++;
    if (child_index < RRB_MASK) {
      nodes_to_copy = nodes_visited;
      pos = child_index;
    }

    current = current->child[child_index];
    // This will only happen in a pvec subtree
    if (current == NULL) {
      nodes_to_copy = nodes_visited;
      pos = child_index;

      // if next element we're looking at is null, we can copy all above. Good
      // times.
      goto copyable_count_end;
    }
    shift -= RRB_BITS;
  }
  // if we're here, we're at the leaf node (or lowest non-leaf), which is
  // `current`

  // no need to even use index here: We know it'll be placed at current->len,
  // if there's enough space. That check is easy.
  if (shift != 0) {
    nodes_visited++;
    if (current->len < RRB_BRANCHING) {
      nodes_to_copy = nodes_visited;
      pos = current->len;
    }
  }

 copyable_count_end:
  // GURRHH, nodes_visited is not yet handled nicely. for loop down to get
  // nodes_visited set straight.
  while (shift > INC_SHIFT(LEAF_NODE_SHIFT)) {
    nodes_visited++;
    shift -= RRB_BITS;
  }

  // Increasing height of tree.
  if (nodes_to_copy == 0) {
    InternalNode *new_root = internal_node_create(vm, 2);
    new_root->child[0] = (InternalNode *) rrb->root;
    new_rrb->root = (TreeNode *) new_root;
    new_rrb->shift = INC_SHIFT(RRB_SHIFT(new_rrb));

    // create size table if the original rrb root has a size table.
    if (rrb->root->type != LEAF_NODE &&
        ((const InternalNode *)rrb->root)->size_table != NULL) {
      RRBSizeTable *table = size_table_create(vm, 2);
      table->size[0] = rrb->cnt - old_tail->len;
      // If we insert the tail, the old size minus the old tail size will be the
      // amount of elements in the left branch. If there is no tail, the size is
      // just the old rrb-tree.

      table->size[1] = rrb->cnt;
      // If we insert the tail, the old size would include the tail.
      // Consequently, it has to be the old size. If we have no tail, we append
      // a single element to the old vector, therefore it has to be one more
      // than the original.

      new_root->size_table = table;
    }

    // nodes visited == original rrb tree height. Nodes visited > 0.
    InternalNode **to_set = append_empty(vm, &((InternalNode *) new_rrb->root)->child[1],
                                         nodes_visited);
    *to_set = (InternalNode *) old_tail;
  }
  else {
    InternalNode **node = copy_first_k(vm, rrb, new_rrb, nodes_to_copy, old_tail->len);
    InternalNode **to_set = append_empty(vm, node, nodes_visited - nodes_to_copy);
    *to_set = (InternalNode *) old_tail;
  }

  return new_rrb;
}

// - Height should be shift or height, not max element size
// - copy_first_k returns a pointer to the next pointer to set
// - append_empty now returns a pointer to the *void we're supposed to set

static InternalNode** copy_first_k(vm_t *vm, const RRB *rrb, RRB *new_rrb, const uint32_t k,
                                   const uint32_t tail_size) {
  const InternalNode *current = (const InternalNode *) rrb->root;
  InternalNode **to_set = (InternalNode **) &new_rrb->root;
  uint32_t index = rrb->cnt - 1;
  uint32_t shift = RRB_SHIFT(rrb);

  // Copy all non-leaf nodes first. Happens when shift > RRB_BRANCHING
  uint32_t i = 1;
  while (i <= k && shift != 0) {
    // First off, copy current node and stick it in.
    InternalNode *new_current;
    if (i != k) {
      new_current = internal_node_clone(vm, current);
      if (current->size_table != NULL) {
        new_current->size_table = size_table_clone(vm, new_current->size_table, new_current->len);
        new_current->size_table->size[new_current->len-1] += tail_size;
      }
    }
    else { // increment size of last elt -- will only happen if we append empties
      new_current = internal_node_inc(vm, current);
      if (current->size_table != NULL) {
        new_current->size_table->size[new_current->len-1] =
          new_current->size_table->size[new_current->len-2] + tail_size;
      }
    }
    *to_set = new_current;

    // calculate child index
    uint32_t child_index;
    if (current->size_table == NULL) {
      child_index = (index >> shift) & RRB_MASK;
    }
    else {
      // no need for sized_pos here, luckily.
      child_index = new_current->len - 1;
      // Decrement index
      if (child_index != 0) {
        index -= current->size_table->size[child_index-1];
      }
    }
    to_set = &new_current->child[child_index];
    current = current->child[child_index];

    i++;
    shift -= RRB_BITS;
  }

  return to_set;
}

static InternalNode** append_empty(vm_t *vm, InternalNode **to_set,
                                   uint32_t empty_height) {
  if (0 < empty_height) {
    InternalNode *leaf = internal_node_create(vm, 1);
    InternalNode *empty = (InternalNode *) leaf;
    for (uint32_t i = 1; i < empty_height; i++) {
      InternalNode *new_empty = internal_node_create(vm, 1);
      new_empty->child[0] = empty;
      empty = new_empty;
    }
    // this root node must be one larger, otherwise segfault
    *to_set = empty;
    return &leaf->child[0];
  }
  else {
    return to_set;
  }
}

static uint32_t sized_pos(const InternalNode *node, uint32_t *index,
                          uint32_t sp) {
  RRBSizeTable *table = node->size_table;
  uint32_t is = *index >> sp;
  while (table->size[is] <= *index) {
    is++;
  }
  if (is != 0) {
    *index -= table->size[is-1];
  }
  return is;
}

static const InternalNode* sized(const InternalNode *node, uint32_t *index,
                                 uint32_t sp) {
  uint32_t is = sized_pos(node, index, sp);
  return (InternalNode *) node->child[is];
}

void* rrb_nth(const RRB *rrb, uint32_t index) {
  if (index >= rrb->cnt) {
    return NULL;
  }
  const uint32_t tail_offset = rrb->cnt - rrb->tail_len;
  if (tail_offset <= index) {
    return (void*) rrb->tail->child[index - tail_offset];
  }
  else {
    const InternalNode *current = (const InternalNode *) rrb->root;
    for (uint32_t shift = RRB_SHIFT(rrb); shift > 0; shift -= RRB_BITS) {
      if (current->size_table == NULL) {
        const uint32_t subidx = (index >> shift) & RRB_MASK;
        current = current->child[subidx];
      }
      else {
        current = sized(current, &index, shift);
      }
    }
    return (void*) ((const LeafNode *)current)->child[index & RRB_MASK];
  }
}

uint32_t rrb_count(const RRB *rrb) {
  return rrb->cnt;
}

void* rrb_peek(const RRB *rrb) {
  return (void *) rrb->tail->child[rrb->tail_len-1];
}

/**
 * Destructively replaces the rightmost leaf as the new tail, discarding the
 * old.
 */
// Note that this is very similar to the direct pop algorithm, which is
// described further down in this file.
static void promote_rightmost_leaf(vm_t *vm, RRB *new_rrb) {
  InternalNode *path[RRB_MAX_HEIGHT+1];
  path[0] = (InternalNode *) new_rrb->root;
  uint32_t i = 0, shift = LEAF_NODE_SHIFT;

  // populate path array
  for (i = 0, shift = LEAF_NODE_SHIFT; shift < RRB_SHIFT(new_rrb);
       i++, shift += RRB_BITS) {
    path[i+1] = path[i]->child[path[i]->len-1];
  }

  const uint32_t height = i;
  // Set the leaf node as tail.
  new_rrb->tail = (LeafNode *) path[height];
  new_rrb->tail_len = path[height]->len;
  const uint32_t tail_len = new_rrb->tail_len;

  // last element is now always null, in contrast to direct pop
  path[height] = NULL;

  while (i --> 0) {
    // TODO: First skip will always happen. Can we use that somehow?
    if (path[i+1] == NULL) {
      if (path[i]->len == 1) {
        path[i] = NULL;
      }
      else if (i == 0 && path[i]->len == 2) {
        path[i] = path[i]->child[0];
        new_rrb->shift -= RRB_BITS;
      }
      else {
        path[i] = internal_node_dec(vm, path[i]);
      }
    }
    else {
      path[i] = internal_node_clone(vm, path[i]);
      path[i]->child[path[i]->len-1] = path[i+1];
      if (path[i]->size_table != NULL) {
        path[i]->size_table = size_table_clone(vm, path[i]->size_table, path[i]->len);
        // this line differs, as we remove `tail_len` elements from the trie,
        // instead of just 1 as in the direct pop algorithm.
        path[i]->size_table->size[path[i]->len-1] -= tail_len;
      }
    }
  }

  new_rrb->root = (TreeNode *) path[0];
}

static RRB* slice_right(vm_t *vm, const RRB *rrb, const uint32_t right) {
  if (right == 0) {
    return (RRB *) rrb_create();
  }
  else if (right < rrb->cnt) {
    const uint32_t tail_offset = rrb->cnt - rrb->tail_len;
    // Can just cut the tail slightly
    if (tail_offset < right) {
      RRB *new_rrb = rrb_head_clone(vm, rrb);
      const uint32_t new_tail_len = right - tail_offset;
      LeafNode *new_tail = leaf_node_create(vm, new_tail_len);
      memcpy(new_tail->child, rrb->tail->child, new_tail_len * sizeof(void *));
      new_rrb->cnt = right;
      new_rrb->tail = new_tail;
      new_rrb->tail_len = new_tail_len;
      return new_rrb;
    }

    RRB *new_rrb = rrb_mutable_create(vm);
    TreeNode *root = slice_right_rec(vm, &RRB_SHIFT(new_rrb), rrb->root, right - 1,
                                     RRB_SHIFT(rrb), false);
    new_rrb->cnt = right;
    new_rrb->root = root;

    // Not sure if this is necessary in this part of the program, due to issues
    // wrt. slice_left and roots without size tables.
    promote_rightmost_leaf(vm, new_rrb);
    new_rrb->tail_len = new_rrb->tail->len;
    return new_rrb;
  }
  else {
    return (RRB *) rrb;
  }
}

static TreeNode* slice_right_rec(vm_t *vm, uint32_t *total_shift, const TreeNode *root,
                                 uint32_t right, uint32_t shift,
                                 char has_left) {
  const uint32_t subshift = DEC_SHIFT(shift);
  uint32_t subidx = right >> shift;
  if (shift > LEAF_NODE_SHIFT) {
    const InternalNode *internal_root = (InternalNode *) root;
    if (internal_root->size_table == NULL) {
      TreeNode *right_hand_node =
        slice_right_rec(vm, total_shift,
                        (TreeNode *) internal_root->child[subidx],
                        right - (subidx << shift), subshift,
                        (subidx != 0) | has_left);
      if (subidx == 0) {
        if (has_left) {
          InternalNode *right_hand_parent = internal_node_create(vm, 1);
          right_hand_parent->child[0] = (InternalNode *) right_hand_node;
          *total_shift = shift;
          return (TreeNode *) right_hand_parent;
        }
        else { // if (!has_left)
          return right_hand_node;
        }
      }
      else { // if (subidx != 0)
        InternalNode *sliced_root = internal_node_create(vm, subidx + 1);
        memcpy(sliced_root->child, internal_root->child,
               subidx * sizeof(InternalNode *));
        sliced_root->child[subidx] = (InternalNode *) right_hand_node;
        *total_shift = shift;
        return (TreeNode *) sliced_root;
      }
    }
    else { // if (internal_root->size_table != NULL)
      RRBSizeTable *table = internal_root->size_table;
      uint32_t idx = right;

      while (table->size[subidx] <= idx) {
        subidx++;
      }
      if (subidx != 0) {
        idx -= table->size[subidx-1];
      }

      const TreeNode *right_hand_node =
        slice_right_rec(vm, total_shift, (const TreeNode*) internal_root->child[subidx], idx,
                        subshift, (subidx != 0) | has_left);
      if (subidx == 0) {
        if (has_left) {
          // As there is one above us, must place the right hand node in a
          // one-node
          InternalNode *right_hand_parent = internal_node_create(vm, 1);
          RRBSizeTable *right_hand_table = size_table_create(vm, 1);

          right_hand_table->size[0] = right + 1;
          // TODO: Not set size_table if the underlying node doesn't have a
          // table as well.
          right_hand_parent->size_table = right_hand_table;
          right_hand_parent->child[0] = (InternalNode *) right_hand_node;

          *total_shift = shift;
          return (TreeNode *) right_hand_parent;
        }
        else { // if (!has_left)
          return (TreeNode *) right_hand_node;
        }
      }
      else { // if (subidx != 0)
        InternalNode *sliced_root = internal_node_create(vm, subidx+1);
        RRBSizeTable *sliced_table = size_table_create(vm, subidx+1);

        memcpy(sliced_table->size, table->size, subidx * sizeof(uint32_t));
        sliced_table->size[subidx] = right+1;

        memcpy(sliced_root->child, internal_root->child,
               subidx * sizeof(InternalNode *));
        sliced_root->size_table = sliced_table;
        sliced_root->child[subidx] = (InternalNode *) right_hand_node;

        *total_shift = shift;
        return (TreeNode *) sliced_root;
      }
    }
  }
  else { // if (shift <= RRB_BRANCHING)
    // Just pure copying into a new node
    const LeafNode *leaf_root = (LeafNode *) root;
    LeafNode *left_vals = leaf_node_create(vm, subidx + 1);

    memcpy(left_vals->child, leaf_root->child, (subidx + 1) * sizeof(void *));
    *total_shift = shift;
    return (TreeNode *) left_vals;
  }
}

const RRB* slice_left(vm_t *vm, RRB *rrb, uint32_t left) {
  if (left >= rrb->cnt) {
    return rrb_create();
  }
  else if (left > 0) {
    const uint32_t remaining = rrb->cnt - left;

    // If we slice into the tail, we just need to modify the tail itself
    if (remaining <= rrb->tail_len) {
      LeafNode *new_tail = leaf_node_create(vm, remaining);
      memcpy(new_tail->child, &rrb->tail->child[rrb->tail_len - remaining],
             remaining * sizeof(void *));

      RRB *new_rrb = rrb_mutable_create(vm);
      new_rrb->cnt = remaining;
      new_rrb->tail_len = remaining;
      new_rrb->tail = new_tail;
      return new_rrb;
    }
    // Otherwise, we don't really have to take the tail into consideration.
    // Good!

    RRB *new_rrb = rrb_mutable_create(vm);
    InternalNode *root = (InternalNode *)
      slice_left_rec(vm, &RRB_SHIFT(new_rrb), rrb->root, left,
                     RRB_SHIFT(rrb), false);
    new_rrb->cnt = remaining;
    new_rrb->root = (TreeNode *) root;

    // Ensure last element in size table is correct size, if the root is an
    // internal node.
    if (new_rrb->shift != LEAF_NODE_SHIFT && root->size_table != NULL) {
      root->size_table->size[root->len-1] = new_rrb->cnt - rrb->tail_len;
    }
    new_rrb->tail = rrb->tail;
    new_rrb->tail_len = rrb->tail_len;
    rrb = new_rrb;
  }

  // TODO: I think the code below also applies to root nodes where size_table
  // == NULL and (cnt - tail_len) & 0xff != 0, but it may be that this is
  // resolved by slice_right itself. Perhaps not promote in the right slicing,
  // but here instead?

  // This case handles leaf nodes < RRB_BRANCHING size, by redistributing
  // values from the tail into the actual leaf node.
  if (RRB_SHIFT(rrb) == 0 && rrb->root != NULL) {
    // two cases to handle: cnt <= RRB_BRANCHING
    //     and (cnt - tail_len) < RRB_BRANCHING

    if (rrb->cnt <= RRB_BRANCHING) {
      // can put all into a new tail
      LeafNode *new_tail = leaf_node_create(vm, rrb->cnt);

      memcpy(&new_tail->child[0], &((LeafNode *) rrb->root)->child[0],
             rrb->root->len * sizeof(void *));
      memcpy(&new_tail->child[rrb->root->len], &rrb->tail->child[0],
             rrb->tail_len * sizeof(void *));
      rrb->tail_len = rrb->cnt;
      rrb->root = NULL;
      rrb->tail = new_tail;
    }
    // no need for <= here, because if the root node is == rrb_branching, the
    // invariant is kept.
    else if (rrb->cnt - rrb->tail_len < RRB_BRANCHING) {
      // create both a new tail and a new root node
      const uint32_t tail_cut = RRB_BRANCHING - rrb->root->len;
      LeafNode *new_root = leaf_node_create(vm, RRB_BRANCHING);
      LeafNode *new_tail = leaf_node_create(vm, rrb->tail_len - tail_cut);

      memcpy(&new_root->child[0], &((LeafNode *) rrb->root)->child[0],
             rrb->root->len * sizeof(void *));
      memcpy(&new_root->child[rrb->root->len], &rrb->tail->child[0],
             tail_cut * sizeof(void *));
      memcpy(&new_tail->child[0], &rrb->tail->child[tail_cut],
             (rrb->tail_len - tail_cut) * sizeof(void *));

      rrb->tail_len = rrb->tail_len - tail_cut;
      rrb->tail = new_tail;
      rrb->root = (TreeNode *) new_root;
    }
  }
  return rrb;
}

static TreeNode* slice_left_rec(vm_t *vm, uint32_t *total_shift, const TreeNode *root,
                                uint32_t left, uint32_t shift,
                                char has_right) {
  const uint32_t subshift = DEC_SHIFT(shift);
  uint32_t subidx = left >> shift;
  if (shift > LEAF_NODE_SHIFT) {
    const InternalNode *internal_root = (InternalNode *) root;
    uint32_t idx = left;
    if (internal_root->size_table == NULL) {
      idx -= subidx << shift;
    }
    else { // if (internal_root->size_table != NULL)
      const RRBSizeTable *table = internal_root->size_table;

      while (table->size[subidx] <= idx) {
        subidx++;
      }
      if (subidx != 0) {
        idx -= table->size[subidx - 1];
      }
    }

    const uint32_t last_slot = internal_root->len - 1;
    const TreeNode *child = (TreeNode *) internal_root->child[subidx];
    TreeNode *left_hand_node =
      slice_left_rec(vm, total_shift, child, idx, subshift,
                     (subidx != last_slot) | has_right);
    if (subidx == last_slot) { // No more slots left
      if (has_right) {
        InternalNode *left_hand_parent = internal_node_create(vm, 1);
        const InternalNode *internal_left_hand_node = (InternalNode *) left_hand_node;
        left_hand_parent->child[0] = (InternalNode *) internal_left_hand_node;

        if (subshift != LEAF_NODE_SHIFT && internal_left_hand_node->size_table != NULL) {
          RRBSizeTable *sliced_table = size_table_create(vm, 1);
          sliced_table->size[0] =
            internal_left_hand_node->size_table->size[internal_left_hand_node->len-1];
          left_hand_parent->size_table = sliced_table;
        }
        *total_shift = shift;
        return (TreeNode *) left_hand_parent;
      }
      else { // if (!has_right)
        return left_hand_node;
      }
    }
    else { // if (subidx != last_slot)

      const uint32_t sliced_len = internal_root->len - subidx;
      InternalNode *sliced_root = internal_node_create(vm, sliced_len);

      // TODO: Can shrink size here if sliced_len == 2, using the ambidextrous
      // vector technique w. offset. Takes constant time.

      memcpy(&sliced_root->child[1], &internal_root->child[subidx + 1],
             (sliced_len - 1) * sizeof(InternalNode *));

      const RRBSizeTable *table = internal_root->size_table;

      // TODO: Can check if left is a power of the tree size. If so, all nodes
      // will be completely populated, and we can ignore the size table. Most
      // importantly, this will remove the need to alloc a size table, which
      // increases perf.
      RRBSizeTable *sliced_table = size_table_create(vm, sliced_len);

      if (table == NULL) {
        for (uint32_t i = 0; i < sliced_len; i++) {
          // left is total amount sliced off. By adding in subidx, we get faster
          // computation later on.
          sliced_table->size[i] = (subidx + 1 + i) << shift;
          // NOTE: This doesn't really work properly for top root, as last node
          // may have a higher count than it *actually* has. To remedy for this,
          // the top function performs a check afterwards, which may insert the
          // correct value if there's a size table in the root.
        }
      }
      else { // if (table != NULL)
        memcpy(sliced_table->size, &table->size[subidx],
               sliced_len * sizeof(uint32_t));
      }

      for (uint32_t i = 0; i < sliced_len; i++) {
        sliced_table->size[i] -= left;
      }

      sliced_root->size_table = sliced_table;
      sliced_root->child[0] = (InternalNode *) left_hand_node;
      *total_shift = shift;
      return (TreeNode *) sliced_root;
    }
  }
  else { // if (shift <= RRB_BRANCHING)
    LeafNode *leaf_root = (LeafNode *) root;
    const uint32_t right_vals_len = leaf_root->len - subidx;
    LeafNode *right_vals = leaf_node_create(vm, right_vals_len);

    memcpy(right_vals->child, &leaf_root->child[subidx],
           right_vals_len * sizeof(void *));
    *total_shift = shift;

    return (TreeNode *) right_vals;
  }
}

const RRB* rrb_slice(vm_t *vm, const RRB *rrb, uint32_t from, uint32_t to) {
  return slice_left(vm, slice_right(vm, rrb, to), from);
}

const RRB* rrb_update(vm_t *vm, const RRB *restrict rrb, uint32_t index, const void *restrict elt) {
  if (index < rrb->cnt) {
    RRB *new_rrb = rrb_head_clone(vm, rrb);
    const uint32_t tail_offset = rrb->cnt - rrb->tail_len;
    if (tail_offset <= index) {
      LeafNode *new_tail = leaf_node_clone(vm, rrb->tail);
      new_tail->child[index - tail_offset] = elt;
      new_rrb->tail = new_tail;
      return new_rrb;
    }
    InternalNode **previous_pointer = (InternalNode **) &new_rrb->root;
    InternalNode *current = (InternalNode *) rrb->root;
    for (uint32_t shift = RRB_SHIFT(rrb); shift > 0; shift -= RRB_BITS) {
      current = internal_node_clone(vm, current);
      *previous_pointer = current;

      uint32_t child_index;
      if (current->size_table == NULL) {
        child_index = (index >> shift) & RRB_MASK;
      }
      else {
        child_index = sized_pos(current, &index, shift);
      }
      previous_pointer = &current->child[child_index];
      current = current->child[child_index];
    }

    LeafNode *leaf = (LeafNode *) current;
    leaf = leaf_node_clone(vm, leaf);
    *previous_pointer = (InternalNode *) leaf;
    leaf->child[index & RRB_MASK] = elt;
    return new_rrb;
  }
  else {
    return NULL;
  }
}

// Also assume direct append
const RRB* rrb_pop(vm_t *vm, const RRB *rrb) {
  if (rrb->cnt == 1) {
    return rrb_create();
  }
  RRB* new_rrb = rrb_head_clone(vm, rrb);
  new_rrb->cnt--;

  if (rrb->tail_len == 1) {
    promote_rightmost_leaf(vm, new_rrb);
    return new_rrb;
  }
  else {
    LeafNode *new_tail = leaf_node_dec(vm, rrb->tail);
    new_rrb->tail_len--;
    new_rrb->tail = new_tail;
    return new_rrb;
  }
}

// PUBLIC API

owl_term owl_list_init() {
  const RRB *rrb = rrb_create();
  return rrb_to_list(rrb);
}

owl_term owl_list_push(vm_t *vm, owl_term list, owl_term elem) {
  const RRB *rrb = list_to_rrb(list);
  rrb = rrb_push(vm, rrb, (void*) elem);
  return rrb_to_list(rrb);
}

owl_term owl_list_nth(owl_term list, owl_term owl_index) {
  const RRB *rrb = list_to_rrb(list);
  uint64_t index = int_from_owl_int(owl_index);
  void *found = rrb_nth(rrb, index);

  if (found == NULL) {
    return OWL_NIL;
  } else {
    return (owl_term) found;
  }
}

owl_term owl_list_count(owl_term list) {
  const RRB *rrb = list_to_rrb(list);
  return owl_int_from(rrb_count(rrb));
}

owl_term owl_list_slice(vm_t *vm, owl_term list, owl_term from, owl_term to) {
  const RRB *rrb = list_to_rrb(list);
  uint64_t from_int = int_from_owl_int(from);
  uint64_t to_int = int_from_owl_int(to);
  const RRB *sliced = rrb_slice(vm, rrb, from_int, to_int);

  return rrb_to_list(sliced);
}

owl_term owl_list_concat(vm_t *vm, owl_term left_list, owl_term right_list) {
  const RRB *left = list_to_rrb(left_list);
  const RRB *right = list_to_rrb(right_list);
  const RRB *result = rrb_concat(vm, left, right);
  return rrb_to_list(result);
}

void owl_list_print(vm_t *vm, owl_term list) {
  const RRB *rrb = list_to_rrb(list);

  uint32_t count = rrb_count(rrb);

  printf("[");
  for (uint32_t i = 0; i < count; i++) {
    owl_term_print(vm, (owl_term) rrb_nth(rrb, i));
    if (i != count - 1) {
      printf(",");
    }
  }
  printf("]\n");
}

bool owl_list_is_empty(owl_term list) {
  return list_to_rrb(list) == &EMPTY_RRB;
}

bool owl_list_eq(owl_term left_list, owl_term right_list) {
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
