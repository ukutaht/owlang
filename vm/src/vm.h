#include <stdbool.h>

#define REGISTER_COUNT 100
#define STACK_DEPTH 100

#define DEBUG true
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

#ifndef VM_H
#define VM_H 1

typedef int ir_term;

typedef struct frame_t {
    ir_term registers[REGISTER_COUNT]; // Each frame has their own registers
    unsigned int ret_address;
} frame_t;

struct vm;
typedef void opcode_impl(struct vm *in);

typedef struct vm {
    frame_t frames[STACK_DEPTH];
    unsigned int current_frame;
    unsigned int ip;                 // Instruction pointer
    unsigned char *code;             // loaded code
    unsigned int size;               // loaded code size
    opcode_impl *opcodes[255];       // opcode lookup table
    bool running;
} vm_t;


vm_t *vm_new(unsigned char *code, unsigned int size);

void vm_run(vm_t *vm);

void vm_free(vm_t *vm);

#endif // VM_H
