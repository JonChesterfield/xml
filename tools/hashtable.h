#ifndef TOOLS_HASHTABLE_H_INCLUDED
#define TOOLS_HASHTABLE_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "contract.h"

struct hashtable_module_ty;
typedef const struct hashtable_module_ty *hashtable_module;

#define hashtable_require(X)                                                   \
  do {                                                                         \
    if (hashtable_contract_active(mod)) {                                      \
      mod->maybe_contract(X, contract_message(X),                              \
                          sizeof(contract_message(X)) - 1);                    \
    }                                                                          \
  } while (0)

typedef struct {
  uint64_t state[4];
} hashtable_t;

// size is power of two or zero
static inline hashtable_t hashtable_create(hashtable_module mod, uint64_t size);
static inline void hashtable_destroy(hashtable_module mod, hashtable_t);
static inline bool hashtable_valid(hashtable_module mod, hashtable_t);

// Inefficient. Order of insertion independent.
static inline bool hashtable_equal(hashtable_module mod, hashtable_t,
                                   hashtable_t);

// N is a power of two or zero. N must be greater than size.
static inline hashtable_t hashtable_rehash(hashtable_module mod, hashtable_t,
                                           uint64_t N);

//
// Module level queries
//

// Number of bytes used for key and for value
// key, value align is a power of two
// key size is nonzero, value can be zero
static inline uint32_t hashtable_key_align(hashtable_module mod);
static inline uint32_t hashtable_value_align(hashtable_module mod);
static inline uint32_t hashtable_key_size(hashtable_module mod);
static inline uint32_t hashtable_value_size(hashtable_module mod);

// Returns key_size bytes representing a key that cannot be inserted
static inline const unsigned char *hashtable_key_sentinel(hashtable_module mod);

// a memcmp, not a pointer compare
static inline bool hashtable_key_is_sentinel(hashtable_module mod, hashtable_t,
                                             const unsigned char *key);

static inline bool hashtable_contract_active(hashtable_module mod);

//
// Hashtable level queries
//

// Takes key_size bytes, calculates a hash from them
static inline uint64_t hashtable_key_hash(hashtable_module mod, hashtable_t,
                                          unsigned char *bytes);

// Compare keys with respect to a given hash table, e.g. by memcmp
static inline bool hashtable_key_equal(hashtable_module mod, hashtable_t,
                                       const unsigned char *left,
                                       const unsigned char *right);

// Count of keys in the table
static inline uint64_t hashtable_size(hashtable_module mod, hashtable_t);

// Capacity of table. Keys are either as inserted or sentinels
static inline uint64_t hashtable_capacity(hashtable_module mod, hashtable_t);

// capacity - size
static inline uint64_t hashtable_available(hashtable_module mod, hashtable_t);

//
// Mutators. All of them require available() > 0.
//

// Test if key corresponds to a key/value pair in the table
static inline bool hashtable_contains(hashtable_module mod, hashtable_t,
                                      unsigned char *key);

// Returns pointer to key_size bytes matching key (0 if not contained)
static inline unsigned char *
hashtable_lookup_key(hashtable_module mod, hashtable_t, unsigned char *key);

// Returns pointer to value_size bytes matching key (0 if not contained)
static inline unsigned char *
hashtable_lookup_value(hashtable_module mod, hashtable_t, unsigned char *key);

// (value_size == 0) != (value != NULL)
static inline void hashtable_insert(hashtable_module mod, hashtable_t *,
                                    unsigned char *key, unsigned char *value);

static inline void hashtable_remove(hashtable_module mod, hashtable_t *,
                                    unsigned char *key);

struct hashtable_module_ty {
  hashtable_t (*const create)(uint64_t size);
  void (*const destroy)(hashtable_t);
  bool (*const valid)(hashtable_t);

  const uint32_t key_align;
  const uint32_t key_size;
  const uint32_t value_align;
  const uint32_t value_size;

  const unsigned char *sentinel;

  uint64_t (*const key_hash)(hashtable_t, unsigned char *bytes);
  bool (*const key_equal)(hashtable_t, const unsigned char *,
                          const unsigned char *);

  uint64_t (*const size)(hashtable_t);
  uint64_t (*const capacity)(hashtable_t);

  // Find the key. if it is missing, return where it should be inserted.
  // If the key is missing and availability is zero, returns ~0
  uint64_t (*const lookup_offset)(hashtable_t, unsigned char *key);

  // key and value lookup tend to use the same offset computation
  unsigned char *(*const location_key)(hashtable_t, uint64_t offset);

  // location_value could be 0 if value_size==0
  unsigned char *(*const location_value)(hashtable_t, uint64_t offset);

