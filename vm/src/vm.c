#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include "vm.h"
#include "opcodes.h"
#include "util/file.h"
#include "std/owl_code.h"

vm_t *vm_new() {
  vm_t *vm;

  vm = malloc(sizeof(struct vm));
  if (!vm)
    return NULL;
  memset(vm, '\0', sizeof(struct vm));

  // Allocate 64k for code
  vm->code = malloc(0xFFFF);
  if (vm->code == NULL) {
    return NULL;
  }
  memset(vm->code, '\0', 0xFFFF);
  vm->code_size = 0;

  vm->function_names = strings_new();
  vm->intern_pool = strings_new();
  vm->ip = 0;
  vm->current_frame = 0;
  memset(vm->functions, NO_FUNCTION, MAX_FUNCTIONS);

  opcode_init(vm);

  return vm;
}

void vm_load_module_from_file(vm_t *vm, const char *filename) {
  FILE *f = fopen(filename, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  uint8_t *bytecode = malloc(fsize);
  fread(bytecode, fsize, 1, f);
  fclose(f);

  owl_load_module(vm, bytecode, fsize);
  free(bytecode);
}

bool find_module_from_dir(char *buffer, char *dirname, const char *module_name) {
  DIR           *d;
  struct dirent *dir;
  d = opendir(dirname);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (dir->d_type == DT_REG) {
        char *found = module_name_from_filename(dir->d_name);
        if (strcmp(found, module_name) == 0) {
          sprintf(buffer, "%s/%s", dirname, dir->d_name);
          free(found);
          return true;
        } else {
          free(found);
        }
      }
    }

    closedir(d);
  }

  return false;

}

bool find_file_for_module(char *buffer, const char *module_name) {
  char *env_load_path = getenv("OWL_LOAD_PATH");
  char load_path[strlen(env_load_path)];
  strcpy(load_path, env_load_path);

  char *load_dir = strtok(load_path, ":");
  while (load_dir != NULL) {
    if(find_module_from_dir(buffer, load_dir, module_name)) {
      return true;
    }
    load_dir = strtok(NULL, ":");
  }

  return false;
}

void vm_load_module(vm_t *vm, const char *module_name) {
  char module_file[500];
  if (find_file_for_module(module_file, module_name)) {
    debug_print("Found file for module: %s\n", module_file);
    vm_load_module_from_file(vm, module_file);
  } else {
    debug_print("No file found for module: %s\n", module_name);
  }
}

void vm_run(vm_t *vm) {
  while (true) {
    int opcode = vm->code[vm->ip];

    if (vm->opcodes[opcode] != NULL)
      vm->opcodes[opcode] (vm);

  }
}

void vm_run_function(vm_t *vm, const char *function_name) {
  uint8_t function_id = strings_lookup(vm->function_names, function_name);

  if (function_id != 0) {
    uint8_t call_code[6] = {
      OP_CALL, 0, function_id, 0,
      OP_EXIT, 0
    };

    memcpy(&vm->code[vm->code_size], &call_code, 6);
    vm->ip = vm->code_size;
    vm->code_size += 6;
    vm_run(vm);
  } else {
    printf("Function %s not found\n", function_name);
    exit(1);
  }
}
