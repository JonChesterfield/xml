#ifndef TOOLS_HASHTABLE_H_INCLUDED
#define TOOLS_HASHTABLE_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "contract.h"

#include <stdio.h>

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

// Initial capacity is power of two or zero
static inline hashtable_t hashtable_create(hashtable_module mod,
                                           uint64_t capacity);
static inline void hashtable_destroy(hashtable_module mod, hashtable_t);
static inline bool hashtable_valid(hashtable_module mod, hashtable_t);

// A word of user defined state, semantics opaque to hashtable
static inline void hashtable_store_userdata(hashtable_module mod, hashtable_t *,
                                            uint64_t);
static inline uint64_t hashtable_load_userdata(hashtable_module mod,
                                               hashtable_t *);

// Inefficient. Order of insertion independent.
static inline bool hashtable_equal(hashtable_module mod, hashtable_t,
                                   hashtable_t);

// Allocate a new table, same as the old. Key/values in same order.
static inline hashtable_t hashtable_clone(hashtable_module mod, hashtable_t);

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

// Index into table that key would ideally be placed
static inline uint64_t hashtable_preferred_offset(hashtable_module mod,
                                                  hashtable_t h,
                                                  unsigned char *key);

// Whether the offset does not contain the sentinel
static inline bool hashtable_offset_occupied(hashtable_module mod,
                                             hashtable_t h, uint64_t offset);

// Offset where key needs to be placed given existing data
// or UINT64_MAX if it is absent and available is zero
static inline uint64_t hashtable_lookup_offset(hashtable_module mod,
                                               hashtable_t h,
                                               unsigned char *key);

// Test if key corresponds to a key/value pair in the table
static inline bool hashtable_contains(hashtable_module mod, hashtable_t,
                                      unsigned char *key);

// Returns pointer to key_size bytes matching key (0 if not contained)
static inline unsigned char *
hashtable_lookup_key(hashtable_module mod, hashtable_t, unsigned char *key);

// Returns pointer to value_size bytes matching key (0 if not contained)
static inline unsigned char *
hashtable_lookup_value(hashtable_module mod, hashtable_t, unsigned char *key);

// requires available > 0
// (value_size == 0) != (value != NULL)
static inline void hashtable_insert(hashtable_module mod, hashtable_t *,
                                    unsigned char *key, unsigned char *value);

// Zero out the table
static inline void hashtable_clear(hashtable_module mod, hashtable_t *h);

static inline void hashtable_remove(hashtable_module mod, hashtable_t *,
                                    unsigned char *key);

// Debugging hack
static inline void hashtable_dump(hashtable_module mod, hashtable_t h);

struct hashtable_module_ty {
  hashtable_t (*const create)(uint64_t capacity);
  void (*const destroy)(hashtable_t);
  bool (*const valid)(hashtable_t);

  void (*const store_userdata)(hashtable_t *, uint64_t);
  uint64_t (*const load_userdata)(hashtable_t *);

  const uint32_t key_align;
  const uint32_t key_size;
  const uint32_t value_align;
  const uint32_t value_size;

  const unsigned char *sentinel;

  uint64_t (*const key_hash)(hashtable_t, unsigned char *bytes);
  bool (*const key_equal)(hashtable_t, const unsigned char *,
                          const unsigned char *);

  uint64_t (*const size)(hashtable_t);

  // Hashtable has slots from [0 capacity)
  // containing sentinel or keys
  uint64_t (*const capacity)(hashtable_t);

  // key and value lookup tend to use the same offset computation
  unsigned char *(*const location_key)(hashtable_t, uint64_t offset);

  // location_value could be 0 if value_size==0
  unsigned char *(*const location_value)(hashtable_t, uint64_t offset);

  // Size is always computable by walking the table and checking for
  // key_equal with sentinel but O(1) size is a good feature
  void (*const assign_size)(hashtable_t *, uint64_t);

  void (*const maybe_contract)(bool, const char *message,
                               size_t message_length);
};