  void (*const set_size)(hashtable_t *, uint64_t);

  // insert is derived from the lookups but remove may need to rearrange
  // internals and may not be available
  void (*const maybe_remove)(hashtable_t *, unsigned char *key);

  void (*const maybe_contract)(bool, const char *message,
                               size_t message_length);
};

static inline hashtable_t hashtable_create(hashtable_module mod,
                                           uint64_t size) {
  hashtable_require(contract_is_zero_or_power_of_two(size));
  return mod->create(size);
}

static inline void hashtable_destroy(hashtable_module mod, hashtable_t h) {
  hashtable_require(hashtable_valid(mod, h));
  mod->destroy(h);
}

static inline bool hashtable_valid(hashtable_module mod, hashtable_t h) {
  return mod->valid(h);
}

//
// Module level queries
//

// Number of bytes used for key and for value
// key, value align is a power of two
// key size is nonzero, value can be zero
static inline uint32_t hashtable_key_align(hashtable_module mod) {
  return mod->key_align;
}
static inline uint32_t hashtable_value_align(hashtable_module mod) {
  return mod->value_align;
}
static inline uint32_t hashtable_key_size(hashtable_module mod) {
  return mod->key_size;
}
static inline uint32_t hashtable_value_size(hashtable_module mod) {
  return mod->value_size;
}

// Returns key_size bytes representing a key that cannot be inserted
static inline const unsigned char *
hashtable_key_sentinel(hashtable_module mod) {
  return mod->sentinel;
}

static inline bool hashtable_key_is_sentinel(hashtable_module mod,
                                             hashtable_t h,
                                             const unsigned char *key) {
  const unsigned char *sentinel = hashtable_key_sentinel(mod);
  return hashtable_key_equal(mod, h, key, sentinel);
}

static inline bool hashtable_contract_active(hashtable_module mod) {
  return mod->maybe_contract != 0;
}

// Takes key_size bytes, calculates a hash from them
static inline uint64_t hashtable_key_hash(hashtable_module mod, hashtable_t h,
                                          unsigned char *bytes) {
  return mod->key_hash(h, bytes);
}

static inline bool hashtable_key_equal(hashtable_module mod, hashtable_t h,
                                       const unsigned char *left,
                                       const unsigned char *right) {
  return mod->key_equal(h, left, right);
}

static inline uint64_t hashtable_size(hashtable_module mod, hashtable_t h) {
  hashtable_require(hashtable_valid(mod, h));
  return mod->size(h);
}

static inline uint64_t hashtable_capacity(hashtable_module mod, hashtable_t h) {
  hashtable_require(hashtable_valid(mod, h));
  uint64_t r = mod->capacity(h);
  hashtable_require(contract_is_zero_or_power_of_two(r));
  return r;
}

static inline uint64_t hashtable_available(hashtable_module mod,
                                           hashtable_t h) {
  hashtable_require(hashtable_valid(mod, h));
  return hashtable_capacity(mod, h) - hashtable_size(mod, h);
}

static inline bool hashtable_contains(hashtable_module mod, hashtable_t h,
                                      unsigned char *key) {
  hashtable_require(hashtable_valid(mod, h));

  uint64_t offset = mod->lookup_offset(h, key);
  if (offset == UINT64_MAX) {
    return false;
  }

  unsigned char *kres = mod->location_key(h, offset);
  return !hashtable_key_is_sentinel(mod, h, kres);
}

static inline unsigned char *
hashtable_lookup_key(hashtable_module mod, hashtable_t h, unsigned char *key) {
  hashtable_require(hashtable_valid(mod, h));
  uint64_t offset = mod->lookup_offset(h, key);
  if (offset == UINT64_MAX) {
    return 0;
  }
  unsigned char *kres = mod->location_key(h, offset);
  return hashtable_key_is_sentinel(mod, h, kres) ? 0 : kres;
}

static inline unsigned char *hashtable_lookup_value(hashtable_module mod,
                                                    hashtable_t h,
                                                    unsigned char *key) {
  hashtable_require(hashtable_valid(mod, h));
  hashtable_require(mod->value_size != 0);
  uint64_t offset = mod->lookup_offset(h, key);
  if (offset == UINT64_MAX) {
    return 0;
  }
  unsigned char *kres = mod->location_key(h, offset);
  return hashtable_key_is_sentinel(mod, h, kres)
             ? 0
             : mod->location_value(h, offset);
}

