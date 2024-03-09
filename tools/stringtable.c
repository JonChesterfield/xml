#include "stringtable.h"
#include "intmap_util.h"

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

static uint64_t intset_util_key_hash(hashtable_user_t,
                                     unsigned char *bytes);
static bool intset_util_key_equal(hashtable_user_t, 
                                  const unsigned char *left,
                                  const unsigned char *right);

INTMAP_UTIL(arena_mod, 1, intset_util_key_hash, intset_util_key_equal,
            contract_unit_test);

static const struct hashtable_module_ty hashtable_mod_state = {
    .create = intmap_util_create,
    .destroy = intmap_util_destroy,
    .valid = intmap_util_valid,
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

static const hashtable_module hashtable_mod = &hashtable_mod_state;

static uint64_t word_from_bytes(const unsigned char *bytes) {
  uint64_t r;
  __builtin_memcpy(&r, bytes, 8);
  return r;
}

static uint64_t intset_util_key_hash(hashtable_user_t user,
                                     unsigned char *bytes) {
  (void)user;
  char *arena_base = (char *)user.state;

  uint64_t word = word_from_bytes(bytes);
  assert(word != UINT64_MAX);

  assert(arena_base != 0);

  const char *str = arena_base + word_from_bytes(bytes);

  return str[0]; // not a good hash
}

static bool intset_util_key_equal(hashtable_user_t user, 
                                  const unsigned char *left,
                                  const unsigned char *right) {
  (void)user;

  char *arena_base = (char *)user.state;

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

  uint64_t left_size = word_from_bytes(L - 8);
  uint64_t right_size = word_from_bytes(R - 8);

  if (left_size != right_size) {
    return false;
  }

  return __builtin_memcmp(L, R, left_size) == 0;
}

stringtable_t stringtable_create(void) {
  stringtable_t tab;
  tab.hash = intmap_util_create_using_arena(arena_mod, 8, true);
  tab.arena = arena_create(arena_mod, 128);
  tab.arena_mod = arena_mod;
  return tab;
}

void stringtable_destroy(stringtable_t tab) {
  intmap_util_destroy_using_arena(arena_mod, tab.hash);
  arena_destroy(arena_mod, tab.arena);
}

bool stringtable_valid(stringtable_t tab) {
  return intmap_util_valid_using_arena(arena_mod, tab.hash) &&
         arena_valid(arena_mod, tab.arena);
}

static hashtable_user_t stringtable_to_userdata(stringtable_t *tab) {
  return (hashtable_user_t){
      .state = (uint64_t)arena_base_address(arena_mod, tab->arena)};
}

bool stringtable_contains(stringtable_t *tab, stringtable_index_t index) {
  return hashtable_contains(hashtable_mod, stringtable_to_userdata(tab),
                            tab->hash, (unsigned char *)&index.value);
}

// if index is from the table, does not fail
const char *stringtable_lookup(stringtable_t *tab, stringtable_index_t index) {

  unsigned char *r =
      hashtable_lookup_key(hashtable_mod, stringtable_to_userdata(tab),
                           tab->hash, (unsigned char *)&index.value);
  if (!r) {
    return 0;
  }

  uint64_t v = word_from_bytes(r);

  return (const char *)arena_base_address(arena_mod, tab->arena) + v;
}

size_t stringtable_lookup_size(stringtable_t *tab, stringtable_index_t index) {
  // size is stored immediately before the bytes of the string in the arena
  unsigned char *r = stringtable_lookup(tab, index);
  if (!r) {
    return SIZE_MAX;
  }
  return word_from_bytes(r-8);
}


static stringtable_index_t stringtable_record_impl(stringtable_t *tab, uint64_t N, bool extra_nul) {
  // Called to deal with the N bytes allocated at the edge of the arena
  // On success, return an index into the arena corresponding to those bytes
  // On failure, needs to drop those N bytes from the arena
  // When the key is already present, also needs to drop those N bytes
  const stringtable_index_t failure = {
      .value = UINT64_MAX,
  };

  size_t number_bytes_to_discard = N;

  {
    // Shuffle the bytes down by 8 and write the size into that space
    if (!arena_request_available(arena_mod, &tab->arena, 8 + (extra_nul ? 1 : 0))) {
      arena_discard_last_allocated(arena_mod, &tab->arena,
                                   number_bytes_to_discard);
      return failure;
    }
    arena_allocate_into_existing_capacity(arena_mod, &tab->arena, 8);
    number_bytes_to_discard += 8;
    char *str = (char *)arena_next_address(arena_mod, tab->arena) - N - 8;
    __builtin_memmove(str + 8, str, N);
    __builtin_memcpy(str, &N, 8);
  }

  // Level of arena excluding the N bytes just appended, i.e.
  // relative pointer to the string of interest
  uint64_t arena_level = arena_next_offset(arena_mod, tab->arena) - N;

  // Absolute pointer to string of interest
  char *str = (char *)arena_next_address(arena_mod, tab->arena) - N;

  // Make current arena base available after allocating and before calling into
  // hashtable

  if (hashtable_available(hashtable_mod, stringtable_to_userdata(tab),
                          tab->hash) < 2) {
    if (!hashtable_rehash_double(hashtable_mod, stringtable_to_userdata(tab),
                                 &tab->hash)) {
      arena_discard_last_allocated(arena_mod, &tab->arena,
                                   number_bytes_to_discard);
      return failure;
    }
  }

  // Check for existing value and deallocate str if present
  {
    unsigned char *r =
        hashtable_lookup_key(hashtable_mod, stringtable_to_userdata(tab),
                             tab->hash, (unsigned char *)&arena_level);
    if (r) {
      uint64_t v = word_from_bytes(r);
      const stringtable_index_t success = {
          .value = v,
      };
      assert(__builtin_memcmp(str, stringtable_lookup(tab, success), N) == 0);
      assert(stringtable_lookup_size(tab, success) == N);
      arena_discard_last_allocated(arena_mod, &tab->arena,
                                   number_bytes_to_discard);

      return success;
    }
  }

  // Record the current one

  if (extra_nul)
    {
      // Ensured available above but didn't allocate it
      bool r = arena_append_byte(arena_mod, &tab->arena, 0);
      assert(r);
      (void)r;
    }
  
  hashtable_insert(hashtable_mod, stringtable_to_userdata(tab), &tab->hash,
                   (unsigned char *)&arena_level, 0);

  const stringtable_index_t success = {
      .value = arena_level,
  };
  assert(__builtin_memcmp(str, stringtable_lookup(tab, success), N) == 0);
  assert(stringtable_lookup_size(tab, success) == N);
  return success;
}

stringtable_index_t stringtable_record(stringtable_t *tab, uint64_t N) {
  return stringtable_record_impl(tab, N, false);
}

stringtable_index_t stringtable_record_with_trailing_nul(stringtable_t *tab, uint64_t N) {
  return stringtable_record_impl(tab, N, true);
}


// only fails on out of memory
static stringtable_index_t stringtable_insert_impl(stringtable_t *tab, const char *str,
                                            size_t N, bool extra_nul) {
  const stringtable_index_t failure = {
      .value = UINT64_MAX,
  };

  if (!arena_append_bytes(arena_mod, &tab->arena, (const unsigned char *)str,
                          N)) {
    return failure;
  }

  return stringtable_record_impl(tab, N, extra_nul);
}

stringtable_index_t stringtable_insert(stringtable_t *tab, const char *str,
                                            size_t N) {
  return stringtable_insert_impl(tab,str,N,false);
}
stringtable_index_t stringtable_insert_with_trailing_nul(stringtable_t *tab, const char *str,
                                            size_t N) {
  return stringtable_insert_impl(tab,str,N,true);
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
    CHECK(idx.value == 8);

    const char *res = stringtable_lookup(&tab, idx);
    CHECK(stringtable_lookup_size(&tab, idx) == N+1);
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
      CHECK(idx.value == 8);
      const char *res = stringtable_lookup(&tab, idx);
      CHECK(stringtable_lookup_size(&tab, idx) == N+1);
      CHECK(__builtin_memcmp(str, res, N + 1) == 0);
    }

    {
      stringtable_index_t idx = stringtable_insert(&tab, str, N + 1);
      CHECK(idx.value != UINT64_MAX);
      CHECK(idx.value == 8);
      const char *res = stringtable_lookup(&tab, idx);
      CHECK(stringtable_lookup_size(&tab, idx) == N+1);
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
        8 + 0,
        16 + 7,
        24 + 13,
        32 + 21,
    };
    size_t N = 4;

    for (size_t rep = 0; rep < 40; rep++) {
      for (size_t i = 0; i < N; i++) {
        size_t len = __builtin_strlen(strs[i]) + 1;
        stringtable_index_t idx = stringtable_insert(&tab, strs[i], len);
        CHECK(idx.value != UINT64_MAX);
        CHECK(idx.value == offsets[i]);
        const char *res = stringtable_lookup(&tab, idx);
        CHECK(stringtable_lookup_size(&tab, idx) == len);
        CHECK(__builtin_memcmp(strs[i], res, len) == 0);
      }
    }

    stringtable_destroy(tab);
  }
}
