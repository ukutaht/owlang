#include <gc/gc.h>
#include "alloc.h"

void* owl_alloc(vm_t *vm, int n_bytes) {
  return GC_memalign(8, n_bytes);
}
