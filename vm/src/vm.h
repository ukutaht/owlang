#ifndef VM_H
#define VM_H

#include "owl.h"

vm_t *vm_new();
void vm_load_module_from_file(vm_t *vm, const char *filename);
void vm_load_module(vm_t *vm, const char *module_name);
void vm_run_function(vm_t *vm, const char *function_name);

#endif // VM_H
