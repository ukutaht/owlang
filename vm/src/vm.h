#include <stdbool.h>
#include <stdio.h>
#include <intern/strings.h>

#include "term.h"

#ifndef VM_H
#define VM_H 1

#define REGISTER_COUNT 100
#define STACK_DEPTH 100
#define MAX_FUNCTIONS 255

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
    unsigned int ip;             // Instruction pointer
    unsigned char *code;         // Loaded code
    opcode_impl *opcodes[255];   // Opcode lookup table
    struct strings *function_names;      // Interned function names
    uint64_t functions[MAX_FUNCTIONS];   // Function lookup table
    bool running;
} vm_t;


vm_t *vm_new();

void vm_load_module_from_file(vm_t *vm, const char *filename);
void vm_run(vm_t *vm);

#endif // VM_H
