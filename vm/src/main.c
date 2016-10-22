#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "util/file.h"
#include "vm.h"

static void init_load_path() {
  char *load_path = getenv("OWL_LOAD_PATH");
  char *this_dir = dirname(__FILE__);
  char *relative_stdlib = "../../.build/stdlib";

  char absolute_stdlib[strlen(this_dir) + strlen(relative_stdlib) + 2];
  sprintf(absolute_stdlib, "%s/%s", this_dir, relative_stdlib);

  if (load_path == NULL) {
    setenv("OWL_LOAD_PATH", absolute_stdlib, true);
  } else {
    char final_path[strlen(load_path) + strlen(absolute_stdlib) + 1];
    sprintf(final_path, "%s:%s", load_path, absolute_stdlib);

    setenv("OWL_LOAD_PATH", final_path, true);
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s input-file\n", argv[0]);
    return 0;
  }

  init_load_path();

  vm_t *vm = vm_new();

  vm_load_module_from_file(vm, argv[1]);

  char *main_module = module_name_from_filename(argv[1]);
  char main_function[strlen(main_module) + 8];
  sprintf(main_function, "%s.main\\0", main_module);

  vm_run_function(vm, main_function);
  free(main_module);

  return 0;
}