static inline hashtable_t hashtable_create(hashtable_module mod,
                                           uint64_t capacity) {
  hashtable_require(contract_is_zero_or_power_of_two(capacity));
  hashtable_t res = mod->create(capacity);
  if (mod->valid(res)) {
    hashtable_require(hashtable_capacity(mod, res) == capacity);
  }
  return res;
}

static inline void hashtable_destroy(hashtable_module mod, hashtable_t h) {
  hashtable_require(hashtable_valid(mod, h));
  mod->destroy(h);
}

static inline bool hashtable_valid_verbose(hashtable_module mod, hashtable_t h,
                                           bool verbose) {
  // valid() is called a lot, this has a dramatic effect on runtime performance
  const bool superficial = false;
  
  if (!mod->valid(h)) {
    return false;
  }

  if (superficial) { return true; }
  
  uint64_t cap = mod->capacity(h);
  const uint64_t mask = cap - 1;

  for (uint64_t off = 0; off < cap; off++) {
    unsigned char *key = mod->location_key(h, off);
    if (hashtable_key_is_sentinel(mod, h, key)) {
      continue;
    }

    uint64_t hash = mod->key_hash(h, key);
    uint64_t pref = hash & mask;

    // Scan linearly looking for that key
    for (uint64_t delta = 0; delta < cap; delta++) {
      uint64_t loc = (pref + delta) & mask;

      if (loc == off) {
        hashtable_require(key == mod->location_key(h, loc));
        break;
      }

      unsigned char *key_loc = mod->location_key(h, loc);

      if (hashtable_key_is_sentinel(mod, h, key_loc)) {
        // Found an empty slot before the value, failure
        if (verbose) {
          fprintf(stdout, "Checking offset %lu, found sentinel at %lu\n", off,
                  loc);
        }
        return false;
      }

      if (hashtable_key_equal(mod, h, key, key_loc)) {
        if (verbose) {
          fprintf(stdout, "Checking offset %lu, found key earlier at %lu\n",
                  off, loc);
        }

        // Found an equal key before the one at current offset
        hashtable_require(loc != off);
        return false;
      }
    }
  }
  return true;
}

static inline bool hashtable_valid(hashtable_module mod, hashtable_t h) {
  return hashtable_valid_verbose(mod, h, false);
}

// A word of user defined state, semantics opaque to hashtable
static inline void hashtable_store_userdata(hashtable_module mod,
                                            hashtable_t *h, uint64_t v) {
  hashtable_require(hashtable_valid(mod, *h));
  mod->store_userdata(h, v);
}

static inline uint64_t hashtable_load_userdata(hashtable_module mod,
                                               hashtable_t *h) {
  hashtable_require(hashtable_valid(mod, *h));
  return mod->load_userdata(h);
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
  return mod->key_equal(h, key, sentinel);
}

static inline bool hashtable_contract_active(hashtable_module mod) {
  return mod->maybe_contract != 0;
}

// Takes key_size bytes, calculates a hash from them
static inline uint64_t hashtable_key_hash(hashtable_module mod, hashtable_t h,
                                          unsigned char *bytes) {
  hashtable_require(hashtable_valid(mod, h));
  return mod->key_hash(h, bytes);
}

static inline bool hashtable_key_equal(hashtable_module mod, hashtable_t h,
                                       const unsigned char *left,
                                       const unsigned char *right) {
  hashtable_require(hashtable_valid(mod, h));
  return mod->key_equal(h, left, right);
}

static inline uint64_t hashtable_size(hashtable_module mod, hashtable_t h) {
  hashtable_require(hashtable_valid(mod, h));
  uint64_t msize = mod->size(h);
  if (hashtable_contract_active(mod)) {
    uint64_t cap = hashtable_capacity(mod, h);
    uint64_t count = 0;
    for (uint64_t off = 0; off < cap; off++) {
      if (!hashtable_offset_occupied(mod, h, off)) {
        count++;
      }
    }
    hashtable_require(msize == count);
  }
  return msize;
}

