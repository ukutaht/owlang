#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <gc/gc.h>

#include "vm.h"
#include "opcodes.h"
#include "util/file.h"

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
  char ch;

  FILE *fp = fopen(filename, "rb");

  if (!fp) {
      printf("Failed to open program-file %s\n", filename);
      exit(1);
  }

  unsigned char *code_ptr = vm->code + vm->code_size;

  while ((ch = fgetc(fp)) != EOF ) {
    switch(ch) {
      default:
        printf("Unknown opcode: 0x%02x\n", ch);
        exit(1);
      case OP_RETURN:
        *code_ptr++ = ch;
        vm->code_size += 1;
        break;
      case OP_EXIT:
      case OP_PRINT:
      case OP_FILE_PWD:
      case OP_STORE_TRUE:
      case OP_STORE_FALSE:
      case OP_STORE_NIL:
      case OP_JMP:
        *code_ptr++ = ch;
        *code_ptr++ = fgetc(fp);
        vm->code_size += 2;
        break;
      case OP_MOV:
      case OP_FILE_LS:
      case OP_NOT:
      case OP_TEST:
        *code_ptr++ = ch;
        *code_ptr++ = fgetc(fp);
        *code_ptr++ = fgetc(fp);
        vm->code_size += 3;
        break;
      case OP_STORE_INT:
      case OP_ADD:
      case OP_SUB:
      case OP_EQ:
      case OP_NOT_EQ:
      case OP_GREATER_THAN:
      case OP_TUPLE_NTH:
      case OP_CONCAT:
        *code_ptr++ = ch;
        *code_ptr++ = fgetc(fp);
        *code_ptr++ = fgetc(fp);
        *code_ptr++ = fgetc(fp);
        vm->code_size += 4;
        break;
      case OP_TUPLE:
      case OP_LIST:
        *code_ptr++ = ch;
        *code_ptr++ = fgetc(fp);
        uint8_t size = fgetc(fp);
        *code_ptr++ = size;
        vm->code_size += 3;
        for (int i = 0; i < size; i++) {
          *code_ptr++ = fgetc(fp);
          vm->code_size += 1;
        }
        break;
      case OP_CAPTURE: {
        *code_ptr++ = OP_CAPTURE;
        *code_ptr++ = fgetc(fp); // ret loc

        uint8_t name_size = fgetc(fp);
        char name[name_size];
        fread(&name, name_size, 1, fp);
        uint64_t id = strings_intern(vm->function_names, name);

        *code_ptr++ = (uint8_t) id;
        vm->code_size += 3;
        break;
        }
      case OP_PUB_FN: {
        uint8_t name_size = fgetc(fp);
        char name[name_size];
        fread(&name, name_size, 1, fp);
        uint64_t id = strings_intern(vm->function_names, name);
        uint64_t instruction = (uint64_t) (code_ptr - vm->code);

        vm->functions[id] = instruction;
        break;
        }
      case OP_LOAD_STRING: {
        *code_ptr++ = OP_LOAD_STRING;
        *code_ptr++ = fgetc(fp); // ret loc

        uint8_t size = fgetc(fp);
        char str[size];
        fread(&str, size, 1, fp);
        uint64_t id = strings_intern(vm->intern_pool, str);
        *code_ptr++ = (uint8_t) id; // string_id (needs to be bigger than 8 bit int)
        vm->code_size += 3;
        break;
      }
      case OP_CALL_LOCAL: {
        *code_ptr++ = OP_CALL_LOCAL;
        *code_ptr++ = fgetc(fp); // ret loc
        *code_ptr++ = fgetc(fp); // function_loc

        uint8_t func_arity = fgetc(fp);
        *code_ptr++ = func_arity;
        vm->code_size += 4;

        for (int i = 0; i < func_arity; i++) {
          *code_ptr++ = fgetc(fp); // arguments
          vm->code_size += 1;
        }
        break;
      }
      case OP_CALL:
        *code_ptr++ = OP_CALL;
        *code_ptr++ = fgetc(fp); // ret loc

        uint8_t name_size = fgetc(fp);
        char name[name_size];
        fread(&name, name_size, 1, fp);
        uint64_t id = strings_intern(vm->function_names, name);
        *code_ptr++ = (uint8_t) id; // function_id (needs to be bigger than 8 bit int)

        uint8_t arity = fgetc(fp);
        *code_ptr++ = arity;
        vm->code_size += 4;

        for (int i = 0; i < arity; i++) {
          *code_ptr++ = fgetc(fp); // arguments
          vm->code_size += 1;
        }
        break;
    }
  }
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
  GC_init();

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
