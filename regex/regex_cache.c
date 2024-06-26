#include "regex_cache.h"
#include "../tools/arena.libc.h"
#include "../tools/contract.h"
#include "regex.h"

#include "../tools/intset.h"
#include "../tools/stack.libc.h"
#include "regex_string.h"

#include <assert.h>

static const uint64_t hashderiv_sentinel = UINT64_MAX;

#define CACHE_CONTRACTS 1

static const struct arena_module_ty arena_libc_contract =
    ARENA_MODULE_INIT(arena_libc,
#if CACHE_CONTRACTS
                      contract_unit_test
#else
                      0
#endif
    );

static arena_module arena_mod = &arena_libc_contract;

static inline arena_t hashderiv_hash_to_arena(hashtable_t h) {
  arena_t a;
  a.base = h.state[0];
  a.next = h.state[1];
  a.limit = h.state[2];
  return a;
}

static inline hashtable_t hashderiv_arena_to_hash(arena_t a) {
  hashtable_t h;
  h.state[0] = a.base;
  h.state[1] = a.next;
  h.state[2] = a.limit;
  h.state[3] = 0;
  return h;
}

// Layout could be an array of keys followed by an array of value, or
// an array of {key, values}
// The former is faster here as values is larger than cache lines
// Storing size in the arena next/allocator position

enum {
  bytes_key_size = 8,
  bytes_value_size = 8 * 256 + 8,
  element_size = bytes_key_size + bytes_value_size,
};

static inline hashtable_t hashderiv_create(hashtable_user_t user,
                                           uint64_t size) {
  arena_t a = arena_create(arena_mod, element_size * size);
  unsigned char *p = arena_base_address(arena_mod, a);

  for (uint64_t i = 0; i < (size * element_size) / 8; i++) {
    // Keys placed at the start, but going to write -0 across the whole thing
    unsigned char *c = p + 8 * i;
    __builtin_memcpy(c, &hashderiv_sentinel, bytes_key_size);
  }

  return hashderiv_arena_to_hash(a);
}

static inline void hashderiv_destroy(hashtable_user_t user, hashtable_t h) {
  arena_destroy(arena_mod, hashderiv_hash_to_arena(h));
}

static inline bool hashderiv_valid(hashtable_user_t user, hashtable_t h) {
  return arena_valid(arena_mod, hashderiv_hash_to_arena(h));
}

static uint64_t hashderiv_key_hash(hashtable_user_t user,
                                   unsigned char *bytes) {
  (void)user;
  // identity at present
  uint64_t r;
  __builtin_memcpy(&r, bytes, 8);
  return r;
}

static bool hashderiv_key_equal(hashtable_user_t user,
                                const unsigned char *left,
                                const unsigned char *right) {
  (void)user;
  return __builtin_memcmp(left, right, bytes_key_size) == 0;
}

// Keys and values are the same length, store size in the arena metadata
static uint64_t hashderiv_size(hashtable_user_t user, hashtable_t h) {
  arena_t a = hashderiv_hash_to_arena(h);
  uint64_t allocation_edge = arena_next_offset(arena_mod, a);
  return allocation_edge / 8;
}

static void hashderiv_assign_size(hashtable_user_t user, hashtable_t *h,
                                  uint64_t s) {
  arena_t a = hashderiv_hash_to_arena(*h);
  arena_change_allocation(arena_mod, &a, s * 8);
  *h = hashderiv_arena_to_hash(a);
}

static uint64_t hashderiv_capacity(hashtable_user_t user, hashtable_t h) {
  arena_t a = hashderiv_hash_to_arena(h);
  return arena_capacity(arena_mod, a) / element_size;
}

static unsigned char *hashderiv_location_key(hashtable_user_t user,
                                             hashtable_t h, uint64_t offset) {
  arena_t a = hashderiv_hash_to_arena(h);
  unsigned char *p = arena_base_address(arena_mod, a);
  return p + offset * bytes_key_size;
}

