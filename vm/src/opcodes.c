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

// Compare contents of two registers for equality
// If they are equal, sets the `z_flag` to true on the vm
void op_cmp(struct vm *vm) {
    unsigned int reg1 = next_reg(vm);
    unsigned int reg2 = next_reg(vm);

    DEBUG("CMP(Reg1:%02x, Reg2:%02x)\n", reg1, reg2);

    /* get the register contents. */
    int val1 = get_int_reg(vm, reg1);
    int val2 = get_int_reg(vm, reg2);

    if (val1 == val2) {
      vm->z_flag = true;
    }

    vm->ip += 1;
}

// Jumps to the given instruction if the z-flag is true
void op_jmpz(struct vm *vm) {
    unsigned int instr = next_int(vm);

    DEBUG("JMPZ(Instr:%02x)\n", instr);

    if (vm->z_flag == true) {
      vm->ip = instr;
    } else {
      vm->ip += 1;
    }
}


void op_call(struct vm *vm) {
    unsigned int location = next_byte(vm);
    unsigned int arity = next_byte(vm);

    DEBUG("CALL(Instr:%d, Arity: %d)\n", location, arity);

    vm->ip = location;
}

void opcode_init(vm_t * vm) {
    // All instructions will default to unknown.
    for (int i = 0; i < 255; i++)
        vm->opcodes[i] = op_unknown;

    vm->opcodes[EXIT] = op_exit;
    vm->opcodes[INT_STORE] = op_int_store;
    vm->opcodes[INT_PRINT] = op_int_print;
    vm->opcodes[CMP] = op_cmp;
    vm->opcodes[JMPZ] = op_jmpz;
    vm->opcodes[CALL] = op_call;
}
