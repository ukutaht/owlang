#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>

#include "vm.h"

void init_load_path() {
  char *load_path = getenv("OWL_LOAD_PATH");
  char *this_dir = dirname(__FILE__);
  char *relative_stdlib = "../../stdlib/.build";

  char absolute_stdlib[strlen(this_dir) + strlen(relative_stdlib) + 1];
  sprintf(absolute_stdlib, "%s/%s", this_dir, relative_stdlib);

  char final_path[strlen(load_path) + strlen(absolute_stdlib) + 1];
  sprintf(final_path, "%s:%s", load_path, absolute_stdlib);

  setenv("OWL_LOAD_PATH", final_path, true);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s input-file\n", argv[0]);
    return 0;
  }

  init_load_path();

  vm_t *vm = vm_new();

  vm_load_module_from_file(vm, argv[1]);

  vm_run(vm);

  return 0;
}