static unsigned char *hashderiv_location_value(hashtable_user_t user,
                                               hashtable_t h, uint64_t offset) {
  arena_t a = hashderiv_hash_to_arena(h);
  unsigned char *p = arena_base_address(arena_mod, a);
  uint64_t capacity = hashderiv_capacity((hashtable_user_t){0}, h);

  // Step over the keys
  p += capacity * bytes_key_size;

  return p + offset * bytes_value_size;
}

static const struct hashtable_module_ty hashtable_mod_state = {
    .create = hashderiv_create,
    .destroy = hashderiv_destroy,
    .valid = hashderiv_valid,
    .key_align = 8,
    .key_size = bytes_key_size,
    .value_align = 8,
    .value_size = bytes_value_size,
    .key_hash = hashderiv_key_hash,
    .key_equal = hashderiv_key_equal,
    .sentinel = (const unsigned char *)&hashderiv_sentinel,
    .size = hashderiv_size,
    .capacity = hashderiv_capacity,
    .location_key = hashderiv_location_key,
    .location_value = hashderiv_location_value,
    .assign_size = hashderiv_assign_size,
#if INTSET_CONTRACTS
    .maybe_contract = contract_unit_test,
#else
    .maybe_contract = 0,
#endif
};

const hashtable_module hashtable_mod = &hashtable_mod_state;

regex_cache_t regex_cache_create(void) {
  regex_cache_t d;
  d.regex_to_derivatives =
      hashtable_create(hashtable_mod, (hashtable_user_t){0}, 16);
  d.regex_to_properties = intmap_create(16);
  d.strtab = stringtable_create();
  return d;
}

void regex_cache_destroy(regex_cache_t d) {
  hashtable_destroy(hashtable_mod, (hashtable_user_t){0},
                    d.regex_to_derivatives);
  intmap_destroy(d.regex_to_properties);
  stringtable_destroy(d.strtab);
}

bool regex_cache_valid(regex_cache_t d) {
  return hashtable_valid(hashtable_mod, (hashtable_user_t){0},
                         d.regex_to_derivatives) &&
         intmap_valid(d.regex_to_properties) && stringtable_valid(d.strtab);
}

static stringtable_index_t strfail(void) {
  return (stringtable_index_t){
      .value = UINT64_MAX,
  };
}

// Fails if the regex is not canonical
stringtable_index_t
regex_cache_insert_regex_canonical_ptree(regex_cache_t *driver, ptree regex) {
  if (ptree_is_failure(regex) || !regex_is_canonical(regex)) {
    return strfail();
  }

  if (intmap_available(driver->regex_to_properties) < 8) {
    if (!intmap_rehash_double(&driver->regex_to_properties)) {
      return strfail();
    }
  }

  // todo, enforce this statically somewhere? maybe have identifier be a smaller
  // type
  uint64_t id = regex_ptree_identifier(regex);
  if (id > UINT32_MAX) {
    return strfail();
  }

  stringtable_index_t res =
      regex_insert_into_stringtable(&driver->strtab, regex);

  if (stringtable_index_valid(res)) {
    uint64_t prop = 0;

    bool is_empty_string = regex_is_empty_string(regex);
    bool is_empty_set = regex_is_empty_set(regex);
    assert(!(is_empty_string && is_empty_set));

    if (is_empty_string)
      prop |= regex_cache_lookup_empty_string;

    if (is_empty_set)
      prop |= regex_cache_lookup_empty_set;

    if (prop == 0)
      prop |= regex_cache_lookup_composite;

    prop |= regex_nullable_p(regex) ? regex_cache_lookup_nullable : 0;

    id = id << 32u;

    prop |= id;

    intmap_insert(&driver->regex_to_properties, res.value, prop);
  }

  return res;
}

enum regex_cache_lookup_properties
regex_cache_lookup_properties(regex_cache_t *c, stringtable_index_t index) {
  uint64_t res = intmap_lookup(c->regex_to_properties, index.value);
  if (res == UINT64_MAX) {
    return regex_cache_lookup_failure;
  }
  return (enum regex_cache_lookup_properties)res;
}

stringtable_index_t regex_cache_insert_regex_ptree(regex_cache_t *c,
                                                   ptree_context ptree_ctx,
                                                   ptree regex) {
  regex = regex_canonicalise(ptree_ctx, regex);
  if (ptree_is_failure(regex)) {
    return strfail();
  }
  return regex_cache_insert_regex_canonical_ptree(c, regex);
}

