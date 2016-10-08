// Using a standard Chaney's copying garbage collector
// https://en.wikipedia.org/wiki/Cheney%27s_algorithm
// Missing forwarding pointers

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "std/owl_list.h"
#include "alloc.h"
#include "term.h"
#include "stack.h"

#define ALIGNMENT 8
#define ALIGN(size) size + (ALIGNMENT - (size % ALIGNMENT))

static owl_term copy(owl_term term, GCState* gc);

void die(const char* message) {
  puts(message);
  exit(1);
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
        return ALIGN((tuple_length + 1) * sizeof(owl_term));
      }
    case STRING:
      return ALIGN(strlen(owl_extract_ptr(term)));
    case FUNCTION:
      {
        Function *fun = owl_extract_ptr(term);

        if (!fun->on_gc_heap) {
          return 0;
        }

        return ALIGN(sizeof(Function));
      }
    case LIST:
      return ALIGN(sizeof(RRB));
    case INT:
      return 0;
    case POINTER:
      die("POINTER");
    default:
      return 0;
  }
}

static void* bump_cpy(GCState *gc, void *from, int size) {
  void* copied = gc->alloc_ptr;
  gc->alloc_ptr += size;
  memcpy(copied, from, size);
  memset(from, 0, size);
  return copied;
}

static TreeNode* copy_list_node(TreeNode *node, GCState *gc) {
  if (node->type == LEAF_NODE) {
    LeafNode *leaf = (LeafNode*) node;

    for (uint32_t i = 0; i < leaf->len; i++) {
      leaf->child[i] = (void*) copy((owl_term) leaf->child[i], gc);
    }

    return bump_cpy(gc, leaf, sizeof(LeafNode) + leaf->len * sizeof(void *));
  } else if (node->type == INTERNAL_NODE) {
    InternalNode *internal = (InternalNode*) node;

    for (uint32_t i = 0; i < internal->len; i++) {
      internal->child[i] = (InternalNode*) copy_list_node((TreeNode*) internal->child[i], gc);
    }

    if (internal->size_table) {
      internal->size_table = bump_cpy(gc, internal->size_table, sizeof(RRBSizeTable) + internal->len * sizeof(uint32_t));
    }
    return bump_cpy(gc, internal, sizeof(InternalNode) + internal->len * sizeof(InternalNode *));
  }
  die("Unknown list node type");
}

static void copy_list_refs(owl_term term, GCState *gc) {
  RRB *rrb = owl_extract_ptr(term);
  if (rrb->root) {
    rrb->root = copy_list_node(rrb->root, gc);
  }
  rrb->tail = (LeafNode*) copy_list_node((TreeNode*) rrb->tail, gc);
}

static void copy_refs(owl_term term, GCState *gc) {
  if (term == OWL_FALSE || term == OWL_TRUE || term == OWL_NIL) {
    return;
  }

  switch(owl_tag_of(term)) {
    case INT:
    case STRING:
      return;
    case TUPLE:
      {
        owl_term *ary = owl_extract_ptr(term);
        uint32_t tuple_length = ary[0];
        for(uint32_t i = 1; i <= tuple_length; i++) {
          ary[i] = copy(ary[i], gc);
        }
        return;
      }
    case FUNCTION:
      {
        Function *fun = owl_extract_ptr(term);

        if (!fun->on_gc_heap) {
          return;
        }

        for(int i = 0; i < MAX_UPVALUES; i++) {
          fun->upvalues[i] = copy(fun->upvalues[i], gc);
        }
        return;
      }
    case LIST:
      copy_list_refs(term, gc);
      return;
    default:
      die("Cannot copy refs");
  }
}


static owl_term copy(owl_term term, GCState* gc) {
  uint32_t heap_size = heap_size_of(term);

  // Non-heap allocated object. No need to copy
  if (heap_size == 0) {
    return term;
  }

  void* copied = gc->alloc_ptr;
  gc->alloc_ptr += heap_size;
  memcpy(copied, owl_extract_ptr(term), heap_size);
  owl_term copied_term = owl_tag_as(copied, owl_tag_of(term));
  copy_refs(copied_term, gc);
  return copied_term;
}

static void swap_spaces(GCState* gc) {
  void* temp = gc->to_space;
  gc->to_space = gc->from_space;
  gc->from_space = temp;
}

void collect(vm_t *vm) {
  puts("COLLECT");
  swap_spaces(vm->gc);
  vm->gc->alloc_ptr = vm->gc->to_space;

  for(uint32_t a = 0; a < stack_size(vm->gc->protect); a++) {
    RRB** protected = stack_get_indirect(vm->gc->protect, a);
    owl_term copied = copy(rrb_to_list(*protected), vm->gc);
    *protected = list_to_rrb(copied);
  }

  for (uint64_t i = 0; i <= vm->current_frame; i++) {
    frame_t frame = vm->frames[i];

    for (uint64_t j = 0; j < REGISTER_COUNT; j++) {
      owl_term object = frame.registers[j];
      if (object) {
        vm->frames[i].registers[j] = copy(object, vm->gc);
      }
    }
  }
}

void** gc_protect(vm_t* vm, void* item) {
  stack_push(vm->gc->protect, item);
  return stack_peek_indirect(vm->gc->protect);
}

void* gc_unprotect(vm_t* vm) {
  return stack_pop(vm->gc->protect);
}

void* owl_alloc(vm_t *vm, int N) {
  int block_size = ALIGN(N);

  if (vm->gc->alloc_ptr + block_size > vm->gc->to_space + vm->gc->size / 2) {
    collect(vm);
  }
  if (vm->gc->alloc_ptr + block_size > vm->gc->to_space + vm->gc->size / 2) {
    die("Insufficient memory");
  }

  void* object = vm->gc->alloc_ptr;
  vm->gc->alloc_ptr += block_size;

  return object;
}
