#ifndef VM_H
#define VM_H

#include <stdbool.h>
#include <stdio.h>
#include <intern/strings.h>

#include "term.h"
#include "std/owl_function.h"

#define REGISTER_COUNT 100
#define STACK_DEPTH 300
#define MAX_FUNCTIONS 255
#define NO_FUNCTION UINT64_MAX

#define DEBUG false
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

typedef struct frame_t {
    unsigned int ret_address;
    unsigned int ret_register;
    owl_term registers[REGISTER_COUNT]; // Each frame has their own registers
} frame_t;

struct vm;
typedef void opcode_impl(struct vm *in);

typedef struct vm {
    frame_t frames[STACK_DEPTH];
    unsigned int current_frame;
    unsigned int ip;                    // Instruction pointer
    uint8_t *code;                      // Loaded code
    uint64_t code_size;                 // Loaded code size
    opcode_impl *opcodes[255];          // Opcode lookup table
    struct strings *function_names;     // Interned function names
    struct strings *intern_pool;        // General intern pool
    Function* functions[MAX_FUNCTIONS]; // Function lookup table
    Function* current_function;
} vm_t;


vm_t *vm_new();

void vm_load_module_from_file(vm_t *vm, const char *filename);
void vm_load_module(vm_t *vm, const char *module_name);
void vm_run_function(vm_t *vm, const char *function_name);

#endif // VM_H
