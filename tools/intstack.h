#ifndef TOOLS_INTSTACK_H_INCLUDED
#define TOOLS_INTSTACK_H_INCLUDED

// Stack of uint64_t

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  void *state;
} intstack_t;

intstack_t intstack_create(uint64_t capacity);
void intstack_destroy(intstack_t);
bool intstack_valid(intstack_t);

bool intstack_equal(intstack_t, intstack_t);
intstack_t intstack_clone(intstack_t);

// Number of items
uint64_t intstack_size(intstack_t);

// Allocated size
uint64_t intstack_capacity(intstack_t);

// Change allocated size, return true on success
bool intstack_reserve(intstack_t *, uint64_t);

static inline uint64_t intstack_available(intstack_t s) {
  return intstack_capacity(s) - intstack_size(s);
}

// requires available > 0
void intstack_push(intstack_t *, uint64_t);

// requires size > 0
uint64_t intstack_peek(intstack_t);

// requires size > 0
void intstack_drop(intstack_t *);

void intstack_dump(intstack_t);

#endif
