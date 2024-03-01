#ifndef TOOLS_INTSET_UTIL_H_INCLUDED
#define TOOLS_INTSET_UTIL_H_INCLUDED

#include "arena.h"
#include "hashtable.h"
#include "intset.h"

enum { intset_util_struct_sizeof = 4 * 8 };

static hashtable_t intset_util_to_hash(intset_t s) {
  hashtable_t r;
  __builtin_memcpy(&r.state, &s.state, intset_util_struct_sizeof);
  return r;
}

static intset_t intset_util_to_set(hashtable_t s) {
  intset_t r;
  __builtin_memcpy(&r.state, &s.state, intset_util_struct_sizeof);
  return r;
}

static arena_t intset_util_hash_to_arena(hashtable_t h) {
  arena_t a;
  a.base = h.state[0];
  a.next = h.state[1];
  a.limit = h.state[2];
  return a;
}

static hashtable_t intset_util_arena_to_hash(arena_t a) {
  hashtable_t h;
  h.state[0] = a.base;
  h.state[1] = a.next;
  h.state[2] = a.limit;
  h.state[3] = 0;
  return h;
}

#define INTSET_UTIL(ARENA, KEY_EQUAL, CONTRACT)                                \
                                                                               \
  static hashtable_t intset_util_create(uint64_t size) {                       \
    if (CONTRACT) {                                                            \
      CONTRACT(contract_is_zero_or_power_of_two(size), "size", 4);             \
    }                                                                          \
                                                                               \
    arena_t a = arena_create(ARENA, 8 * size);                                 \
    unsigned char *p = arena_base_address(ARENA, a);                           \
    for (uint64_t i = 0; i < size; i++) {                                      \
      unsigned char *c = p + 8 * i;                                            \
      __builtin_memcpy(c, &sentinel, 8);                                       \
    }                                                                          \
                                                                               \
    hashtable_t r = intset_util_arena_to_hash(a);                              \
    return r;                                                                  \
  }                                                                            \
  static void intset_util_destroy(hashtable_t h) {                             \
    arena_destroy(ARENA, intset_util_hash_to_arena(h));                        \
  }                                                                            \
                                                                               \
  static bool intset_util_valid(hashtable_t h) {                               \
    return arena_valid(ARENA, intset_util_hash_to_arena(h));                   \
  }                                                                            \
                                                                               \
  static uint64_t intset_util_size(hashtable_t h) {\
    arena_t a = intset_util_hash_to_arena(h);      \
    uint64_t allocation_edge =                                          \
      (char*)arena_next_address(ARENA, a) - (char*)arena_base_address(ARENA, a); \
    return allocation_edge/8;                                                  \
  }                                                                     \
                                                                               \
  static uint64_t intset_util_capacity(hashtable_t h) {                        \
    arena_t a = intset_util_hash_to_arena(h);                                  \
    uint64_t r = arena_capacity(arena_mod, a) / 8;                                 \
    if (CONTRACT) {                                                            \
      CONTRACT(contract_is_zero_or_power_of_two(r), "cap", 3);                 \
    }                                                                          \
    return r;                                                                  \
  }                                                                            \
  static uint64_t intset_util_available(hashtable_t h) {                       \
    return intset_util_capacity(h) - intset_util_size(h);                      \
  }                                                                            \
                                                                               \
  static unsigned char *intset_util_location_key(hashtable_t h,                \
                                                 uint64_t offset) {            \
    arena_t a = intset_util_hash_to_arena(h);                                  \
    unsigned char *p = arena_base_address(ARENA, a);                           \
    return p + offset * 8;                                                     \
  }                                                                            \
                                                                               \
  static uint64_t intset_util_lookup_offset_given_hash(                        \
      hashtable_t h, unsigned char *key, uint64_t hash) {                      \
    uint64_t cap = intset_util_capacity(h);                                    \
                                                                               \
    if (CONTRACT) {                                                            \
      CONTRACT(contract_is_power_of_two(cap), "cap offset", 10);               \
    }                                                                          \
                                                                               \
    const uint64_t mask = cap - 1;                                             \
    const uint64_t start_index = hash;                                         \
                                                                               \
    for (uint64_t c = 0; c < cap; c++) {                                       \
      uint64_t index = (start_index + c) & mask;                               \
                                                                               \
      unsigned char *loc_key = intset_util_location_key(h, index);             \
                                                                               \
      if (KEY_EQUAL(h, loc_key, key)) {                                        \
        /* Found key */                                                        \
        return index;                                                          \
      }                                                                        \
                                                                               \
      if (KEY_EQUAL(h, loc_key, (unsigned char *)&sentinel)) {                 \
        /* Found a space */                                                    \
        return index;                                                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    if (CONTRACT) {                                                            \
      CONTRACT(intset_util_available(h) == 0, "avail 0", 7);                   \
    }                                                                          \
                                                                               \
    return UINT64_MAX;                                                         \
  }

#endif
