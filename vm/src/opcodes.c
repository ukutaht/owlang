#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "opcodes.h"
#include "vm.h"
#include "term.h"
#include "alloc.h"
#include "vector.h"

// Read and return the next byte from the current instruction-pointer.
uint8_t next_byte(vm_t *vm) {
  vm->ip += 1;

  return (vm->code[vm->ip]);
}

// Read and return the next byte from the current instruction-pointer
// as a register number. This is used when we expect an argument op
// to describe a register
uint8_t next_reg(vm_t *vm) {
  uint8_t reg = next_byte(vm);
  assert(reg < REGISTER_COUNT);
  return reg;
}

// Read and return the next int from the current instruction-pointer
// Since the bytecode is read one byte at a time, integers larger
// than 256 must be encoded using two bytes e.g
// 8      => 8,   0
// 356    => 100, 1
// 65,792 => 256, 256
owl_term next_int(vm_t *vm) {
  uint8_t val1 = next_byte(vm);
  uint8_t val2 = next_byte(vm);

  uint16_t actual_val = val1 + (256 * val2);

  return owl_int_from(actual_val);
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
  printf("%04X op_unknown(%d)\n", vm->ip, instruction);

  exit(1);
}

void op_exit(vm_t *vm) {
  debug_print("%04x OP_EXIT\n", vm->ip);
  uint8_t exit_code = next_byte(vm);

  exit(exit_code);
}

void op_store(vm_t *vm) {
  debug_print("%04x OP_STORE\n", vm->ip);
  uint8_t reg = next_reg(vm);
  owl_term value = next_int(vm);

  frame_t *curr_frame = &vm->frames[vm->current_frame];
  curr_frame->registers[reg] = value;

  vm->ip += 1;
}

void op_print(struct vm *vm) {
  debug_print("%04x OP_PRINT\n", vm->ip);
  uint8_t reg = next_reg(vm);

  owl_term val = get_reg(vm, reg);

  printf("%llu\n", int_from_owl_int(val));

  vm->ip += 1;
}

void op_test(struct vm *vm) {
  debug_print("%04x OP_TEST\n", vm->ip);
  uint8_t reg = next_reg(vm);
  uint8_t instr = next_byte(vm);

  if (owl_term_truthy(get_reg(vm, reg))) {
    vm->ip += instr;
  } else {
    vm->ip += 1;
  }
}

void op_test_eq(struct vm *vm) {
  debug_print("%04x OP_TEST_EQ\n", vm->ip);
  uint8_t reg1  = next_reg(vm);
  uint8_t reg2  = next_reg(vm);
  signed char instr  = next_byte(vm);

  owl_term val1 = get_reg(vm, reg1);
  owl_term val2 = get_reg(vm, reg2);

  if (int_from_owl_int(val1) == int_from_owl_int(val2)) {
    vm->ip += instr;
  } else {
    vm->ip += 1;
  }
}

void op_test_gt(struct vm *vm) {
  debug_print("%04x OP_TEST_GT\n", vm->ip);
  uint8_t reg1  = next_reg(vm);
  uint8_t reg2  = next_reg(vm);
  unsigned int instr = next_byte(vm);

  owl_term val1 = get_reg(vm, reg1);
  owl_term val2 = get_reg(vm, reg2);

  if (int_from_owl_int(val1) > int_from_owl_int(val2)) {
    vm->ip += instr;
  } else {
    vm->ip += 1;
  }
}

void op_test_gte(struct vm *vm) {
  debug_print("%04x OP_TEST_GTE\n", vm->ip);
  unsigned int reg1  = next_reg(vm);
  unsigned int reg2  = next_reg(vm);
  unsigned int instr = next_byte(vm);

  owl_term val1 = get_reg(vm, reg1);
  owl_term val2 = get_reg(vm, reg2);

  if (int_from_owl_int(val1) >= int_from_owl_int(val2)) {
    vm->ip += instr;
  } else {
    vm->ip += 1;
  }
}

