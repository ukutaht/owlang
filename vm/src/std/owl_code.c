#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

#include "opcodes.h"
#include "vm.h"
#include "alloc.h"
#include "std/owl_code.h"
#include "std/owl_list.h"
#include "std/owl_string.h"
#include "util/file.h"


typedef struct scanner_t {
    uintptr_t index;
    uintptr_t size;
    uint8_t *mem;
} scanner_t;

scanner_t *scanner_new(uintptr_t size, uint8_t *mem) {
  scanner_t *scanner = malloc(sizeof(scanner_t));
  scanner->index = 0;
  scanner->size = size;
  scanner->mem = mem;
  return scanner;
}

bool scanner_has_next(scanner_t *scanner) {
  return scanner->index < scanner->size;
}

uint8_t scanner_next(scanner_t *scanner) {
  return scanner->mem[scanner->index++];
}

void scanner_read(void * address, uintptr_t size, scanner_t *scanner) {
  memcpy(address, &scanner->mem[scanner->index], size);
  scanner->index += size;
}

owl_term owl_load_module(vm_t *vm, uint8_t *bytecode, size_t size) {
  uint8_t ch;

  scanner_t *scanner = scanner_new(size, bytecode);
  owl_term function_list = owl_list_init();
  unsigned char *code_ptr = vm->code + vm->code_size;

  while (scanner_has_next(scanner)) {
    ch = scanner_next(scanner);

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
        *code_ptr++ = scanner_next(scanner);
        vm->code_size += 2;
        break;
      case OP_MOV:
      case OP_FILE_LS:
      case OP_NOT:
      case OP_LIST_COUNT:
      case OP_STRING_COUNT:
      case OP_CODE_LOAD:
      case OP_TEST:
        *code_ptr++ = ch;
        *code_ptr++ = scanner_next(scanner);
        *code_ptr++ = scanner_next(scanner);
        vm->code_size += 3;
        break;
      case OP_TUPLE_NTH:
      case OP_STORE_INT:
      case OP_ADD:
      case OP_SUB:
      case OP_EQ:
      case OP_NOT_EQ:
      case OP_GREATER_THAN:
      case OP_LIST_NTH:
      case OP_CONCAT:
      case OP_STRING_CONTAINS:
      case OP_CALL_BY_NAME:
        *code_ptr++ = ch;
        *code_ptr++ = scanner_next(scanner);
        *code_ptr++ = scanner_next(scanner);
        *code_ptr++ = scanner_next(scanner);
        vm->code_size += 4;
        break;
      case OP_LIST_SLICE:
      case OP_STRING_SLICE:
        *code_ptr++ = ch;
        *code_ptr++ = scanner_next(scanner);
        *code_ptr++ = scanner_next(scanner);
        *code_ptr++ = scanner_next(scanner);
        *code_ptr++ = scanner_next(scanner);
        vm->code_size += 5;
        break;
      case OP_TUPLE:
      case OP_LIST:
        *code_ptr++ = ch;
        *code_ptr++ = scanner_next(scanner);
        uint8_t size = scanner_next(scanner);
        *code_ptr++ = size;
        vm->code_size += 3;
        for (int i = 0; i < size; i++) {
          *code_ptr++ = scanner_next(scanner);
          vm->code_size += 1;
        }
        break;
      case OP_CAPTURE: {
        *code_ptr++ = OP_CAPTURE;
        *code_ptr++ = scanner_next(scanner); // ret loc

        uint8_t name_size = scanner_next(scanner);
        char name[name_size];
        scanner_read(name, name_size, scanner);
        uint64_t id = strings_intern(vm->function_names, name);

        *code_ptr++ = (uint8_t) id;
        vm->code_size += 3;
        break;
        }
      case OP_PUB_FN: {
        uint8_t name_size = scanner_next(scanner);
        char name[name_size];
        scanner_read(name, name_size, scanner);
        uint64_t id = strings_intern(vm->function_names, name);
        uint64_t instruction = (uint64_t) (code_ptr - vm->code);

        const char *function_name = strings_lookup_id(vm->function_names, id);
        owl_term owl_name = owl_string_from(function_name);
        function_list = owl_list_push(function_list, owl_name);

        assert(id < MAX_FUNCTIONS);
        vm->functions[id] = instruction;
        break;
        }
      case OP_LOAD_STRING: {
        *code_ptr++ = OP_LOAD_STRING;
        *code_ptr++ = scanner_next(scanner); // ret loc

        uint8_t size = scanner_next(scanner);
        char str[size];
        scanner_read(str, size, scanner);
        uint64_t id = strings_intern(vm->intern_pool, str);
        assert(id < UINT8_MAX);
        *code_ptr++ = (uint8_t) id; // string_id (needs to be bigger than 8 bit int)
        vm->code_size += 3;
        break;
      }
      case OP_CALL_LOCAL: {
        *code_ptr++ = OP_CALL_LOCAL;
        *code_ptr++ = scanner_next(scanner); // ret loc
        *code_ptr++ = scanner_next(scanner); // function_loc

        uint8_t func_arity = scanner_next(scanner);
        *code_ptr++ = func_arity;
        vm->code_size += 4;

        for (int i = 0; i < func_arity; i++) {
          *code_ptr++ = scanner_next(scanner); // arguments
          vm->code_size += 1;
        }
        break;
      }
      case OP_CALL:
        *code_ptr++ = OP_CALL;
        *code_ptr++ = scanner_next(scanner); // ret loc

        uint8_t name_size = scanner_next(scanner);
        char name[name_size];
        scanner_read(&name, name_size, scanner);
        uint64_t id = strings_intern(vm->function_names, name);
        assert(id < MAX_FUNCTIONS);
        *code_ptr++ = (uint8_t) id; // function_id (needs to be bigger than 8 bit int)

        uint8_t arity = scanner_next(scanner);
        *code_ptr++ = arity;
        vm->code_size += 4;

        for (int i = 0; i < arity; i++) {
          *code_ptr++ = scanner_next(scanner); // arguments
          vm->code_size += 1;
        }
        break;
    }
  }
  free(scanner);
  return function_list;
}

owl_term owl_code_load(vm_t *vm, owl_term owl_filename) {
  char *filename = owl_extract_ptr(owl_filename);
  compiled_module_t *compiled = compile_file_to_memory(filename);
  owl_term functions = owl_load_module(vm, compiled->bytecode, compiled->size);
  free_module(compiled);
  return functions;
}

