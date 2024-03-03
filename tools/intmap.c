#include "intmap.h"
#include "EvilUnit/EvilUnit.h"

#include "hashtable.h"
#include "intmap_util.h"

#include "arena.libc.h"

#define INTSET_CONTRACTS 1

static const struct arena_module_ty arena_libc_contract =
    ARENA_MODULE_INIT(arena_libc,
#if INTSET_CONTRACTS
                      contract_unit_test
#else
                      0
#endif
    );

static arena_module arena_mod = &arena_libc_contract;

enum { intmap_util_struct_sizeof = 4 * 8 };

_Static_assert(sizeof(hashtable_t) == intmap_util_struct_sizeof, "");
_Static_assert(sizeof(intmap_t) == intmap_util_struct_sizeof, "");

static uint64_t intmap_util_key_hash(hashtable_t h, unsigned char *bytes) {
  (void)h;
  // identity at present
  uint64_t r;
  __builtin_memcpy(&r, bytes, 8);
  return r;
}

static bool intmap_util_key_equal(hashtable_t h, const unsigned char *left,
                                  const unsigned char *right) {
  (void)h;
  return __builtin_memcmp(left, right, 8) == 0;
}

INTMAP_UTIL(arena_mod, 0, intmap_util_key_hash, intmap_util_key_equal,
            contract_unit_test);

static const struct hashtable_module_ty mod_state = {
    .create = intmap_util_create,
    .destroy = intmap_util_destroy,
    .valid = intmap_util_valid,
    .store_userdata = intmap_util_store_userdata,
    .load_userdata = intmap_util_load_userdata,
    .key_align = 8,
    .key_size = 8,
    .value_align = 8,
    .value_size = 8,
    .key_hash = intmap_util_key_hash,
    .key_equal = intmap_util_key_equal,
    .sentinel = (const unsigned char *)&intmap_util_sentinel,
    .size = intmap_util_size,
    .capacity = intmap_util_capacity,
    .lookup_offset = intmap_util_lookup_offset,
    .location_key = intmap_util_location_key,
    .location_value = intmap_util_location_value,
    .set_size = intmap_util_set_size,
    .maybe_remove = 0,
#if INTSET_CONTRACTS
    .maybe_contract = contract_unit_test,
#else
    .maybe_contract = 0,
#endif
};

static const hashtable_module mod = &mod_state;

static inline hashtable_t intmap_util_to_hash(intmap_t s) {
  hashtable_t r;
  __builtin_memcpy(&r.state, &s.state, intmap_util_struct_sizeof);
  return r;
}

static inline intmap_t intmap_util_to_set(hashtable_t s) {
  intmap_t r;
  __builtin_memcpy(&r.state, &s.state, intmap_util_struct_sizeof);
  return r;
}

intmap_t intmap_create(uint64_t size) {
  return intmap_util_to_set(hashtable_create(mod, size));
}

void intmap_destroy(intmap_t s) {
  hashtable_destroy(mod, intmap_util_to_hash(s));
}
bool intmap_valid(intmap_t s) {
  return hashtable_valid(mod, intmap_util_to_hash(s));
}

bool intmap_equal(intmap_t x, intmap_t y) {
  return hashtable_equal(mod, intmap_util_to_hash(x), intmap_util_to_hash(y));
}

intmap_t intmap_rehash(intmap_t s, uint64_t size) {
  return intmap_util_to_set(
      hashtable_rehash(mod, intmap_util_to_hash(s), size));
}

uint64_t intmap_size(intmap_t s) {
  return hashtable_size(mod, intmap_util_to_hash(s));
}

uint64_t intmap_capacity(intmap_t s) {
  return hashtable_capacity(mod, intmap_util_to_hash(s));
}

bool intmap_contains(intmap_t s, uint64_t k) {
  return hashtable_contains(mod, intmap_util_to_hash(s), (unsigned char *)&k);
}

uint64_t intmap_lookup(intmap_t s, uint64_t k) {
  unsigned char *v =
      hashtable_lookup_value(mod, intmap_util_to_hash(s), (unsigned char *)&k);
  uint64_t res = UINT64_MAX;
  if (v) {
    __builtin_memcpy(&res, v, 8);
  }
  return res;
}

void intmap_insert(intmap_t *s, uint64_t v, uint64_t k) {
  hashtable_t h = intmap_util_to_hash(*s);
  hashtable_insert(mod, &h, (unsigned char *)&v, (unsigned char *)&k);
  *s = intmap_util_to_set(h);
}