stringtable_index_t regex_cache_insert_regex_bytes_using_lexer(
    lexer_t lexer,
    lexer_token_t (*lexer_iterator_step)(lexer_t, lexer_iterator_t *),
    regex_cache_t *c, const char *bytes, size_t N) {
  ptree_context ptree_ctx = regex_ptree_create_context();

  ptree p = regex_from_char_sequence_using_lexer(lexer, lexer_iterator_step,
                                                 ptree_ctx, bytes, N);
  if (ptree_is_failure(p)) {
    return strfail();
  }

  stringtable_index_t res = regex_cache_insert_regex_ptree(c, ptree_ctx, p);
  regex_ptree_destroy_context(ptree_ctx);
  return res;
}

#define E() {UINT64_MAX},
#define E4() E() E() E() E()
#define E16() E4() E4() E4() E4()
#define E64() E16() E16() E16() E16()
#define E256() E64() E64() E64() E64()

// memset might be better but the hashtable has copying semantics
static const stringtable_index_t empty_derivatives[256] = {E256()};

static stringtable_index_t
regex_cache_calculate_derivative_given_hashtable_values(
    lexer_t lexer,
    lexer_token_t (*lexer_iterator_step)(lexer_t, lexer_iterator_t *),
    regex_cache_t *driver, stringtable_index_t index, uint8_t ith,
    unsigned char *values) {

  // Invalidated by insert_regex at the end of this function so recompute here
  const char *const regex = stringtable_lookup(&driver->strtab, index);
  if (!regex) {
    // printf("can't calculate derivative of unknown string %lu\n",
    // index.value);
    return strfail();
  }

  unsigned char *this_value = values + 8 * ith;

  {
    stringtable_index_t res;
    __builtin_memcpy(&res, this_value, 8);
    if (res.value != UINT64_MAX) {
      // Cache hit
      return res;
    }
  }

  ptree_context ptree_ctx = regex_ptree_create_context();

  size_t N = stringtable_lookup_size(&driver->strtab, index);
  ptree tree = regex_from_char_sequence_using_lexer(lexer, lexer_iterator_step,
                                                    ptree_ctx, regex, N);

  if (ptree_is_failure(tree)) {
    // printf("failed to make a regex from %s, size %lu\n", regex, N);
    return strfail();
  }
  if (!regex_is_canonical(tree)) {
    fprintf(stdout, "Not canonical\n");
    regex_ptree_as_xml(&stack_libc, stdout, tree);
    fflush(stdout);
    return strfail();
  }

  stringtable_index_t loc = strfail();

  ptree canonical_derivative = regex_canonical_derivative(ptree_ctx, tree, ith);
  if (!ptree_is_failure(canonical_derivative)) {
    loc =
        regex_cache_insert_regex_canonical_ptree(driver, canonical_derivative);

    // Write it into the hashtable. If it failed, this replaces failed with
    // failed.
    __builtin_memcpy(this_value, &loc.value, 8);
  }

  regex_ptree_destroy_context(ptree_ctx);
  return loc;
}

static unsigned char *
insert_missing_empty_table_entry(regex_cache_t *driver,
                                 stringtable_index_t index) {
  const hashtable_user_t user = {0};
  if (hashtable_available(hashtable_mod, user, driver->regex_to_derivatives) <
      8) {
    if (!hashtable_rehash_double(hashtable_mod, user,
                                 &driver->regex_to_derivatives)) {
      return 0;
    }
  }

  hashtable_insert(hashtable_mod, user, &driver->regex_to_derivatives,
                   (unsigned char *)&index.value,
                   (unsigned char *)&empty_derivatives[0]);

  return hashtable_lookup_value(hashtable_mod, user,
                                driver->regex_to_derivatives,
                                (unsigned char *)&index.value);
}

