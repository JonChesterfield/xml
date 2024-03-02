#include "stringtable.h"
#include "intset_util.h"

#include "arena.libc.h"

#include "contract.h"

#include <assert.h>
#include <stddef.h>

#include "EvilUnit/EvilUnit.h"

#define STRINGTABLE_CONTRACTS 1

static const struct arena_module_ty arena_libc_contract =
    ARENA_MODULE_INIT(arena_libc,
#if STRINGTABLE_CONTRACTS
                      contract_unit_test
#else
                      0
#endif
    );

static arena_module arena_mod = &arena_libc_contract;

static uint64_t intset_util_key_hash(hashtable_t h, unsigned char *bytes);
static bool intset_util_key_equal(hashtable_t h, const unsigned char *left,
                                  const unsigned char *right);

INTSET_UTIL(arena_mod, intset_util_key_hash, intset_util_key_equal,
            contract_unit_test);

static const struct hashtable_module_ty hashtable_mod_state = {
    .create = intset_util_create,
    .destroy = intset_util_destroy,
    .valid = intset_util_valid,
    .store_userdata = intset_util_store_userdata,
    .load_userdata = intset_util_load_userdata,
    .key_align = 8,
    .key_size = 8,
    .value_align = 1,
    .value_size = 0,
    .key_hash = intset_util_key_hash,
    .key_equal = intset_util_key_equal,
    .sentinel = (const unsigned char *)&intset_util_sentinel,
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

static const hashtable_module hashtable_mod = &hashtable_mod_state;

static uint64_t word_from_bytes(const unsigned char *bytes) {
  uint64_t r;
  __builtin_memcpy(&r, bytes, 8);
  return r;
}

static uint64_t intset_util_key_hash(hashtable_t h, unsigned char *bytes) {
  assert(hashtable_load_userdata(hashtable_mod, &h) != 0);
  char *arena_base = (char *)hashtable_load_userdata(hashtable_mod, &h);

  uint64_t word = word_from_bytes(bytes);
  assert(word != UINT64_MAX);

  assert(arena_base != 0);

  const char *str = arena_base + word_from_bytes(bytes);

  return str[0]; // not a good hash
}

static bool intset_util_key_equal(hashtable_t h, const unsigned char *left,
                                  const unsigned char *right) {
  assert(hashtable_load_userdata(hashtable_mod, &h) != 0);
  char *arena_base = (char *)hashtable_load_userdata(hashtable_mod, &h);

  uint64_t word_left = word_from_bytes(left);
  uint64_t word_right = word_from_bytes(right);

  bool sentinel_left = word_left == UINT64_MAX;
  bool sentinel_right = word_right == UINT64_MAX;

  if (sentinel_left && sentinel_right) {
    return true;
  }

  if (sentinel_left || sentinel_right) {
    return false;
  }

  const char *L = arena_base + word_left;
  const char *R = arena_base + word_right;

  uint64_t N = __builtin_strlen(L);
  if (N != __builtin_strlen(R)) {
    return false;
  }

  return __builtin_memcmp(L, R, N) == 0;
}

stringtable_t stringtable_create(void) {
  stringtable_t tab;
  tab.hash = intset_util_create_using_arena(arena_mod, 8);
  tab.arena = arena_create(arena_mod, 128);
  tab.arena_mod = arena_mod;
  return tab;
}

void stringtable_destroy(stringtable_t tab) {
  intset_util_destroy_using_arena(arena_mod, tab.hash);
  arena_destroy(arena_mod, tab.arena);
}

bool stringtable_valid(stringtable_t tab) {
  return intset_util_valid_using_arena(arena_mod, tab.hash) &&
         arena_valid(arena_mod, tab.arena);
}

static void pass_userdata(stringtable_t *tab) {
  hashtable_store_userdata(hashtable_mod, &tab->hash,
                           (uint64_t)arena_base_address(arena_mod, tab->arena));
}

// if index is from the table, does not fail
const char *stringtable_lookup(stringtable_t *tab, stringtable_index_t index) {
  pass_userdata(tab);

  unsigned char *r = hashtable_lookup_key(hashtable_mod, tab->hash,
                                          (unsigned char *)&index.value);
  if (!r) {
    return 0;
  }

  uint64_t v;
  __builtin_memcpy(&v, r, 8);

  return (const char *)arena_base_address(arena_mod, tab->arena) + v;
}

stringtable_index_t stringtable_record(stringtable_t *tab, uint64_t N) {
  const stringtable_index_t failure = {
      .value = UINT64_MAX,
  };

  uint64_t arena_level = (char *)arena_next_address(arena_mod, tab->arena) -
                         (char *)arena_base_address(arena_mod, tab->arena) - N;

  const char *str = (char *)arena_next_address(arena_mod, tab->arena) - N;

  // Make current arena base available after allocating and before calling into
  // hashtable
  pass_userdata(tab);

  if (hashtable_available(hashtable_mod, tab->hash) < 2) {
    if (!hashtable_rehash_double(hashtable_mod, &tab->hash)) {
      return failure;
    }
    pass_userdata(tab);
  }

  // Check for existing value and deallocate str if present
  {
    unsigned char *r = hashtable_lookup_key(hashtable_mod, tab->hash,
                                            (unsigned char *)&arena_level);
    if (r) {
      uint64_t v;
      __builtin_memcpy(&v, r, 8);
      const stringtable_index_t success = {
          .value = v,
      };
      assert(__builtin_memcmp(str, stringtable_lookup(tab, success), N) == 0);
      return success;
    }
  }

  // Record it
  hashtable_insert(hashtable_mod, &tab->hash, (unsigned char *)&arena_level, 0);

  const stringtable_index_t success = {
      .value = arena_level,
  };
  assert(__builtin_memcmp(str, stringtable_lookup(tab, success), N) == 0);
  return success;
}

// only fails on out of memory
stringtable_index_t stringtable_insert(stringtable_t *tab, const char *str,
                                       size_t N) {
  const stringtable_index_t failure = {
      .value = UINT64_MAX,
  };

  uint64_t arena_level = (char *)arena_next_address(arena_mod, tab->arena) -
                         (char *)arena_base_address(arena_mod, tab->arena);

  
  // Include the trailing 0
  if (!arena_append_bytes(arena_mod, &tab->arena, (const unsigned char *)str,
                          N)) {
    return failure;
  }

  stringtable_index_t r = stringtable_record(tab, N);
  if (!stringtable_index_valid(r)) {
    arena_change_allocation(arena_mod, &tab->arena, arena_level);
  }

  return r;
}

MODULE(stringtable) {
  TEST("create / destroy") {
    stringtable_t tab = stringtable_create();
    CHECK(stringtable_valid(tab));
    stringtable_destroy(tab);
  }

  TEST("insert one") {
    stringtable_t tab = stringtable_create();
    CHECK(stringtable_valid(tab));

    const char *str = "foobar";
    size_t N = __builtin_strlen(str);
    stringtable_index_t idx = stringtable_insert(&tab, str, N + 1);
    CHECK(idx.value != UINT64_MAX);
    CHECK(idx.value == 0);

    const char *res = stringtable_lookup(&tab, idx);
    CHECK(__builtin_memcmp(str, res, N + 1) == 0);

    stringtable_destroy(tab);
  }

  TEST("insert same pointer twice") {
    stringtable_t tab = stringtable_create();
    CHECK(stringtable_valid(tab));

    const char *str = "foobar";
    size_t N = __builtin_strlen(str);

    {
      stringtable_index_t idx = stringtable_insert(&tab, str, N + 1);
      CHECK(idx.value != UINT64_MAX);
      CHECK(idx.value == 0);
      const char *res = stringtable_lookup(&tab, idx);
      CHECK(__builtin_memcmp(str, res, N + 1) == 0);
    }

    {
      stringtable_index_t idx = stringtable_insert(&tab, str, N + 1);
      CHECK(idx.value != UINT64_MAX);
      CHECK(idx.value == 0);
      const char *res = stringtable_lookup(&tab, idx);
      CHECK(__builtin_memcmp(str, res, N + 1) == 0);
    }

    stringtable_destroy(tab);
  }

  TEST("insert different strings") {
    stringtable_t tab = stringtable_create();
    CHECK(stringtable_valid(tab));

    const char *strs[] = {
        "foobar",
        "other",
        "another",
        "more",
    };
    size_t offsets[] = {
        0,
        7,
        13,
        21,
    };
    size_t N = 4;

    for (size_t rep = 0; rep < 40; rep++) {
      for (size_t i = 0; i < N; i++) {
        size_t len = __builtin_strlen(strs[i]) + 1;
        stringtable_index_t idx = stringtable_insert(&tab, strs[i], len);
        CHECK(idx.value != UINT64_MAX);
        CHECK(idx.value == offsets[i]);
        const char *res = stringtable_lookup(&tab, idx);
        CHECK(__builtin_memcmp(strs[i], res, len) == 0);
      }
    }

    stringtable_destroy(tab);
  }
}
