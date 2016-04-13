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
    OP_STORE,
    OP_PRINT,
    OP_TEST_EQ,
    OP_TEST_GT,
    OP_TEST_GTE,
    OP_TEST_LT,
    OP_TEST_LTE,
    OP_ADD,
    OP_SUB,
    OP_CALL,
    OP_RETURN,
    OP_MOV,
    OP_TAILCALL,
    OP_JMP,
    OP_TUPLE,
    OP_TUPLE_NTH,
    OP_ASSERT_EQ,
};

void opcode_init(vm_t *vm);

#endif  // VM_OPCODES_H
