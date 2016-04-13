#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "opcodes.h"
#include "vm.h"
#include "term.h"
#include "alloc.h"

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
  frame_t curr_frame = vm->frames[vm->current_frame];

  return curr_frame.registers[reg];
}

owl_term get_reg(vm_t *vm, uint8_t reg) {
  frame_t curr_frame = vm->frames[vm->current_frame];

  return curr_frame.registers[reg];
}

void set_reg(vm_t *vm, uint8_t reg, owl_term term) {
  frame_t *curr_frame = &vm->frames[vm->current_frame];
  curr_frame->registers[reg] = term;
}

void op_unknown(vm_t * vm) {
    int instruction = vm->code[vm->ip];
    printf("%04X - op_unknown(%02X)\n", vm->ip, instruction);

    vm->ip += 1;
}

void op_exit(vm_t *vm) {
    debug_print("Exiting%s", "\n");
    vm->running = false;
    vm->ip += 1;
}

void op_store(vm_t *vm) {
    unsigned int reg = next_reg(vm);
    unsigned int value = next_int(vm);

    debug_print("STORE_INT(Reg:%02x) => %d\n", reg, value);

    frame_t *curr_frame = &vm->frames[vm->current_frame];
    curr_frame->registers[reg] = value;

    vm->ip += 1;
}

