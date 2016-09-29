#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gc/gc.h>
#include "alloc.h"
#include "term.h"

owl_term copy_tuple(owl_term tuple, GCState* gc) {
  owl_term *ary = owl_extract_ptr(tuple);
  uint64_t tuple_length = ary[0];
  uint64_t mem_size = (tuple_length + 1) * sizeof(owl_term);
  void* copied = gc->alloc_ptr;
  gc->alloc_ptr += mem_size;
  memcpy(copied, (void*) ary, mem_size);
  return owl_tag_as(copied, TUPLE);
}

owl_term copy_string(owl_term string, GCState* gc) {
  char *str = owl_extract_ptr(string);
  uint64_t mem_size = strlen(str);
  void* copied = gc->alloc_ptr;
  gc->alloc_ptr += mem_size;
  memcpy(copied, (void*) str, mem_size);
  return owl_tag_as(copied, STRING);
}

owl_term copy_function(owl_term f, GCState* gc) {
  Function *fun = owl_extract_ptr(f);

  if (!fun->on_gc_heap) {
    return f;
  }

  uint64_t mem_size = sizeof(Function);
  void* copied = gc->alloc_ptr;
  gc->alloc_ptr += mem_size;
  memcpy(copied, (void*) fun, mem_size);
  return owl_tag_as(copied, FUNCTION);
}

owl_term copy(owl_term term, GCState* gc) {
  if (term == OWL_FALSE || term == OWL_TRUE || term == OWL_NIL) {
    return term;
  }

  switch(owl_tag_of(term)) {
    case TUPLE:
      return copy_tuple(term, gc);
    case STRING:
      return copy_string(term, gc);
    case FUNCTION:
      return copy_function(term, gc);
    default:
      return term;
  }
}

void swap_spaces(GCState* gc) {
  void* temp = gc->to_space;
  gc->to_space = gc->from_space;
  gc->from_space = temp;
}

void collect(vm_t *vm) {
  swap_spaces(vm->gc);
  vm->gc->alloc_ptr = vm->gc->to_space;
  vm->gc->scan_ptr = vm->gc->to_space;

  for (uint64_t i = 0; i < vm->current_frame; i++) {
    frame_t frame = vm->frames[i];

    for (uint64_t j = 0; j < REGISTER_COUNT; j++) {
      owl_term object = frame.registers[j];
      if (object) {
        frame.registers[j] = copy(object, vm->gc);
      }
    }
  }
}

void* allocate(vm_t* vm, int N) {
  if (vm->gc->alloc_ptr + N > vm->gc->to_space + vm->gc->size / 2) {
    collect(vm);
  }
  if (vm->gc->alloc_ptr + N > vm->gc->to_space + vm->gc->size / 2) {
    puts("insufficient memory");
    exit(1);
  }

  void* object = vm->gc->alloc_ptr;
  vm->gc->alloc_ptr += N;

  printf("%llu\n", (vm->gc->alloc_ptr - vm->gc->to_space));
  return object;
}

void* owl_alloc(vm_t *vm, int n_bytes) {
  allocate(vm, n_bytes);
  return GC_memalign(8, n_bytes);
}