// Compute ith derivative, record it in the cache
stringtable_index_t regex_cache_calculate_derivative_using_lexer(
    lexer_t lexer,
    lexer_token_t (*lexer_iterator_step)(lexer_t, lexer_iterator_t *),
    regex_cache_t *driver, stringtable_index_t index, uint8_t ith) {

  const char *const regex = stringtable_lookup(&driver->strtab, index);
  if (!regex) {
    return strfail();
  }

  unsigned char *values = hashtable_lookup_value(
      hashtable_mod, (hashtable_user_t){0}, driver->regex_to_derivatives,
      (unsigned char *)&index.value);

  if (!values) {
    values = insert_missing_empty_table_entry(driver, index);
    if (!values) {
      return strfail();
    }
  }

  return regex_cache_calculate_derivative_given_hashtable_values(
      lexer, lexer_iterator_step, driver, index, ith, values);
}

static stringtable_index_t regex_cache_lookup_derivative_given_hashtable_values(
    regex_cache_t *driver, stringtable_index_t index, uint8_t ith,
    unsigned char *values) {
  assert(values != 0);
  unsigned char *this_value = values + 8 * ith;
  {
    stringtable_index_t res;
    __builtin_memcpy(&res, this_value, 8);
    if (res.value != UINT64_MAX) {
      // Cache hit
      return res;
    }
  }

  return strfail();
}

stringtable_index_t regex_cache_lookup_derivative(regex_cache_t *driver,
                                                  stringtable_index_t index,
                                                  uint8_t ith)

{
  if (!stringtable_contains(&driver->strtab, index)) {
    return strfail();
  }

  unsigned char *values = hashtable_lookup_value(
      hashtable_mod, (hashtable_user_t){0}, driver->regex_to_derivatives,
      (unsigned char *)&index.value);

  if (!values) {
    return strfail();
  }

  return regex_cache_lookup_derivative_given_hashtable_values(driver, index,
                                                              ith, values);
}

bool regex_cache_all_derivatives_computed(regex_cache_t *driver,
                                          stringtable_index_t index) {

  if (!stringtable_contains(&driver->strtab, index)) {
    return false;
  }

  unsigned char *values = hashtable_lookup_value(
      hashtable_mod, (hashtable_user_t){0}, driver->regex_to_derivatives,
      (unsigned char *)&index.value);

  if (!values) {
    return false;
  }

  bool all_computed = true;
  for (unsigned i = 0; i < 256; i++) {
    uint8_t ith = (uint8_t)i;

    unsigned char *this_value = values + 8 * ith;
    {
      stringtable_index_t res;
      __builtin_memcpy(&res, this_value, 8);
      if (res.value == UINT64_MAX) {
        all_computed = false;
      }
    }
  }

  return all_computed;
}

bool regex_cache_calculate_all_derivatives_using_lexer(
    lexer_t lexer,
    lexer_token_t (*lexer_iterator_step)(lexer_t, lexer_iterator_t *),
    regex_cache_t *driver, stringtable_index_t index) {
  const char *const regex = stringtable_lookup(&driver->strtab, index);
  if (!regex) {
    return false;
  }

  unsigned char *values = hashtable_lookup_value(
      hashtable_mod, (hashtable_user_t){0}, driver->regex_to_derivatives,
      (unsigned char *)&index.value);

  if (!values) {
    values = insert_missing_empty_table_entry(driver, index);
    if (!values) {
      return false;
    }
  }

  for (unsigned i = 0; i < 256; i++) {
    stringtable_index_t ith =
        regex_cache_calculate_derivative_given_hashtable_values(
            lexer, lexer_iterator_step, driver, index, (uint8_t)i, values);
    if (ith.value == UINT64_MAX) {
      return false;
    }
  }
  return true;
}

