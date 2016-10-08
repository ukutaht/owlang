#ifndef OWL_H
#define OWL_H

#define REGISTER_COUNT 100
#define STACK_DEPTH 300
#define MAX_FUNCTIONS 255
#define NO_FUNCTION UINT64_MAX
#define MAX_UPVALUES 128

#define DEBUG false
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)


#include <stdbool.h>
#include <stdint.h>
#include <intern/strings.h>
#include "stack.h"

typedef uint64_t owl_term;
typedef struct vm vm_t;

// pointer: 000
// int:     001
// tuple:   010
// list:    011
// string:  100
typedef enum owl_tag {
    POINTER = 0,
    INT,
    TUPLE,
    LIST,
    STRING,
    FUNCTION,
} owl_tag;

typedef struct GCState {
  uint64_t* mem;
  void* to_space;
  void* from_space;
  void* alloc_ptr;
  uint64_t size;
  stack* protect;
} GCState;

typedef struct Function {
  uint64_t location;
  const char* name;
  bool on_gc_heap;
  owl_term upvalues[128];
} Function;

typedef struct frame_t {
    unsigned int ret_address;
    unsigned int ret_register;
    owl_term registers[REGISTER_COUNT]; // Each frame has their own registers
} frame_t;

typedef void opcode_impl(vm_t *in);

struct vm {
    frame_t frames[STACK_DEPTH];
    unsigned int current_frame;
    unsigned int ip;                     // Instruction pointer
    uint8_t *code;                       // Loaded code
    uint64_t code_size;                  // Loaded code size
    opcode_impl *opcodes[255];           // Opcode lookup table
    struct strings *function_names;      // Interned function names
    struct strings *intern_pool;         // General intern pool
    Function* functions[MAX_FUNCTIONS];   // Function lookup table
    Function* current_function;
    GCState* gc;
};


#endif // VM_H
