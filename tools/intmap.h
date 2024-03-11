#ifndef TOOLS_INTMAP_H_INCLUDED
#define TOOLS_INTMAP_H_INCLUDED

// Map on uint64_t's. Cannot use UINT64_MAX as a key.
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint64_t state[4];
} intmap_t;

intmap_t intmap_create(uint64_t capacity); // power 2 or zero
void intmap_destroy(intmap_t);
bool intmap_valid(intmap_t);

bool intmap_equal(intmap_t, intmap_t);
intmap_t intmap_clone(intmap_t);

intmap_t intmap_rehash(intmap_t, uint64_t capacity);

uint64_t intmap_size(intmap_t);
uint64_t intmap_capacity(intmap_t);

static inline bool intmap_rehash_double(intmap_t *h) {
  uint64_t cap = intmap_capacity(*h);
  intmap_t n = intmap_rehash(*h, 2 * cap);

  if (!intmap_valid(n)) {
    return false;
  }

  intmap_destroy(*h);
  *h = n;
  return true;
}

static inline uint64_t intmap_available(intmap_t s) {
  return intmap_capacity(s) - intmap_size(s);
}

bool intmap_contains(intmap_t, uint64_t);
uint64_t intmap_lookup(intmap_t, uint64_t);
void intmap_insert(intmap_t *, uint64_t k, uint64_t v);
void intmap_remove(intmap_t *, uint64_t);
void intmap_clear(intmap_t *);

#endif
