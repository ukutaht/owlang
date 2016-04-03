#define REGISTER_COUNT 10

#ifndef _Bool
#define _Bool short
#define true   1
#define false  0
#endif

#define DEBUG(...) if (getenv("DEBUG") != NULL) printf(__VA_ARGS__);

#ifndef VM_H
#define VM_H 1

// Registers can hold multiple types of values
typedef struct registers {
    union {
        unsigned int integer;
        char *string;
    } content;
    enum { INTEGER, STRING } type;
} reg_t;

struct vm;
typedef void opcode_impl(struct vm *in);

typedef struct vm {
    reg_t registers[REGISTER_COUNT]; // VM registers
    unsigned int ret_address;
    unsigned int ip;                 // Instruction pointer
    unsigned char *code;             // loaded code
    unsigned int size;               // loaded code size
    opcode_impl *opcodes[255];       // opcode lookup table
    _Bool z_flag;
    _Bool running;
} vm_t;


vm_t *vm_new(unsigned char *code, unsigned int size);

void vm_run(vm_t *vm);

void vm_free(vm_t *vm);

#endif // VM_H
