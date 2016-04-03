#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "opcodes.h"
#include "vm.h"

// Read and return the next byte from the current instruction-pointer.
// This function ensures that reading will wrap around the address-space
// of the virtual CPU.
unsigned char next_byte(vm_t *vm) {
    vm->ip += 1;

    if (vm->ip >= 0xFFFF)
        vm->ip = 0;

    return (vm->code[vm->ip]);
}

// Read and return the next byte from the current instruction-pointer
// as a register number. This is used when we expect an argument op
// to describe a register
unsigned int next_reg(vm_t *vm) {
    unsigned int reg = next_byte(vm);
    assert(reg < REGISTER_COUNT);
    return reg;
}

// Read and return the next int from the current instruction-pointer
// Since the bytecode is read one byte at a time, integers larger
// than 256 must be encoded using two bytes e.g
// 8      => 8,   0
// 356    => 100, 1
// 65,792 => 256, 256
unsigned int next_int(vm_t *vm) {
    unsigned int val1 = next_byte(vm);
    unsigned int val2 = next_byte(vm);

    return val1 + (256 * val2);
}

// Helper to return the integer-content of a register.
int get_int_reg(vm_t *vm, int reg) {
  assert(vm->registers[reg].type == INTEGER);

  return vm->registers[reg].content.integer;
}

void op_unknown(vm_t * vm) {
    int instruction = vm->code[vm->ip];
    printf("%04X - op_unknown(%02X)\n", vm->ip, instruction);

    vm->ip += 1;
}

void op_exit(vm_t *vm) {
    DEBUG("Exiting\n");
    vm->running = false;
    vm->ip += 1;
}

void op_int_store(vm_t *vm) {
    unsigned int reg = next_reg(vm);
    unsigned int value = next_int(vm);

    DEBUG("STORE_INT(Reg:%02x) => %d\n", reg, value);

    /* if the register stores a string .. free it */
    if ((vm->registers[reg].type == STRING) && (vm->registers[reg].content.string))
        free(vm->registers[reg].content.string);

    vm->registers[reg].content.integer = value;
    vm->registers[reg].type = INTEGER;

    vm->ip += 1;
}

void op_int_print(struct vm *vm) {
    unsigned int reg = next_reg(vm);

    DEBUG("INT_PRINT(Reg:%02x)\n", reg);

    /* get the register contents. */
    int val = get_int_reg(vm, reg);

    printf("%d\n", val);

    vm->ip += 1;
}

// Tests if two registers are equal
// If they are equal, continue running. If not equal,
// jumps to the provided label
void op_test_eq(struct vm *vm) {
    unsigned int reg1  = next_reg(vm);
    unsigned int reg2  = next_reg(vm);
    unsigned int instr = next_byte(vm);

    DEBUG("TEST_EQ(Reg1: %02x, Reg2: %02x, IfTrue:%02x)\n", reg1, reg2, instr);

    int val1 = get_int_reg(vm, reg1);
    int val2 = get_int_reg(vm, reg2);

    if (val1 == val2) {
      vm->ip = instr;
    } else {
      vm->ip += 1;
    }
}

// Tests if the first register is greater than other
// If true, continue running. If not equal,
// jumps to the provided label
void op_test_gt(struct vm *vm) {
    unsigned int reg1  = next_reg(vm);
    unsigned int reg2  = next_reg(vm);
    unsigned int instr = next_byte(vm);

    DEBUG("TEST_GT(Reg1: %02x, Reg2: %02x, IfTrue:%d)\n", reg1, reg2, instr);

    int val1 = get_int_reg(vm, reg1);
    int val2 = get_int_reg(vm, reg2);

    if (val1 > val2) {
      vm->ip = instr;
    } else {
      vm->ip += 1;
    }
}

// Tests if the first register is greater than or equal to other
// If true, continue running. If not equal,
// jumps to the provided label
void op_test_gte(struct vm *vm) {
    unsigned int reg1  = next_reg(vm);
    unsigned int reg2  = next_reg(vm);
    unsigned int instr = next_byte(vm);

    DEBUG("TEST_GTE(Reg1: %02x, Reg2: %02x, IfTrue:%d)\n", reg1, reg2, instr);

    int val1 = get_int_reg(vm, reg1);
    int val2 = get_int_reg(vm, reg2);

    if (val1 >= val2) {
      vm->ip = instr;
    } else {
      vm->ip += 1;
    }
}

// Tests if the first register is less than other
// If true, continue running. If not equal,
// jumps to the provided label
void op_test_lt(struct vm *vm) {
    unsigned int reg1  = next_reg(vm);
    unsigned int reg2  = next_reg(vm);
    unsigned int instr = next_byte(vm);

    DEBUG("TEST_LT(Reg1: %02x, Reg2: %02x, IfTrue:%d)\n", reg1, reg2, instr);

    int val1 = get_int_reg(vm, reg1);
    int val2 = get_int_reg(vm, reg2);

    if (val1 < val2) {
      vm->ip = instr;
    } else {
      vm->ip += 1;
    }
}

void op_call(struct vm *vm) {
    unsigned int location = next_byte(vm);
    unsigned int arity = next_byte(vm);

    DEBUG("CALL(Instr:%d, Arity: %d)\n", location, arity);

    vm->ret_address = vm->ip + 1;
    vm->ip = location;
}

void op_return(struct vm *vm) {
    DEBUG("RETURN\n");

    vm->ip = vm->ret_address;
}

void opcode_init(vm_t * vm) {
    // All instructions will default to unknown.
    for (int i = 0; i < 255; i++)
        vm->opcodes[i] = op_unknown;

    vm->opcodes[EXIT] = op_exit;
    vm->opcodes[INT_STORE] = op_int_store;
    vm->opcodes[INT_PRINT] = op_int_print;
    vm->opcodes[TEST_EQ] = op_test_eq;
    vm->opcodes[TEST_GT] = op_test_gt;
    vm->opcodes[TEST_GTE] = op_test_gte;
    vm->opcodes[TEST_LT] = op_test_lt;
    vm->opcodes[CALL] = op_call;
    vm->opcodes[RETURN] = op_return;
}
