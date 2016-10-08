#ifndef STACK_H
#define STACK_H

#include <stdint.h>

typedef struct stack stack;

stack* stack_create(uint32_t capacity);
uint32_t stack_size(stack* s);
void stack_push(stack* s, void *item);
void** stack_peek_indirect(stack* s);
void** stack_get_indirect(stack* s, uint32_t index);
void* stack_pop(stack* s);

#endif
