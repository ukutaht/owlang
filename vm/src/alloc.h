#ifndef ALLOC_H
#define ALLOC_H

#include "owl.h"

void collect(vm_t *vm);
void** gc_protect(vm_t *vm, void* item);
void* gc_unprotect(vm_t *vm);
void* owl_alloc(vm_t *vm, int n_bytes);

#endif  // ALLOC_H
