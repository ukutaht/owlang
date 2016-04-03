#include "vm.h"

#ifndef VM_OPCODES_H
#define VM_OPCODES_H 1


/**
 * This enum stores the values of our opcodes.
 *
 * These could be injected at compile-time using the preprocessor
 *
 * This is less painful to update than having a series of #define instructions,
 * and ensure that we have unique and incrementing values for the various
 * opcodes.
 *
 */
enum opcode_values {
    EXIT = 0,
    INT_STORE,
    INT_PRINT,
    TEST_EQ,
    CALL
};

void op_exit(vm_t *in);
void op_int_store(vm_t *in);
void op_int_print(vm_t *in);
void op_test_eq(vm_t *in);
void op_call(vm_t *in);

/**
 * Initialization function.
 */
void opcode_init(vm_t *vm);

#endif  // VM_OPCODES_H
