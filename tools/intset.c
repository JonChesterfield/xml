#include "intset.h"

#include "EvilUnit/EvilUnit.h"

#include "hashtable.h"

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

arena_module arena_mod = &arena_libc_contract;

enum { table_sizeof = 4 * 8 };

_Static_assert(sizeof(hashtable_t) == table_sizeof, "");
_Static_assert(sizeof(intset_t) == table_sizeof, "");

static const uint64_t sentinel = UINT64_MAX;

static hashtable_t to_hash(intset_t s) {
  hashtable_t r;
  __builtin_memcpy(&r.state, &s.state, table_sizeof);
  return r;
}

static intset_t to_set(hashtable_t s) {
  intset_t r;
  __builtin_memcpy(&r.state, &s.state, table_sizeof);
  return r;
}

static arena_t hash_to_arena(hashtable_t h) {
  arena_t a;
  a.base = h.state[0];
  a.next = h.state[1];
  a.limit = h.state[2];
  return a;
}

static hashtable_t arena_to_hash(arena_t a) {
  hashtable_t h;
  h.state[0] = a.base;
  h.state[1] = a.next;
  h.state[2] = a.limit;
  h.state[3] = 0;
  return h;
}

static hashtable_t create(uint64_t size) {
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

  hashtable_t r = arena_to_hash(a);
  return r;
}

static void destroy(hashtable_t h) {
  arena_destroy(arena_mod, hash_to_arena(h));
}

static bool valid(hashtable_t h) {
  return arena_valid(arena_mod, hash_to_arena(h));
}

uint64_t key_hash(unsigned char *bytes) {
  // identity at present
  uint64_t r;
  __builtin_memcpy(&r, bytes, 8);
  return r;
}

uint64_t size(hashtable_t h) { return h.state[3]; }

uint64_t capacity(hashtable_t h) {
  arena_t a = hash_to_arena(h);
  uint64_t r = arena_size(arena_mod, a) / 8;
#if INTSET_CONTRACTS
  contract_unit_test(contract_is_zero_or_power_of_two(r), "cap", 3);
#endif
  return r;
}

unsigned char *location_key(hashtable_t h, uint64_t offset) {
  arena_t a = hash_to_arena(h);
  unsigned char *p = arena_base_address(arena_mod, a);
  return p + offset * 8;
}

uint64_t lookup_offset(hashtable_t h, unsigned char *key) {
  uint64_t hash = key_hash(key);
  uint64_t cap = capacity(h);

#if INTSET_CONTRACTS
  contract_unit_test(contract_is_power_of_two(cap), "cap offset", 10);
#endif

  const uint64_t mask = cap - 1;
  const uint64_t start_index = hash;

  for (uint64_t c = 0; c < cap; c++) {
    uint64_t index = (start_index + c) & mask;

    unsigned char *loc_key = location_key(h, index);

    if (__builtin_memcmp(loc_key, key, 8) == 0) {
      // Found key
      return index;
    }

    if (__builtin_memcmp(loc_key, &sentinel, 8) == 0) {
      // Found a space
      return index;
    }
  }

  contract_unit_test(false, "look offset", 11);
  return UINT64_MAX;
}

static void set_size(hashtable_t *h, uint64_t s) { h->state[3] = s; }

static const struct hashtable_module_ty mod_state = {
    .create = create,
    .destroy = destroy,
    .valid = valid,
    .key_align = 8,
    .key_size = 8,
    .value_align = 1,
    .value_size = 0,
    .key_hash = key_hash,
    .sentinel = (unsigned char *)&sentinel,
    .size = size,
    .capacity = capacity,
    .lookup_offset = lookup_offset,
    .location_key = location_key,
    .location_value = 0,
    .set_size = set_size,
    .maybe_remove = 0,
    .maybe_contract = contract_unit_test,
};

const hashtable_module mod = &mod_state;

intset_t intset_create(uint64_t size) {
  return to_set(hashtable_create(mod, size));
}

void intset_destroy(intset_t s) { hashtable_destroy(mod, to_hash(s)); }
bool intset_valid(intset_t s) { return hashtable_valid(mod, to_hash(s)); }

bool intset_equal(intset_t x, intset_t y) {
  return hashtable_equal(mod, to_hash(x), to_hash(y));
}

intset_t intset_rehash(intset_t s, uint64_t size) {
  return to_set(hashtable_rehash(mod, to_hash(s), size));
}

uint64_t intset_size(intset_t s) { return hashtable_size(mod, to_hash(s)); }

uint64_t intset_capacity(intset_t s) {
  return hashtable_capacity(mod, to_hash(s));
}

bool intset_contains(intset_t s, uint64_t v) {
  return hashtable_contains(mod, to_hash(s), (unsigned char *)&v);
}

void intset_insert(intset_t *s, uint64_t v) {
  hashtable_t h = to_hash(*s);
  hashtable_insert(mod, &h, (unsigned char *)&v, 0);
  *s = to_set(h);
}

#if 0 // todo
void intset_remove(intset_t* s, uint64_t v)
{
  hashtable_remove(mod, to_hash(s), (unsigned char*)&v);
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

  TEST("non-zero, odd") { DEATH(intset_create(5)); }

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
    DEATH(intset_insert(&s, 3));
    CHECK(intset_available(s) == 0);
    DEATH(intset_insert(&s, 4));

    // at capacity, also can't insert an wxisting value
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
    DEATH(intset_insert(&s, 3));
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
    DEATH(intset_rehash(s, 0));
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

  TEST("to full fails") { DEATH(intset_rehash(s, 2)); }

  intset_destroy(s);
}

MODULE(tools_intset) {
  DEPENDS(create_destroy);
  DEPENDS(operations);
  DEPENDS(rehash_empty);
  DEPENDS(rehash_partial);
}

MAIN_MODULE() { DEPENDS(tools_intset); }
