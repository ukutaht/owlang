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
    EXIT = 0,
    INT_STORE,
    INT_PRINT,
    TEST_EQ,
    TEST_GT,
    CALL,
    RETURN,
};

void op_exit(vm_t *in);
void op_int_store(vm_t *in);
void op_int_print(vm_t *in);
void op_test_eq(vm_t *in);
void op_test_gt(vm_t *in);
void op_call(vm_t *in);
void op_return(vm_t *in);

void opcode_init(vm_t *vm);

#endif  // VM_OPCODES_H
