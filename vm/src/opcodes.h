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
    OP_GREATER_THAN,
    OP_LOAD_STRING,
    OP_FILE_PWD,
    OP_CONCAT,
    OP_FILE_LS,
    OP_CAPTURE,
    OP_CALL_LOCAL,
    OP_LIST_NTH,
    OP_LIST_COUNT,
    OP_LIST_SLICE,
    OP_STRING_SLICE,
    OP_CODE_LOAD,
    OP_CALL_BY_NAME,
    OP_STRING_COUNT,
    OP_STRING_CONTAINS,
    OP_TO_STRING,
    OP_ANON_FN,
};

void opcode_init(vm_t *vm);

#endif  // VM_OPCODES_H
