#include "vm.h"

#ifndef VM_OPCODES_H
#define VM_OPCODES_H 1


/**
 * This enum stores the values of our opcodes.
 *
 * These could be injected at compile-time using the preprocessor
 * but an enum is less painful to update than having a series of #define instructions,
 * and ensure that we have unique and incrementing values for the various
 * opcodes.
 *
 */
enum opcode_values {
    OP_EXIT = 0,
    OP_STORE_INT,
    OP_PRINT,
    OP_ADD,
    OP_SUB,
    OP_CALL,
    OP_RETURN,
    OP_MOV,
    OP_JMP,
    OP_TUPLE,
    OP_TUPLE_NTH,
    OP_LIST,
    OP_PUB_FN,
    OP_STORE_TRUE,
    OP_STORE_FALSE,
    OP_TEST,
    OP_EQ,
    OP_NOT_EQ,
    OP_NOT,
    OP_STORE_NIL,
};

void opcode_init(vm_t *vm);

#endif  // VM_OPCODES_H
