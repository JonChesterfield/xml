#ifndef TOOLS_INTSET_H_INCLUDED
#define TOOLS_INTSET_H_INCLUDED

// Set on uint64_t's. Cannot store UINT64_MAX.
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint64_t state[4];
} intset_t;

intset_t intset_create(uint64_t capacity); // power 2 or zero
void intset_destroy(intset_t);
bool intset_valid(intset_t);

bool intset_equal(intset_t, intset_t);
intset_t intset_clone(intset_t);

intset_t intset_rehash(intset_t, uint64_t capacity);

uint64_t intset_size(intset_t);
uint64_t intset_capacity(intset_t);

static inline bool intset_rehash_double(intset_t *h) {
  uint64_t cap = intset_capacity(*h);
  intset_t n = intset_rehash(*h, 2 * cap);

  if (!intset_valid(n)) {
    return false;
  }

  intset_destroy(*h);
  *h = n;
  return true;
}

static inline uint64_t intset_available(intset_t s) {
  return intset_capacity(s) - intset_size(s);
}

bool intset_contains(intset_t, uint64_t);
void intset_insert(intset_t *, uint64_t);
void intset_remove(intset_t*, uint64_t);
void intset_clear(intset_t *);

#endif
