#include "intset.h"

#include "EvilUnit/EvilUnit.h"

#include "hashtable.h"
#include "intset_util.h"

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

_Static_assert(sizeof(hashtable_t) == intset_util_struct_sizeof, "");
_Static_assert(sizeof(intset_t) == intset_util_struct_sizeof, "");

static const uint64_t sentinel = UINT64_MAX;


// INTEST_UTIL(arena_mod, contract_unit_test);

static hashtable_t intset_util_create(uint64_t size) {
#if INTSET_CONTRACTS
  contract_unit_test(contract_is_zero_or_power_of_two(size), "size", 4);
#endif

  arena_t a = arena_create(arena_mod, 8 * size);
  arena_allocate_into_existing_capacity(arena_mod, &a, 8 * size);
  unsigned char *p = arena_base_address(arena_mod, a);
  for (uint64_t i = 0; i < size; i++) {
    unsigned char *c = p + 8 * i;
    __builtin_memcpy(c, &sentinel, 8);
  }

  hashtable_t r = intset_util_arena_to_hash(a);
  return r;
}

static void intset_util_destroy(hashtable_t h) {
  arena_destroy(arena_mod, intset_util_hash_to_arena(h));
}

static bool intset_util_valid(hashtable_t h) {
  return arena_valid(arena_mod, intset_util_hash_to_arena(h));
}

static uint64_t intset_util_key_hash(hashtable_t h, unsigned char *bytes) {
  (void)h;
  // identity at present
  uint64_t r;
  __builtin_memcpy(&r, bytes, 8);
  return r;
}

static bool intset_util_key_equal(hashtable_t h, const unsigned char *left,
               const unsigned char *right) {
  (void)h;
  return __builtin_memcmp(left, right, 8) == 0;
}

static uint64_t intset_util_size(hashtable_t h) { return h.state[3]; }

static uint64_t intset_util_capacity(hashtable_t h) {
  arena_t a = intset_util_hash_to_arena(h);
  uint64_t r = arena_size(arena_mod, a) / 8;
#if INTSET_CONTRACTS
  contract_unit_test(contract_is_zero_or_power_of_two(r), "cap", 3);
#endif
  return r;
}

static uint64_t intset_util_available(hashtable_t h) { return intset_util_capacity(h) - intset_util_size(h); }

static unsigned char *intset_util_location_key(hashtable_t h, uint64_t offset) {
  arena_t a = intset_util_hash_to_arena(h);
  unsigned char *p = arena_base_address(arena_mod, a);
  return p + offset * 8;
}

static uint64_t intset_util_lookup_offset(hashtable_t h, unsigned char *key) {
  uint64_t hash = intset_util_key_hash(h, key);
  uint64_t cap = intset_util_capacity(h);

#if INTSET_CONTRACTS
  contract_unit_test(contract_is_power_of_two(cap), "cap offset", 10);
#endif

  const uint64_t mask = cap - 1;
  const uint64_t start_index = hash;

  for (uint64_t c = 0; c < cap; c++) {
    uint64_t index = (start_index + c) & mask;

    unsigned char *loc_key = intset_util_location_key(h, index);

    if (intset_util_key_equal(h, loc_key, key)) {
      // Found key
      return index;
    }

    if (intset_util_key_equal(h, loc_key, (unsigned char *)&sentinel)) {
      // Found a space
      return index;
    }
  }

#if INTSET_CONTRACTS
  contract_unit_test(intset_util_available(h) == 0, "avail 0", 7);
#endif

  return UINT64_MAX;
}

static void intset_util_set_size(hashtable_t *h, uint64_t s) { h->state[3] = s; }

static const struct hashtable_module_ty mod_state = {
    .create = intset_util_create,
    .destroy = intset_util_destroy,
    .valid = intset_util_valid,
    .key_align = 8,
    .key_size = 8,
    .value_align = 1,
    .value_size = 0,
    .key_hash = intset_util_key_hash,
    .key_equal = intset_util_key_equal,
    .sentinel = (const unsigned char *)&sentinel,
    .size = intset_util_size,
    .capacity = intset_util_capacity,
    .lookup_offset = intset_util_lookup_offset,
    .location_key = intset_util_location_key,
    .location_value = 0,
    .set_size = intset_util_set_size,
    .maybe_remove = 0,
#if INTSET_CONTRACTS
    .maybe_contract = contract_unit_test,
#else
    .maybe_contract = 0,
#endif
};

static const hashtable_module mod = &mod_state;

intset_t intset_create(uint64_t size) {
  return intset_util_to_set(hashtable_create(mod, size));
}

void intset_destroy(intset_t s) { hashtable_destroy(mod, intset_util_to_hash(s)); }
bool intset_valid(intset_t s) { return hashtable_valid(mod, intset_util_to_hash(s)); }

bool intset_equal(intset_t x, intset_t y) {
  return hashtable_equal(mod, intset_util_to_hash(x), intset_util_to_hash(y));
}

intset_t intset_rehash(intset_t s, uint64_t size) {
  return intset_util_to_set(hashtable_rehash(mod, intset_util_to_hash(s), size));
}

uint64_t intset_size(intset_t s) { return hashtable_size(mod, intset_util_to_hash(s)); }

uint64_t intset_capacity(intset_t s) {
  return hashtable_capacity(mod, intset_util_to_hash(s));
}

bool intset_contains(intset_t s, uint64_t v) {
  return hashtable_contains(mod, intset_util_to_hash(s), (unsigned char *)&v);
}

