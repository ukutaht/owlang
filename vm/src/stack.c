#include <stdlib.h>
#include "stack.h"

struct stack {
  uint32_t size;
  uint32_t capacity;
  void** items;
};

stack* stack_create(uint32_t capacity) {
  stack* s = malloc(sizeof(stack));
  s->capacity = capacity;
  s->size = 0;
  s->items = malloc(sizeof(void *) * capacity);
  return s;
}

uint32_t stack_size(stack* s) {
  return s->size;
}

static void stack_resize(stack* s, uint32_t capacity) {
  void **items = realloc(s->items, sizeof(void *) * capacity);
  if (items) {
    s->items = items;
    s->capacity = capacity;
  }
}

void stack_push(stack* s, void *item) {
  if (s->capacity == s->size)
    stack_resize(s, s->capacity * 2);
  s->items[s->size++] = item;
}

void** stack_peek_indirect(stack* s) {
  return &(s->items[s->size - 1]);
}

void** stack_get_indirect(stack* s, uint32_t index) {
  return &(s->items[index]);
}

void* stack_pop(stack* s) {
  if (s->size == 0) {
    return NULL;
  }

  void* item = s->items[s->size-1];
  s->items[s->size-1] = NULL;
  s->size--;

  if (s->size > 0 && s->size <= s->capacity / 4)
    stack_resize(s, s->capacity / 2);

  return item;
}