// (value_size == 0) != (value != NULL)
static inline void hashtable_insert(hashtable_module mod, hashtable_t *h,
                                    unsigned char *key, unsigned char *value) {
  hashtable_require(hashtable_valid(mod, *h));
  const bool is_set = mod->value_size == 0;
  bool have_value = value != 0;
  hashtable_require(is_set != have_value);
  hashtable_require(hashtable_available(mod, *h) > 0);

  uint64_t offset = mod->lookup_offset(*h, key);

  unsigned char *k_res = mod->location_key(*h, offset);
  unsigned char *v_res = is_set ? 0 : mod->location_value(*h, offset);

  if (hashtable_key_is_sentinel(mod, *h, k_res)) {
    mod->set_size(h, hashtable_size(mod, *h) + 1);
  }

  __builtin_memcpy(k_res, key, mod->key_size);
  if (!is_set) {
    __builtin_memcpy(v_res, value, mod->value_size);
  }

  hashtable_require(hashtable_contains(mod, *h, key));

  if (hashtable_contract_active(mod)) {
    unsigned char *k_chk = hashtable_lookup_key(mod, *h, key);
    unsigned char *v_chk = (is_set ? 0 : hashtable_lookup_value(mod, *h, key));

    hashtable_require(hashtable_key_equal(mod, *h, k_res, k_chk));

    hashtable_require(is_set ||
                      (__builtin_memcmp(v_res, v_chk, mod->value_size) == 0));
  }
}

static inline void hashtable_remove(hashtable_module mod, hashtable_t *h,
                                    unsigned char *key) {
  hashtable_require(hashtable_valid(mod, *h));
  hashtable_require(mod->maybe_remove != NULL);

  uint64_t size = hashtable_contract_active(mod) ? hashtable_size(mod, *h) : 0;
  bool contains_before =
      hashtable_contract_active(mod) ? hashtable_contains(mod, *h, key) : false;

  mod->maybe_remove(h, key);

  hashtable_require(hashtable_size(mod, *h) ==
                    (contains_before ? (size - 1) : size));

  hashtable_require(hashtable_contains(mod, *h, key) == false);
}

static inline bool hashtable_equal(hashtable_module mod, hashtable_t x,
                                   hashtable_t y) {
  hashtable_require(hashtable_valid(mod, x));
  hashtable_require(hashtable_valid(mod, y));

  const bool is_set = mod->value_size == 0;

  uint64_t N = hashtable_size(mod, x);
  if (N != hashtable_size(mod, y)) {
    return false;
  }

  // Guess x is smaller
  uint64_t cap = hashtable_capacity(mod, x);
  hashtable_t s = x;
  hashtable_t l = y;

  {
    // See if that guess was wrong, shuffle variables if so
    uint64_t capy = hashtable_capacity(mod, y);
    if (capy > cap) {
      cap = capy;
      s = y;
      l = x;
    }
  }

  for (uint64_t offset = 0; offset < cap; offset++) {
    unsigned char *k_res = mod->location_key(s, offset);
    if (hashtable_key_is_sentinel(mod, s, k_res)) {
      continue;
    }

    unsigned char *v_res = is_set ? 0 : mod->location_value(s, offset);

    uint64_t large_offset = mod->lookup_offset(l, k_res);
    unsigned char *l_k_res = mod->location_key(l, large_offset);

    bool eq_s = hashtable_key_equal(mod, s, k_res, l_k_res);
    hashtable_require(eq_s == hashtable_key_equal(mod, l, k_res, l_k_res));
    if (!eq_s) {
      return false;
    }

    if (!is_set) {
      unsigned char *l_v_res = mod->location_value(l, large_offset);
      if (__builtin_memcmp(v_res, l_v_res, mod->value_size) != 0) {
        return false;
      }
    }
  }

  return true;
}

static inline hashtable_t hashtable_rehash(hashtable_module mod, hashtable_t h,
                                           uint64_t N) {
  hashtable_require(hashtable_valid(mod, h));
  hashtable_require(contract_is_zero_or_power_of_two(N));
  hashtable_require(N >= hashtable_size(mod, h));
  const bool is_set = mod->value_size == 0;

  hashtable_t ret = hashtable_create(mod, N);

  uint64_t cap = hashtable_capacity(mod, h);
  for (uint64_t offset = 0; offset < cap; offset++) {
    unsigned char *k_res = mod->location_key(h, offset);

    if (!hashtable_key_is_sentinel(mod, h, k_res)) {
      unsigned char *v_res = is_set ? 0 : mod->location_value(h, offset);
      hashtable_insert(mod, &ret, k_res, v_res);
    }
  }

  hashtable_require(hashtable_equal(mod, h, ret));

  return ret;
}

#endif