void op_print(struct vm *vm) {
    unsigned int reg = next_reg(vm);

    debug_print("INT_PRINT(Reg:%02x)\n", reg);

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
    signed char instr  = next_byte(vm);

    debug_print("TEST_EQ(Reg1: %02x, Reg2: %02x, IfTrue:%02x)\n", reg1, reg2, instr);

    int val1 = get_int_reg(vm, reg1);
    int val2 = get_int_reg(vm, reg2);

    if (val1 == val2) {
      vm->ip += instr;
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

    debug_print("TEST_GT(Reg1: %02x, Reg2: %02x, IfTrue:%d)\n", reg1, reg2, instr);

    int val1 = get_int_reg(vm, reg1);
    int val2 = get_int_reg(vm, reg2);

    if (val1 > val2) {
      vm->ip += instr;
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

    debug_print("TEST_GTE(Reg1: %02x, Reg2: %02x, IfTrue:%d)\n", reg1, reg2, instr);

    int val1 = get_int_reg(vm, reg1);
    int val2 = get_int_reg(vm, reg2);

    if (val1 >= val2) {
      vm->ip += instr;
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

    debug_print("TEST_LT(Reg1: %02x, Reg2: %02x, IfTrue:%d)\n", reg1, reg2, instr);

    int val1 = get_int_reg(vm, reg1);
    int val2 = get_int_reg(vm, reg2);

    if (val1 < val2) {
      vm->ip += instr;
    } else {
      vm->ip += 1;
    }
}

// Tests if the first register is less than or equal to other
// If true, continue running. If not equal,
// jumps to the provided label
void op_test_lte(struct vm *vm) {
    unsigned int reg1  = next_reg(vm);
    unsigned int reg2  = next_reg(vm);
    unsigned int instr = next_byte(vm);

    debug_print("TEST_LTE(Reg1: %02x, Reg2: %02x, IfTrue:%d)\n", reg1, reg2, instr);

    int val1 = get_int_reg(vm, reg1);
    int val2 = get_int_reg(vm, reg2);

    if (val1 <= val2) {
      vm->ip += instr;
    } else {
      vm->ip += 1;
    }
}

// Adds regsiters 2 and 3. Stores result in 1
void op_add(struct vm *vm) {
    unsigned int reg1  = next_reg(vm);
    unsigned int reg2  = next_reg(vm);
    unsigned int reg3  = next_reg(vm);

    debug_print("ADD(Reg1: %02x, Reg2: %02x, REG3:%02x)\n", reg1, reg2, reg3);

    int val1 = get_int_reg(vm, reg2);
    int val2 = get_int_reg(vm, reg3);
    int result = val1 + val2;

    frame_t *curr_frame = &vm->frames[vm->current_frame];

    curr_frame->registers[reg1] = result;

    vm->ip += 1;
}

// Subtracts regsiters 2 and 3. Stores result in 1
void op_sub(struct vm *vm) {
    unsigned int reg1  = next_reg(vm);
    unsigned int reg2  = next_reg(vm);
    unsigned int reg3  = next_reg(vm);

    debug_print("SUB(Reg1: %02x, Reg2: %02x, REG3:%02x)\n", reg1, reg2, reg3);

    int val1 = get_int_reg(vm, reg2);
    int val2 = get_int_reg(vm, reg3);
    int result = val1 - val2;

    frame_t *curr_frame = &vm->frames[vm->current_frame];
    curr_frame->registers[reg1] = result;

    vm->ip += 1;
}

void op_call(struct vm *vm) {
    unsigned int ret_reg = next_byte(vm);
    unsigned int location = next_byte(vm);
    unsigned int arity = next_byte(vm);

    debug_print("CALL(Instr:%d, Arity: %d)\n", location, arity);

    assert(vm->current_frame + 1 <= STACK_DEPTH);

    unsigned int next_frame = vm->current_frame + 1;

    for(unsigned int i = 0; i < arity; i++) {
      unsigned int arg = get_int_reg(vm, next_reg(vm));
      vm->frames[next_frame].registers[i + 1] = arg;
    }

    vm->frames[next_frame].ret_address = vm->ip + 1;
    vm->frames[next_frame].ret_register = ret_reg;
    vm->current_frame += 1;

    vm->ip = location;
}

void op_tailcall(struct vm *vm) {
    unsigned int location = next_byte(vm);
    unsigned int arity = next_byte(vm);

    debug_print("TAILCALL(Instr:%d, Arity: %d)\n", location, arity);

    vm->ip = location;
}

void op_return(struct vm *vm) {
    debug_print("RETURN%s", "\n");

    frame_t *curr_frame = &vm->frames[vm->current_frame];
    frame_t *prev_frame = &vm->frames[vm->current_frame - 1];
    unsigned int ret_address = curr_frame->ret_address;

    prev_frame->registers[curr_frame->ret_register] = curr_frame->registers[0];

    vm->current_frame -= 1;
    vm->ip = ret_address;
}

void op_mov(struct vm *vm) {
    unsigned int reg1 = next_reg(vm);
    unsigned int reg2 = next_reg(vm);

    debug_print("MOV(Reg1: %02x, Reg2: %02x)\n", reg1, reg2);

    frame_t *curr_frame = &vm->frames[vm->current_frame];
    curr_frame->registers[reg1] = curr_frame->registers[reg2];

    vm->ip += 1;
}

void op_jmp(struct vm *vm) {
    unsigned int loc = next_byte(vm);

    debug_print("JMP(Loc: %d)\n", loc);

    vm->ip = loc;
}

void op_tuple(struct vm *vm) {
    uint8_t reg  = next_byte(vm);
    uint8_t size = next_byte(vm);

    debug_print("TUPLE(Reg: %d, Size: %d)\n", reg, size);

    owl_term *ary = owl_alloc(sizeof(owl_term) * (size + 1));
    ary[0] = size;

    for(uint8_t i = 1; i <= size; i++) {
      ary[i] = get_reg(vm, next_byte(vm));
    }

    owl_term tuple = (owl_term) ary;
    owl_term tagged_tuple =  (tuple << 3) | TUPLE;

    frame_t *curr_frame = &vm->frames[vm->current_frame];
    curr_frame->registers[reg] = tagged_tuple;

    vm->ip += 1;
}

void op_tuple_nth(struct vm *vm) {
    uint8_t reg = next_byte(vm);
    uint8_t tuple = next_byte(vm);
    uint8_t index = next_byte(vm);

    debug_print("TUPLE_NTH(Reg: %d, Tuple: %d, Index: %d)\n", reg, tuple, index);


    owl_term *ary = owl_extract_ptr(get_reg(vm, tuple));
    owl_term elem = ary[get_reg(vm, index) + 1];
    set_reg(vm, reg, elem);

    vm->ip += 1;
}

void opcode_init(vm_t * vm) {
    // All instructions will default to unknown.
    for (int i = 0; i < 255; i++)
        vm->opcodes[i] = op_unknown;

    vm->opcodes[OP_EXIT] = op_exit;
    vm->opcodes[OP_STORE] = op_store;
    vm->opcodes[OP_PRINT] = op_print;
    vm->opcodes[OP_TEST_EQ] = op_test_eq;
    vm->opcodes[OP_TEST_GT] = op_test_gt;
    vm->opcodes[OP_TEST_GTE] = op_test_gte;
    vm->opcodes[OP_TEST_LT] = op_test_lt;
    vm->opcodes[OP_TEST_LTE] = op_test_lte;
    vm->opcodes[OP_ADD] = op_add;
    vm->opcodes[OP_SUB] = op_sub;
    vm->opcodes[OP_CALL] = op_call;
    vm->opcodes[OP_RETURN] = op_return;
    vm->opcodes[OP_MOV] = op_mov;
    vm->opcodes[OP_TAILCALL] = op_tailcall;
    vm->opcodes[OP_JMP] = op_jmp;
    vm->opcodes[OP_TUPLE]     = op_tuple;
    vm->opcodes[OP_TUPLE_NTH] = op_tuple_nth;
}