static inline uint64_t hashtable_capacity(hashtable_module mod, hashtable_t h) {
  hashtable_require(mod->valid(h));
  uint64_t r = mod->capacity(h);
  hashtable_require(contract_is_zero_or_power_of_two(r));
  return r;
}

static inline uint64_t hashtable_available(hashtable_module mod,
                                           hashtable_t h) {
  hashtable_require(hashtable_valid(mod, h));
  return hashtable_capacity(mod, h) - hashtable_size(mod, h);
}

static inline uint64_t hashtable_preferred_offset(hashtable_module mod,
                                                  hashtable_t h,
                                                  unsigned char *key) {
  hashtable_require(hashtable_valid(mod, h));
  hashtable_require(!hashtable_key_is_sentinel(mod, h, key));
  uint64_t hash = hashtable_key_hash(mod, h, key);
  uint64_t cap = hashtable_capacity(mod, h);
  hashtable_require(contract_is_zero_or_power_of_two(cap));
  const uint64_t mask = cap - 1;
  return hash & mask;
}

static inline bool hashtable_offset_occupied(hashtable_module mod,
                                             hashtable_t h, uint64_t offset) {
  hashtable_require(hashtable_valid(mod, h));
  hashtable_require(offset < hashtable_capacity(mod, h));
  unsigned char *loc_key = mod->location_key(h, offset);
  return hashtable_key_is_sentinel(mod, h, loc_key);
}

static inline uint64_t hashtable_lookup_offset(hashtable_module mod,
                                               hashtable_t h,
                                               unsigned char *key) {
  hashtable_require(hashtable_valid(mod, h));
  hashtable_require(!hashtable_key_is_sentinel(mod, h, key));
  uint64_t size_before =
      hashtable_contract_active(mod) ? hashtable_size(mod, h) : 0;

  uint64_t cap = hashtable_capacity(mod, h);
  hashtable_require(contract_is_zero_or_power_of_two(cap));
  const uint64_t mask = cap - 1;
  const uint64_t start_index = hashtable_preferred_offset(mod, h, key);

  for (uint64_t c = 0; c < cap; c++) {
    uint64_t index = (start_index + c) & mask;

    unsigned char *loc_key = mod->location_key(h, index);

    bool already_in_table = hashtable_key_equal(mod, h, loc_key, key);
    bool slot_available =
        already_in_table ? false : hashtable_key_is_sentinel(mod, h, loc_key);
    if (already_in_table || slot_available) {
      hashtable_require(index < hashtable_capacity(mod, h));
      hashtable_require(size_before == hashtable_size(mod, h));
      return index;
    }
  }

  hashtable_require(size_before == hashtable_size(mod, h));
  hashtable_require(hashtable_available(mod, h) == 0);
  return UINT64_MAX;
}

static inline bool hashtable_contains(hashtable_module mod, hashtable_t h,
                                      unsigned char *key) {
  hashtable_require(hashtable_valid(mod, h));
  uint64_t offset = hashtable_lookup_offset(mod, h, key);
  if (offset == UINT64_MAX) {
    return false;
  }
  hashtable_require(offset < hashtable_capacity(mod, h));
  unsigned char *kres = mod->location_key(h, offset);
  return !hashtable_key_is_sentinel(mod, h, kres);
}

static inline unsigned char *
hashtable_lookup_key(hashtable_module mod, hashtable_t h, unsigned char *key) {
  hashtable_require(hashtable_valid(mod, h));
  hashtable_require(!hashtable_key_is_sentinel(mod, h, key));
  uint64_t offset = hashtable_lookup_offset(mod, h, key);
  if (offset == UINT64_MAX) {
    return 0;
  }
  hashtable_require(offset < hashtable_capacity(mod, h));
  unsigned char *kres = mod->location_key(h, offset);
  return hashtable_key_is_sentinel(mod, h, kres) ? 0 : kres;
}