int regex_cache_traverse_using_lexer(
    lexer_t lexer,
    lexer_token_t (*lexer_iterator_step)(lexer_t, lexer_iterator_t *),
    regex_cache_t *cache, stringtable_index_t root,
    int (*functor)(regex_cache_t *, stringtable_index_t, void *), void *data) {
  assert(stringtable_index_valid(root));
  stack_module stackmod = &stack_libc;
  intset_t set = intset_create(512);
  if (!intset_valid(set)) {
    return -1;
  }

  void *stack = stack_create(stackmod, 8);
  if (!stack) {
    intset_destroy(set);
    return -1;
  }

  int retval = 0;

  stack_push_assuming_capacity(stackmod, stack, root.value);

  for (uint64_t current_stack_size = 1;
       current_stack_size = stack_size(stackmod, stack),
                current_stack_size != 0;) {
    stringtable_index_t active = {
        .value = stack_pop(stackmod, stack),
    };
    current_stack_size--; // keep the local variable accurate
    assert(active.value != UINT64_MAX);
    if (intset_contains(set, active.value)) {
      continue;
    }

    // Call it on the root
    int f = functor(cache, active, data);
    if (f != 0) {
      retval = f;
      goto done;
    }
    intset_insert(&set, active.value);

    // Check the derivatives are available
    if (!regex_cache_calculate_all_derivatives_using_lexer(
            lexer, lexer_iterator_step, cache, active)) {
      retval = -1;
      goto done;
    }

    // Check allocations are sufficient
    if (intset_available(set) < 256) {
      if (!intset_rehash_double(&set)) {
        retval = -1;
        goto done;
      }
    }

    {
      void *s2 = stack_reserve(stackmod, stack, current_stack_size + 256);
      if (!s2) {
        stack_destroy(stackmod, stack);
        retval = -1;
        goto done;
      }
      stack = s2;
    }

    // Push all derivatives onto the stack in reverse order
    {
      assert(stringtable_contains(&cache->strtab, active));
      unsigned char *values = hashtable_lookup_value(
          hashtable_mod, (hashtable_user_t){0}, cache->regex_to_derivatives,
          (unsigned char *)&active.value);
      assert(values); // from calculate_all above

      for (unsigned i = 256; i-- > 0;) {
        stringtable_index_t ith_deriv =
            regex_cache_lookup_derivative_given_hashtable_values(
                cache, active, (uint8_t)i, values);
        assert(stringtable_index_valid(ith_deriv));
        if (!intset_contains(set, ith_deriv.value)) {
          stack_push_assuming_capacity(stackmod, stack, ith_deriv.value);
        }
      }
    }
  }

done:;

  stack_destroy(stackmod, stack);
  intset_destroy(set);
  return retval;
}

// Functions that create the default lexer then call through to the
// implementation

stringtable_index_t
regex_cache_insert_regex_bytes(regex_cache_t *c, const char *bytes, size_t N) {
  lexer_t lexer = regex_lexer_create();
  if (!regex_lexer_valid(lexer)) {
    return strfail();
  }

  stringtable_index_t res = regex_cache_insert_regex_bytes_using_lexer(
      lexer, regex_lexer_iterator_step, c, bytes, N);

  regex_lexer_destroy(lexer);
  return res;
}

stringtable_index_t regex_cache_calculate_derivative(regex_cache_t *driver,
                                                     stringtable_index_t index,
                                                     uint8_t ith) {
  lexer_t lexer = regex_lexer_create();
  if (!regex_lexer_valid(lexer)) {
    return strfail();
  }

  stringtable_index_t res = regex_cache_calculate_derivative_using_lexer(
      lexer, regex_lexer_iterator_step, driver, index, ith);

  regex_lexer_destroy(lexer);
  return res;
}

bool regex_cache_calculate_all_derivatives(regex_cache_t *driver,
                                           stringtable_index_t index) {
  lexer_t lexer = regex_lexer_create();
  if (!regex_lexer_valid(lexer)) {
    return false;
  }

  bool res = regex_cache_calculate_all_derivatives_using_lexer(
      lexer, regex_lexer_iterator_step, driver, index);

  regex_lexer_destroy(lexer);
  return res;
}

int regex_cache_traverse(regex_cache_t *cache, stringtable_index_t root,
                         int (*functor)(regex_cache_t *, stringtable_index_t,
                                        void *),
                         void *data) {

  lexer_t lexer = regex_lexer_create();
  if (!regex_lexer_valid(lexer)) {
    return false;
  }

  int res = regex_cache_traverse_using_lexer(lexer, regex_lexer_iterator_step,
                                             cache, root, functor, data);

  regex_lexer_destroy(lexer);
  return res;
}