void intset_insert(intset_t *s, uint64_t v) {
  hashtable_t h = intset_util_to_hash(*s);
  hashtable_insert(mod, &h, (unsigned char *)&v, 0);
  *s = intset_util_to_set(h);
}

#if 0 // todo
void intset_remove(intset_t* s, uint64_t v)
{
  hashtable_remove(mod, intset_util_to_hash(s), (unsigned char*)&v);
}
#endif

static MODULE(create_destroy) {
  TEST("size 0") {
    intset_t a = intset_create(0);
    CHECK(intset_valid(a));
    CHECK(intset_size(a) == 0);
    CHECK(intset_capacity(a) == 0);
    intset_destroy(a);
  }

  TEST("non-zero") {
    intset_t a = intset_create(4);
    CHECK(intset_valid(a));
    CHECK(intset_size(a) == 0);
    CHECK(intset_capacity(a) == 4);
    intset_destroy(a);
  }

  TEST("non-zero, non-power-two") {
    DEATH(intset_create(5));
    DEATH(intset_create(18));
  }

  TEST("fail a precondition") {
    intset_t a = {0};
    CHECK(!intset_valid(a));
    DEATH(intset_destroy(a));
  }
}

static MODULE(operations) {
  intset_t s = intset_create(4);
  CHECK(intset_valid(s));
  CHECK(intset_size(s) == 0);
  CHECK(intset_capacity(s) == 4);

  TEST("insert and retrieve single value") {
    CHECK(intset_size(s) == 0);
    CHECK(!intset_contains(s, 42));

    CHECK(intset_size(s) == 0);
    intset_insert(&s, 42);
    CHECK(intset_size(s) == 1);
    CHECK(intset_contains(s, 42));
  }

  TEST("insert two values") {
    CHECK(intset_size(s) == 0);
    CHECK(!intset_contains(s, 42));
    CHECK(!intset_contains(s, 101));

    intset_insert(&s, 42);
    CHECK(intset_size(s) == 1);
    CHECK(intset_contains(s, 42));

    intset_insert(&s, 101);
    CHECK(intset_size(s) == 2);
    CHECK(intset_contains(s, 101));

    CHECK(intset_contains(s, 42));
  }

  TEST("dies on insert limit of capacity") {
    CHECK(intset_available(s) == 4);
    intset_insert(&s, 0);
    intset_insert(&s, 1);
    intset_insert(&s, 2);
    intset_insert(&s, 3);
    CHECK(intset_available(s) == 0);
    DEATH(intset_insert(&s, 4));

    // at capacity, also can't insert an existing value
    DEATH(intset_insert(&s, 2));
  }

  TEST("overwriting existing is ok while avail > 0") {
    intset_insert(&s, 0);
    intset_insert(&s, 1);
    intset_insert(&s, 2);
    intset_insert(&s, 0);
    intset_insert(&s, 1);
    intset_insert(&s, 2);

    CHECK(intset_available(s) == 1);
    intset_insert(&s, 3);
    CHECK(intset_available(s) == 0);

    // and then not ok
    DEATH(intset_insert(&s, 0));
    DEATH(intset_insert(&s, 1));
  }

  intset_destroy(s);
}

static MODULE(rehash_empty) {
  TEST("same size") {
    intset_t s = intset_create(4);
    intset_t t = intset_rehash(s, 4);
    CHECK(intset_equal(s, t));
    intset_destroy(s);
    intset_destroy(t);
  }

  TEST("increase size") {
    intset_t s = intset_create(4);
    intset_t t = intset_rehash(s, 8);
    CHECK(intset_equal(s, t));
    intset_destroy(s);
    intset_destroy(t);
  }

  TEST("decrease size") {
    intset_t s = intset_create(4);
    intset_t t = intset_rehash(s, 2);
    CHECK(intset_equal(s, t));
    intset_destroy(s);
    intset_destroy(t);
  }

  TEST("rehash to zero") {
    intset_t s = intset_create(4);
    intset_rehash(s, 0);
    intset_destroy(s);
  }

  TEST("rehash from zero") {
    intset_t s = intset_create(0);
    intset_rehash(s, 4);
    intset_destroy(s);
  }

  TEST("rehash to non-power-two fails") {
    intset_t s = intset_create(4);
    DEATH(intset_rehash(s, 3));
    intset_destroy(s);
  }
}

static MODULE(rehash_partial) {
  intset_t s = intset_create(8);
  intset_insert(&s, 1);
  intset_insert(&s, 2);

  TEST("same size") {
    intset_t t = intset_rehash(s, 8);
    CHECK(intset_equal(s, t));
    intset_destroy(t);
  }

  TEST("increase size") {
    intset_t t = intset_rehash(s, 16);
    CHECK(intset_equal(s, t));
    intset_destroy(t);
  }

  TEST("decrease size") {
    intset_t t = intset_rehash(s, 4);
    CHECK(intset_equal(s, t));
    intset_destroy(t);
  }

  TEST("decrease to full") {
    intset_t t = intset_rehash(s, 2);
    CHECK(intset_equal(s, t));
    intset_destroy(t);
  }

  TEST("rehash cannot drop elements") { DEATH(intset_rehash(s, 1)); }

  intset_destroy(s);
}

MODULE(tools_intset) {
  DEPENDS(create_destroy);
  DEPENDS(operations);
  DEPENDS(rehash_empty);
  DEPENDS(rehash_partial);
}

MAIN_MODULE() { DEPENDS(tools_intset); }