static inline unsigned char *hashtable_lookup_value(hashtable_module mod,
                                                    hashtable_t h,
                                                    unsigned char *key) {
  hashtable_require(hashtable_valid(mod, h));
  hashtable_require(!hashtable_key_is_sentinel(mod, h, key));
  hashtable_require(mod->value_size != 0);
  uint64_t offset = hashtable_lookup_offset(mod, h, key);
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
  hashtable_require(!hashtable_key_is_sentinel(mod, *h, key));

  const bool is_set = mod->value_size == 0;
  bool have_value = value != 0;
  hashtable_require(is_set != have_value);
  hashtable_require(hashtable_available(mod, *h) > 0);

  uint64_t offset = hashtable_lookup_offset(mod, *h, key);
  hashtable_require(offset != UINT64_MAX);
  hashtable_require(offset < hashtable_capacity(mod, *h));

  unsigned char *k_res = mod->location_key(*h, offset);
  unsigned char *v_res = is_set ? 0 : mod->location_value(*h, offset);

  const bool replacing_existing = !hashtable_key_is_sentinel(mod, *h, k_res);
  const uint64_t size_before = hashtable_size(mod, *h);

  if (!replacing_existing) {
    mod->assign_size(h, size_before + 1);
    hashtable_require(mod->size(*h) == (size_before + 1));
  }

  __builtin_memcpy(k_res, key, mod->key_size);
  if (!is_set) {
    __builtin_memcpy(v_res, value, mod->value_size);
  }

  if (replacing_existing) {
    hashtable_require(hashtable_size(mod, *h) == size_before);
  } else {
    hashtable_require(hashtable_size(mod, *h) == (size_before + 1));
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

static inline void hashtable_clear(hashtable_module mod, hashtable_t *h) {
  const unsigned char *sentinel = hashtable_key_sentinel(mod);
  uint64_t cap = hashtable_capacity(mod, *h);

  for (uint64_t offset = 0; offset < cap; offset++) {
    unsigned char *k_res = mod->location_key(*h, offset);
    __builtin_memcpy(k_res, sentinel, mod->key_size);
  }

  mod->assign_size(h, 0);

  hashtable_require(hashtable_size(mod, *h) == 0);
}

static inline void hashtable_remove(hashtable_module mod, hashtable_t *h,
                                    unsigned char *key) {
  hashtable_require(hashtable_valid(mod, *h));

  // Remove can be done by mutation, without allocation
  // Thus the function returns void - doesn't need to indicate errors
  
  if (!hashtable_contains(mod, *h, key)) {
    return;
  }

  uint64_t size = hashtable_contract_active(mod) ? hashtable_size(mod, *h) : 0;
  uint64_t cap = hashtable_capacity(mod, *h);

  const bool is_set = mod->value_size == 0;

  hashtable_t ret = hashtable_create(mod, cap);

  // However that hasn't been implemented yet, so fail here on OOM for now
  hashtable_require(mod->valid(ret));

  hashtable_store_userdata(mod, &ret, hashtable_load_userdata(mod, h));

  for (uint64_t offset = 0; offset < cap; offset++) {
    unsigned char *key_loc = mod->location_key(*h, offset);

    if (hashtable_key_is_sentinel(mod, *h, key_loc)) {
      continue;
    }

    if (hashtable_key_equal(mod, *h, key, key_loc)) {
      continue;
    }

    unsigned char *v_res = is_set ? 0 : mod->location_value(*h, offset);
    hashtable_insert(mod, &ret, key_loc, v_res);
  }
  hashtable_require(hashtable_valid(mod, ret));

  hashtable_require(hashtable_size(mod, ret) == (size - 1));
  hashtable_require(hashtable_contains(mod, ret, key) == false);
  hashtable_destroy(mod, *h);
  *h = ret;
}

static inline void hashtable_dump(hashtable_module mod, hashtable_t h) {
  uint64_t size = mod->size(h);
  uint64_t capacity = mod->capacity(h);

  fprintf(stdout, "hashtable<%lu,%lu>\n", size, capacity);
  for (uint64_t off = 0; off < capacity; off++) {
    unsigned char *k = mod->location_key(h, off);

    uint64_t hash = mod->key_hash(h, k);

    uint64_t pref_offset = hash & (capacity - 1);

    uint64_t key_start = 0;
    __builtin_memcpy(&key_start, k, mod->key_size < 8 ? mod->key_size : 8);

    if (hashtable_key_is_sentinel(mod, h, k)) {
      fprintf(stdout, "  hash[%lu]: %p/%lu available\n", off, k, key_start);
    } else {
      fprintf(stdout, "  hash[%lu]: %p/%lu, pref %lu\n", off, k, key_start,
              pref_offset);
    }
  }

  hashtable_valid_verbose(mod, h, true);
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
    // If that guess was wrong, swap the arguments over
    uint64_t capy = hashtable_capacity(mod, y);
    if (capy > cap) {
      cap = capy;
      s = y;
      l = x;
    }
  }

  for (uint64_t offset = 0; offset < cap; offset++) {
    unsigned char *key_loc = mod->location_key(s, offset);
    if (hashtable_key_is_sentinel(mod, s, key_loc)) {
      continue;
    }

    unsigned char *v_res = is_set ? 0 : mod->location_value(s, offset);
    uint64_t large_offset = hashtable_lookup_offset(mod, l, key_loc);

    hashtable_require(large_offset < hashtable_capacity(mod, l));
    unsigned char *l_key_loc = mod->location_key(l, large_offset);

    bool eq_s = hashtable_key_equal(mod, s, key_loc, l_key_loc);
    hashtable_require(eq_s == hashtable_key_equal(mod, l, key_loc, l_key_loc));
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

static inline hashtable_t hashtable_clone(hashtable_module mod, hashtable_t h) {
  hashtable_require(hashtable_valid(mod, h));
  const bool is_set = mod->value_size == 0;
  uint64_t cap = hashtable_capacity(mod, h);
  hashtable_t ret = hashtable_create(mod, cap);

  if (mod->valid(ret)) {
    hashtable_store_userdata(mod, &ret, hashtable_load_userdata(mod, &h));
    for (uint64_t offset = 0; offset < cap; offset++) {
      unsigned char *src_key = mod->location_key(h, offset);
      unsigned char *src_val = is_set ? 0 : mod->location_value(h, offset);

      unsigned char *dst_key = mod->location_key(ret, offset);
      unsigned char *dst_val = is_set ? 0 : mod->location_value(ret, offset);

      __builtin_memcpy(dst_key, src_key, mod->key_size);
      if (!is_set) {
        __builtin_memcpy(dst_val, src_val, mod->value_size);
      }
    }

    hashtable_require(hashtable_equal(mod, h, ret));
  }
  return ret;
}

static inline hashtable_t hashtable_rehash(hashtable_module mod, hashtable_t h,
                                           uint64_t N) {
  hashtable_require(hashtable_valid(mod, h));
  hashtable_require(contract_is_zero_or_power_of_two(N));
  hashtable_require(N >= hashtable_size(mod, h));
  const bool is_set = mod->value_size == 0;

  hashtable_t ret = hashtable_create(mod, N);

  if (mod->valid(ret)) {
    hashtable_store_userdata(mod, &ret, hashtable_load_userdata(mod, &h));
  }

  if (hashtable_valid(mod, ret)) {
    uint64_t cap = hashtable_capacity(mod, h);
    for (uint64_t offset = 0; offset < cap; offset++) {
      unsigned char *key_loc = mod->location_key(h, offset);
      if (!hashtable_key_is_sentinel(mod, h, key_loc)) {
        unsigned char *v_res = is_set ? 0 : mod->location_value(h, offset);
        hashtable_insert(mod, &ret, key_loc, v_res);
      }
    }
    hashtable_require(hashtable_equal(mod, h, ret));
  }
  return ret;
}

// true on success, in which case h has been replaced
static inline bool hashtable_rehash_double(hashtable_module mod,
                                           hashtable_t *h) {
  uint64_t cap = hashtable_capacity(mod, *h);
  hashtable_t n = hashtable_rehash(mod, *h, 2 * cap);

  if (!hashtable_valid(mod, n)) {
    return false;
  }

  hashtable_destroy(mod, *h);
  *h = n;
  return true;
}

#endif
