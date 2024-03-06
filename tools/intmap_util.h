#ifndef TOOLS_INTMAP_UTIL_H_INCLUDED
#define TOOLS_INTMAP_UTIL_H_INCLUDED

#include "arena.h"
#include "hashtable.h"
#include "intmap.h"

static inline arena_t intmap_util_hash_to_arena(hashtable_t h) {
  arena_t a;
  a.base = h.state[0];
  a.next = h.state[1];
  a.limit = h.state[2];
  return a;
}

static inline hashtable_t intmap_util_arena_to_hash(arena_t a) {
  hashtable_t h;
  h.state[0] = a.base;
  h.state[1] = a.next;
  h.state[2] = a.limit;
  h.state[3] = 0;
  return h;
}

static inline void intmap_util_store_userdata(hashtable_t *h, uint64_t v) {
  h->state[3] = v;
}

static inline uint64_t intmap_util_load_userdata(hashtable_t *h) {
  return h->state[3];
}

static const uint64_t intmap_util_sentinel = UINT64_MAX;

static inline hashtable_t
intmap_util_create_using_arena(arena_module mod, uint64_t size, bool is_set) {
  uint64_t words = is_set ? size : 2 * size;
  arena_t a = arena_create(mod, 8 * words);
  unsigned char *p = arena_base_address(mod, a);
  for (uint64_t i = 0; i < words; i++) {
    unsigned char *c = p + 8 * i;
    __builtin_memcpy(c, &intmap_util_sentinel, 8);
  }

  return intmap_util_arena_to_hash(a);
}

static inline void intmap_util_destroy_using_arena(arena_module mod,
                                                   hashtable_t h) {
  arena_destroy(mod, intmap_util_hash_to_arena(h));
}

static inline bool intmap_util_valid_using_arena(arena_module mod,
                                                 hashtable_t h) {
  return arena_valid(mod, intmap_util_hash_to_arena(h));
}

#define INTMAP_UTIL(ARENA, IS_SET, KEY_HASH, KEY_EQUAL, CONTRACT)              \
  static size_t bytes_per_elt() { return IS_SET ? 8 : 16; }                    \
  static hashtable_t intmap_util_create(uint64_t size) {                       \
    if (CONTRACT) {                                                            \
      CONTRACT(contract_is_zero_or_power_of_two(size), "size", 4);             \
    }                                                                          \
    return intmap_util_create_using_arena(ARENA, size, IS_SET);                \
  }                                                                            \
                                                                               \
  static void intmap_util_destroy(hashtable_t h) {                             \
    intmap_util_destroy_using_arena(ARENA, h);                                 \
  }                                                                            \
                                                                               \
  static bool intmap_util_valid(hashtable_t h) {                               \
    return intmap_util_valid_using_arena(ARENA, h);                            \
  }                                                                            \
                                                                               \
  static uint64_t intmap_util_size(hashtable_t h) {                            \
    arena_t a = intmap_util_hash_to_arena(h);                                  \
    uint64_t allocation_edge = arena_next_offset(ARENA, a);                    \
    uint64_t r = allocation_edge / bytes_per_elt();                            \
    return r;                                                                  \
  }                                                                            \
                                                                               \
  static void intmap_util_assign_size(hashtable_t *h, uint64_t s) {            \
    arena_t a = intmap_util_hash_to_arena(*h);                                 \
    uint64_t ud = intmap_util_load_userdata(h);                                \
    arena_change_allocation(arena_mod, &a, s *bytes_per_elt());                \
    *h = intmap_util_arena_to_hash(a);                                         \
    intmap_util_store_userdata(h, ud);                                         \
  }                                                                            \
                                                                               \
  static uint64_t intmap_util_capacity(hashtable_t h) {                        \
    arena_t a = intmap_util_hash_to_arena(h);                                  \
    uint64_t r = arena_capacity(arena_mod, a) / bytes_per_elt();               \
    if (CONTRACT) {                                                            \
      CONTRACT(contract_is_zero_or_power_of_two(r), "cap", 3);                 \
    }                                                                          \
    return r;                                                                  \
  }                                                                            \
                                                                               \
  static unsigned char *intmap_util_location_value(hashtable_t h,              \
                                                   uint64_t offset) {          \
    if (IS_SET) {                                                              \
      return 0;                                                                \
    }                                                                          \
    arena_t a = intmap_util_hash_to_arena(h);                                  \
    unsigned char *p = arena_base_address(ARENA, a);                           \
    return p + (offset * bytes_per_elt()) + bytes_per_elt() / 2;               \
  }                                                                            \
                                                                               \
  static unsigned char *intmap_util_location_key(hashtable_t h,                \
                                                 uint64_t offset) {            \
    if (IS_SET) {                                                              \
      (void)intmap_util_location_value;                                        \
    }                                                                          \
    arena_t a = intmap_util_hash_to_arena(h);                                  \
    unsigned char *p = arena_base_address(ARENA, a);                           \
    return p + offset * bytes_per_elt();                                       \
  }

#endif
