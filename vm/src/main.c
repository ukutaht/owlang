#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "vm.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s input-file\n", argv[0]);
    return 0;
  }

  vm_t *vm = vm_new();

  vm_load_module_from_file(vm, argv[1]);

  vm_run(vm);

  return 0;
}
