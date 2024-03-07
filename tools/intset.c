#include "intset.h"

#define INTSET_CONTRACTS 1

#if INTSET_CONTRACTS
#else
#define EVILUNIT_CONTRACT_DEFAULT_IMPLEMENTATION                               \
  EVILUNIT_CONTRACT_IMPLEMENTATION_NONE
#endif

#include "EvilUnit/EvilUnit.h"

#include "hashtable.h"
#include "intmap_util.h"

#include "arena.libc.h"

static const struct arena_module_ty arena_libc_contract =
    ARENA_MODULE_INIT(arena_libc,
#if INTSET_CONTRACTS
                      contract_unit_test
#else
                      0
#endif
    );

static arena_module arena_mod = &arena_libc_contract;

enum { intset_util_struct_sizeof = 4 * 8 };

_Static_assert(sizeof(hashtable_t) == intset_util_struct_sizeof, "");
_Static_assert(sizeof(intset_t) == intset_util_struct_sizeof, "");

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

INTMAP_UTIL(arena_mod, 1, intset_util_key_hash, intset_util_key_equal,
            contract_unit_test);

static const struct hashtable_module_ty mod_state = {
    .create = intmap_util_create,
    .destroy = intmap_util_destroy,
    .valid = intmap_util_valid,
    .store_userdata = intmap_util_store_userdata,
    .load_userdata = intmap_util_load_userdata,
    .key_align = 8,
    .key_size = 8,
    .value_align = 1,
    .value_size = 0,
    .key_hash = intset_util_key_hash,
    .key_equal = intset_util_key_equal,
    .sentinel = (const unsigned char *)&intmap_util_sentinel,
    .size = intmap_util_size,
    .capacity = intmap_util_capacity,
    .location_key = intmap_util_location_key,
    .location_value = 0,
    .assign_size = intmap_util_assign_size,
#if INTSET_CONTRACTS
    .maybe_contract = contract_unit_test,
#else
    .maybe_contract = 0,
#endif
};

static const hashtable_module mod = &mod_state;

static inline hashtable_t intset_util_to_hash(intset_t s) {
  hashtable_t r;
  __builtin_memcpy(&r.state, &s.state, intset_util_struct_sizeof);
  return r;
}

static inline intset_t intset_util_to_set(hashtable_t s) {
  intset_t r;
  __builtin_memcpy(&r.state, &s.state, intset_util_struct_sizeof);
  return r;
}

intset_t intset_create(uint64_t size) {
  return intset_util_to_set(hashtable_create(mod, size));
}

void intset_destroy(intset_t s) {
  hashtable_destroy(mod, intset_util_to_hash(s));
}

bool intset_valid(intset_t s) {
  return hashtable_valid(mod, intset_util_to_hash(s));
}

bool intset_equal(intset_t x, intset_t y) {
  return hashtable_equal(mod, intset_util_to_hash(x), intset_util_to_hash(y));
}

intset_t intset_clone(intset_t x)
{
  return intset_util_to_set(hashtable_clone(mod, intset_util_to_hash(x)));
}

intset_t intset_rehash(intset_t s, uint64_t size) {
  return intset_util_to_set(
      hashtable_rehash(mod, intset_util_to_hash(s), size));
}

uint64_t intset_size(intset_t s) {
  return hashtable_size(mod, intset_util_to_hash(s));
}

uint64_t intset_capacity(intset_t s) {
  return hashtable_capacity(mod, intset_util_to_hash(s));
}

bool intset_contains(intset_t s, uint64_t v) {
  return hashtable_contains(mod, intset_util_to_hash(s), (unsigned char *)&v);
}

// No lookup as no value to look up

void intset_insert(intset_t *s, uint64_t v) {
  hashtable_t h = intset_util_to_hash(*s);
  hashtable_insert(mod, &h, (unsigned char *)&v, 0);
  *s = intset_util_to_set(h);
}

void intset_remove(intset_t* s, uint64_t v)
{
  hashtable_t h = intset_util_to_hash(*s);
  hashtable_remove(mod, &h, (unsigned char *)&v);
  *s = intset_util_to_set(h);
}

void intset_clear(intset_t *s) {
  hashtable_t h = intset_util_to_hash(*s);
  hashtable_clear(mod, &h);
  *s = intset_util_to_set(h);
}

static MODULE(create_destroy) {

  TEST("module parameters") {
    CHECK(hashtable_key_size(mod) == 8);
    CHECK(hashtable_key_align(mod) == 8);
    CHECK(hashtable_value_size(mod) == 0);
    CHECK(hashtable_value_align(mod) == 1);
  }

  TEST("size 0") {
    intset_t a = intset_create(0);
    CHECK(intset_valid(a));
    CHECK(intset_size(a) == 0);
    CHECK(intset_capacity(a) == 0);
    intset_t cp = intset_clone(a);
    CHECK(intset_equal(a,cp));
    intset_destroy(cp);
    intset_destroy(a);
  }

  TEST("non-zero") {
    intset_t a = intset_create(4);
    CHECK(intset_valid(a));
    CHECK(intset_size(a) == 0);
    CHECK(intset_capacity(a) == 4);
    intset_t cp = intset_clone(a);
    CHECK(intset_equal(a,cp));
    intset_destroy(cp);
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

    intset_remove(&s, 42);
    CHECK(intset_size(s) == 0);
    CHECK(!intset_contains(s, 42));
  }

  TEST("insert two values") {
    CHECK(intset_size(s) == 0);
    CHECK(!intset_contains(s, 42));
    CHECK(!intset_contains(s, 101));

    intset_insert(&s, 42);
    CHECK(intset_size(s) == 1);
    CHECK(intset_contains(s, 42));

    CHECK(intset_valid(s));

    intset_insert(&s, 101);
    CHECK(intset_size(s) == 2);
    CHECK(intset_contains(s, 101));

    CHECK(intset_valid(s));

    CHECK(intset_contains(s, 42));

    intset_remove(&s, 42);
    intset_remove(&s, 101);
    CHECK(intset_size(s) == 0);
    CHECK(!intset_contains(s, 42));
    CHECK(!intset_contains(s, 101));
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
    intset_t t = intset_rehash(s, 0);
    intset_destroy(t);
    intset_destroy(s);
  }

  TEST("rehash from zero") {
    intset_t s = intset_create(0);
    intset_t t = intset_rehash(s, 4);
    intset_destroy(t);
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

MODULE(intset) {
  DEPENDS(create_destroy);
  DEPENDS(operations);
  DEPENDS(rehash_empty);
  DEPENDS(rehash_partial);
}
