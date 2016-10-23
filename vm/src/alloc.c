#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "std/owl_list.h"
#include "alloc.h"
#include "term.h"

// Using a standard Chaney's copying garbage collector
// https://en.wikipedia.org/wiki/Cheney%27s_algorithm

#define ALIGNMENT 8
#define ENFORE_MINIMUM(size) size < 8 ? 8 : size

#define BUFFER_PERCENT 10

#define FORWARD_FLAG(ptr) *(((uint8_t*) ptr) - 1)
#define SET_FORWARD_FLAG(ptr, val) FORWARD_FLAG(ptr) = val
#define FORWARD_ADDRESS(ptr) *((void**) ptr)
#define SET_FORWARD_ADDRESS(ptr, val) FORWARD_ADDRESS(ptr) = val
#define ON_HEAP(gc, ptr) (((uint8_t*) ptr) >= gc->from_space && ((uint8_t*) ptr) <= gc->from_space + gc->size)

uint64_t bytes_allocated = 0;
static owl_term copy(owl_term term, vm_t* vm);

static inline uint32_t align(uint32_t size) {
  return size % ALIGNMENT == 0 ? size : size + (ALIGNMENT - (size % ALIGNMENT));
}

void die(const char* message) {
  puts(message);
  exit(1);
}

void gc_check_overlflow(vm_t *vm, uint32_t block_size) {
  if (vm->gc->alloc_ptr + block_size > vm->gc->to_space + vm->gc->size / 2) {
    die("Insufficient memory");
  }
}

void forward(void* old, void* new) {
  SET_FORWARD_FLAG(new, false);
  SET_FORWARD_ADDRESS(old, new);
  SET_FORWARD_FLAG(old, true);
}

// Returns the number of bytes that this term takes up on the heap
// For terms that are not heap-allocated, returns 0
static uint32_t heap_size_of(owl_term term) {
  if (term == OWL_FALSE || term == OWL_TRUE || term == OWL_NIL) {
    return 0;
  }

  switch(owl_tag_of(term)) {
    case TUPLE:
      {
        owl_term *ary = owl_extract_ptr(term);
        uint64_t tuple_length = ary[0];
        return align((tuple_length + 1) * sizeof(owl_term));
      }
    case STRING:
      return align(strlen(owl_extract_ptr(term)));
    case FUNCTION:
      {
        Function* fun = owl_extract_ptr(term);
        return align(sizeof(Function) + fun->n_upvalues * sizeof(owl_term));
      }
    case LIST:
      if (owl_list_is_empty(term)) {
        return 0;
      } else {
        return align(sizeof(RRB));
      }
    case POINTER:
      die("POINTER");
    default:
      return 0;
  }
}

static void* bump_cpy(vm_t *vm, void *from, uint32_t size) {
  uint32_t block_size = ENFORE_MINIMUM(size);

  gc_check_overlflow(vm, block_size + 1);
  void* copied = vm->gc->alloc_ptr + 1;
  vm->gc->alloc_ptr += block_size + 1;
  memcpy(copied, from, block_size);
  forward(from, copied);
  return copied;
}

static TreeNode* copy_list_node(TreeNode *node, vm_t* vm) {
  if (FORWARD_FLAG(node)) {
    return (TreeNode*) FORWARD_ADDRESS(node);
  }

  if (node->type == LEAF_NODE) {
    LeafNode *leaf = (LeafNode*) node;

    for (uint32_t i = 0; i < leaf->len; i++) {
      leaf->child[i] = (void*) copy((owl_term) leaf->child[i], vm);
    }

    return bump_cpy(vm, leaf, sizeof(LeafNode) + leaf->len * sizeof(void *));
  } else { // INTERNAL_NODE
    InternalNode *internal = (InternalNode*) node;

    for (uint32_t i = 0; i < internal->len; i++) {
      internal->child[i] = (InternalNode*) copy_list_node((TreeNode*) internal->child[i], vm);
    }

    if (internal->size_table) {
      internal->size_table = bump_cpy(vm, internal->size_table, sizeof(RRBSizeTable) + internal->len * sizeof(uint32_t));
    }
    return bump_cpy(vm, internal, sizeof(InternalNode) + internal->len * sizeof(InternalNode *));
  }
}

static void copy_refs(owl_term term, vm_t *vm) {
  switch(owl_tag_of(term)) {
    case STRING:
      return;
    case TUPLE:
      {
        owl_term *ary = owl_extract_ptr(term);
        uint32_t tuple_length = ary[0];
        for(uint32_t i = 1; i <= tuple_length; i++) {
          ary[i] = copy(ary[i], vm);
        }
        return;
      }
    case FUNCTION:
      {
        Function *fun = owl_extract_ptr(term);

        for(int i = 0; i < fun->n_upvalues; i++) {
          if (fun->upvalues[i]) {
            fun->upvalues[i] = copy(fun->upvalues[i], vm);
          }
        }
        return;
      }
    case LIST:
      {
        RRB *rrb = owl_extract_ptr(term);
        if (rrb->root) {
          rrb->root = copy_list_node(rrb->root, vm);
        }
        rrb->tail = (LeafNode*) copy_list_node((TreeNode*) rrb->tail, vm);
        return;
      }
    default:
      die("Cannot copy refs");
  }
}

static owl_term copy(owl_term term, vm_t* vm) {
  void* object = owl_extract_ptr(term);

  if(!ON_HEAP(vm->gc, object)) {
    return term;
  }

  if (FORWARD_FLAG(object)) {
    return owl_tag_as(FORWARD_ADDRESS(object), owl_tag_of(term));
  }

  uint32_t heap_size = heap_size_of(term);

  // Non-heap allocated object. Nothing to copy
  if (heap_size == 0) {
    return term;
  }

  gc_check_overlflow(vm, heap_size);

  void* copied = vm->gc->alloc_ptr + 1;
  vm->gc->alloc_ptr += heap_size + 1;
  memcpy(copied, object, heap_size);
  owl_term copied_term = owl_tag_as(copied, owl_tag_of(term));
  copy_refs(copied_term, vm);

  forward(object, copied);

  return copied_term;
}

static void swap_spaces(GCState* gc) {
  void* temp = gc->to_space;
  gc->to_space = gc->from_space;
  gc->from_space = temp;
  gc->alloc_ptr = gc->to_space;
}

uint32_t gc_usage(vm_t *vm) {
  return vm->gc->alloc_ptr - vm->gc->to_space;
}

void gc_collect(vm_t *vm) {
  swap_spaces(vm->gc);

  for (uint32_t i = 0; i <= vm->current_frame; i++) {
    for (uint32_t j = 0; j < REGISTER_COUNT; j++) {
      owl_term object = vm->frames[i].registers[j];
      if (object) {
        vm->frames[i].registers[j] = copy(object, vm);
      }
    }
  }
}

void gc_safepoint(vm_t* vm) {
  uint64_t space_size = vm->gc->size / 2;
  uint64_t end = (uint64_t) vm->gc->to_space + space_size;
  uint64_t buffer = space_size * (BUFFER_PERCENT / 100.0);

  if ((uint64_t) vm->gc->alloc_ptr + buffer > end) {
    gc_collect(vm);
  }
}

uint64_t gc_bytes_allocated() {
  return bytes_allocated;
}

void* owl_alloc(vm_t *vm, uint32_t N) {
  uint32_t block_size = align(N) + 1;

  gc_check_overlflow(vm, block_size);

  uint8_t* object = vm->gc->alloc_ptr;
  vm->gc->alloc_ptr += block_size;
  memset(object, 0, block_size);
  bytes_allocated += block_size;

  return object + 1;
}