void op_test_lt(struct vm *vm) {
  debug_print("%04x OP_TEST_LT\n", vm->ip);
  unsigned int reg1  = next_reg(vm);
  unsigned int reg2  = next_reg(vm);
  unsigned int instr = next_byte(vm);

  owl_term val1 = get_reg(vm, reg1);
  owl_term val2 = get_reg(vm, reg2);

  if (int_from_owl_int(val1) < int_from_owl_int(val2)) {
    vm->ip += instr;
  } else {
    vm->ip += 1;
  }
}

void op_test_lte(struct vm *vm) {
  debug_print("%04x OP_TEST_LTE\n", vm->ip);
  uint8_t reg1  = next_reg(vm);
  uint8_t reg2  = next_reg(vm);
  unsigned int instr = next_byte(vm);

  owl_term val1 = get_reg(vm, reg1);
  owl_term val2 = get_reg(vm, reg2);

  if (int_from_owl_int(val1) <= int_from_owl_int(val2)) {
    vm->ip += instr;
  } else {
    vm->ip += 1;
  }
}

void op_add(struct vm *vm) {
  debug_print("%04x OP_ADD\n", vm->ip);
  uint8_t reg1  = next_reg(vm);
  uint8_t reg2  = next_reg(vm);
  uint8_t reg3  = next_reg(vm);

  owl_term val1 = get_reg(vm, reg2);
  owl_term val2 = get_reg(vm, reg3);
  owl_term result = owl_int_from(int_from_owl_int(val1) + int_from_owl_int(val2));

  set_reg(vm, reg1, result);
  vm->ip += 1;
}

void op_sub(struct vm *vm) {
  debug_print("%04x OP_SUB\n", vm->ip);
  uint8_t reg1  = next_reg(vm);
  uint8_t reg2  = next_reg(vm);
  uint8_t reg3  = next_reg(vm);

  owl_term val1 = get_reg(vm, reg2);
  owl_term val2 = get_reg(vm, reg3);
  owl_term result = owl_int_from(int_from_owl_int(val1) - int_from_owl_int(val2));

  set_reg(vm, reg1, result);
  vm->ip += 1;
}

void op_call(struct vm *vm) {
  debug_print("%04x OP_CALL\n", vm->ip);

  uint8_t ret_reg = next_byte(vm);
  uint8_t function_id = next_byte(vm);
  uint64_t location = vm->functions[function_id];
  uint8_t arity = next_byte(vm);

  if (location == 0) {
    char fname_buf[255];
    char *fname_copy = fname_buf;
    strcpy(fname_copy, strings_lookup_id(vm->function_names, function_id));
    char *module_name = strsep(&fname_copy, ":");
    debug_print("Attempting to load module: %s\n", module_name);
    vm_load_module(vm, module_name);
    location = vm->functions[function_id];
  }

  assert(location != 0);
  assert(vm->current_frame + 1 <= STACK_DEPTH);

  unsigned int next_frame = vm->current_frame + 1;

  for(uint8_t i = 0; i < arity; i++) {
    owl_term arg = get_reg(vm, next_reg(vm));
    vm->frames[next_frame].registers[i + 1] = arg;
  }

  vm->frames[next_frame].ret_address = vm->ip + 1;
  vm->frames[next_frame].ret_register = ret_reg;
  vm->current_frame += 1;

  vm->ip = location;
}

void op_return(struct vm *vm) {
  debug_print("%04x OP_RETURN\n", vm->ip);
  frame_t *curr_frame = &vm->frames[vm->current_frame];
  frame_t *prev_frame = &vm->frames[vm->current_frame - 1];
  unsigned int ret_address = curr_frame->ret_address;

  prev_frame->registers[curr_frame->ret_register] = curr_frame->registers[0];

  vm->current_frame -= 1;
  vm->ip = ret_address;
}

void op_mov(struct vm *vm) {
  debug_print("%04x OP_MOV\n", vm->ip);
  uint8_t reg1 = next_reg(vm);
  uint8_t reg2 = next_reg(vm);

  set_reg(vm, reg1, get_reg(vm, reg2));

  vm->ip += 1;
}

void op_jmp(struct vm *vm) {
  debug_print("%04x OP_JMP\n", vm->ip);
  uint8_t loc = next_byte(vm);

  vm->ip = loc;
}

