#ifndef ALLOC_H
#define ALLOC_H

#include "owl.h"

void gc_collect(vm_t *vm);
void gc_safepoint(vm_t *vm);
uint32_t gc_usage(vm_t *vm);
void* owl_alloc(vm_t *vm, uint32_t n_bytes);

#endif  // ALLOC_H