void op_tuple(struct vm *vm) {
  debug_print("%04x OP_TUPLE\n", vm->ip);
  uint8_t reg  = next_byte(vm);
  uint8_t size = next_byte(vm);

  owl_term *ary = owl_alloc(sizeof(owl_term) * (size + 1));
  ary[0] = size;

  for(uint8_t i = 1; i <= size; i++) {
    ary[i] = get_reg(vm, next_byte(vm));
  }

  owl_term tuple = (owl_term) ary;
  owl_term tagged_tuple =  (tuple << 3) | TUPLE;

  set_reg(vm, reg, tagged_tuple);

  vm->ip += 1;
}

void op_vector(struct vm *vm) {
  debug_print("%04x OP_VECTOR\n", vm->ip);
  uint8_t reg  = next_byte(vm);
  uint8_t size = next_byte(vm);

  owl_term vector = vector_init();

  for(uint8_t i = 0; i < size; i++) {
    vector = vector_push(vector, get_reg(vm, next_byte(vm)));
  }

  set_reg(vm, reg, vector);

  vm->ip += 1;
}

void op_tuple_nth(struct vm *vm) {
  debug_print("%04x OP_TUPLE_NTH\n", vm->ip);
  uint8_t reg = next_byte(vm);
  uint8_t tuple = next_byte(vm);
  uint8_t index_reg = next_byte(vm);

  uint64_t index = int_from_owl_int(get_reg(vm, index_reg));
  owl_term elem = owl_tuple_nth(get_reg(vm, tuple), index);
  set_reg(vm, reg, elem);

  vm->ip += 1;
}

void op_eq(struct vm *vm) {
  debug_print("%04x OP_EQ\n", vm->ip);
  uint8_t result_reg = next_reg(vm);
  uint8_t reg1 = next_reg(vm);
  uint8_t reg2 = next_reg(vm);

  owl_term left = get_reg(vm, reg1);
  owl_term right = get_reg(vm, reg2);

  owl_term result = owl_bool(owl_terms_eq(left, right));
  set_reg(vm, result_reg, result);
  vm->ip += 1;
}

void op_not_eq(struct vm *vm) {
  debug_print("%04x OP_EQ\n", vm->ip);
  uint8_t result_reg = next_reg(vm);
  uint8_t reg1 = next_reg(vm);
  uint8_t reg2 = next_reg(vm);

  owl_term left = get_reg(vm, reg1);
  owl_term right = get_reg(vm, reg2);

  owl_term result = owl_bool(!owl_terms_eq(left, right));
  set_reg(vm, result_reg, result);
  vm->ip += 1;
}

void op_not(struct vm *vm) {
  debug_print("%04x OP_EQ\n", vm->ip);
  uint8_t result_reg = next_reg(vm);
  uint8_t reg = next_reg(vm);

  owl_term value = get_reg(vm, reg);

  set_reg(vm, result_reg, owl_negate(value));
  vm->ip += 1;
}

void op_store_true(struct vm *vm) {
  debug_print("%04x OP_STORE_TRUE\n", vm->ip);
  uint8_t reg = next_byte(vm);

  set_reg(vm, reg, OWL_TRUE);
  vm->ip += 1;
}

void op_store_false(struct vm *vm) {
  debug_print("%04x OP_STORE_TRUE\n", vm->ip);
  uint8_t reg = next_byte(vm);

  set_reg(vm, reg, OWL_FALSE);
  vm->ip += 1;
}

void opcode_init(vm_t * vm) {
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
  vm->opcodes[OP_JMP] = op_jmp;
  vm->opcodes[OP_TUPLE]     = op_tuple;
  vm->opcodes[OP_TUPLE_NTH] = op_tuple_nth;
  vm->opcodes[OP_VECTOR] = op_vector;
  vm->opcodes[OP_STORE_TRUE] = op_store_true;
  vm->opcodes[OP_STORE_FALSE] = op_store_false;
  vm->opcodes[OP_TEST] = op_test;
  vm->opcodes[OP_EQ] = op_eq;
  vm->opcodes[OP_NOT_EQ] = op_not_eq;
  vm->opcodes[OP_NOT] = op_not;
}
