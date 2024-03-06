#include "regex_cache.h"
#include "../tools/arena.libc.h"
#include "../tools/contract.h"
#include "regex.h"

#include "../tools/stack.libc.h"
#include "../tools/intset.h"
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
  bytes_value_size = 8 * 256,
  element_size = bytes_key_size + bytes_value_size,
};

static inline hashtable_t hashderiv_create(uint64_t size) {
  arena_t a = arena_create(arena_mod, element_size * size);
  unsigned char *p = arena_base_address(arena_mod, a);

  for (uint64_t i = 0; i < (size * element_size) / 8; i++) {
    // Keys placed at the start, but going to write -0 across the whole thing
    unsigned char *c = p + 8 * i;
    __builtin_memcpy(c, &hashderiv_sentinel, bytes_key_size);
  }

  return hashderiv_arena_to_hash(a);
}

static inline void hashderiv_destroy(hashtable_t h) {
  arena_destroy(arena_mod, hashderiv_hash_to_arena(h));
}

static inline bool hashderiv_valid(hashtable_t h) {
  return arena_valid(arena_mod, hashderiv_hash_to_arena(h));
}

static inline void hashderiv_store_userdata(hashtable_t *h, uint64_t v) {
  h->state[3] = v;
}

static inline uint64_t hashderiv_load_userdata(hashtable_t *h) {
  return h->state[3];
}

static uint64_t hashderiv_key_hash(hashtable_t h, unsigned char *bytes) {
  (void)h;
  // identity at present
  uint64_t r;
  __builtin_memcpy(&r, bytes, 8);
  return r;
}

static bool hashderiv_key_equal(hashtable_t h, const unsigned char *left,
                                const unsigned char *right) {
  (void)h;
  return __builtin_memcmp(left, right, 8) == 0;
}

// Keys and values are the same length, store size in the arena metadata
static uint64_t hashderiv_size(hashtable_t h) {
  arena_t a = hashderiv_hash_to_arena(h);
  uint64_t allocation_edge = (char *)arena_next_address(arena_mod, a) -
                             (char *)arena_base_address(arena_mod, a);
  return allocation_edge / 8;
}

static void hashderiv_assign_size(hashtable_t *h, uint64_t s) {
  arena_t a = hashderiv_hash_to_arena(*h);
  arena_change_allocation(arena_mod, &a, s * 8);
  *h = hashderiv_arena_to_hash(a);
}

static uint64_t hashderiv_capacity(hashtable_t h) {
  arena_t a = hashderiv_hash_to_arena(h);
  return arena_capacity(arena_mod, a) / element_size;
}

static uint64_t hashderiv_available(hashtable_t h) {
  return hashderiv_capacity(h) - hashderiv_size(h);
}

static unsigned char *hashderiv_location_key(hashtable_t h, uint64_t offset) {
  arena_t a = hashderiv_hash_to_arena(h);
  unsigned char *p = arena_base_address(arena_mod, a);
  return p + offset * bytes_key_size;
}

static unsigned char *hashderiv_location_value(hashtable_t h, uint64_t offset) {
  arena_t a = hashderiv_hash_to_arena(h);
  unsigned char *p = arena_base_address(arena_mod, a);
  uint64_t capacity = hashderiv_capacity(h);

  // Step over the keys
  p += capacity * bytes_key_size;

  return p + offset * bytes_value_size;
}

static uint64_t hashderiv_lookup_offset(hashtable_t h, unsigned char *key) {
  uint64_t hash = hashderiv_key_hash(h, key);
  uint64_t cap = hashderiv_capacity(h);

#if CACHE_CONTRACTS
  contract_unit_test(contract_is_power_of_two(cap), "cap offset", 10);
#endif

  const uint64_t mask = cap - 1;
  const uint64_t start_index = hash;

  for (uint64_t c = 0; c < cap; c++) {
    uint64_t index = (start_index + c) & mask;

    unsigned char *loc_key = hashderiv_location_key(h, index);

    if (hashderiv_key_equal(h, loc_key, key)) {
      /* Found key */
      return index;
    }

    if (hashderiv_key_equal(h, loc_key, (unsigned char *)&hashderiv_sentinel)) {
      /* Found a space */
      return index;
    }
  }

#if CACHE_CONTRACTS
  contract_unit_test(hashderiv_available(h) == 0, "avail 0", 7);
#endif
  return UINT64_MAX;
}

static const struct hashtable_module_ty hashtable_mod_state = {
    .create = hashderiv_create,
    .destroy = hashderiv_destroy,
    .valid = hashderiv_valid,
    .store_userdata = hashderiv_store_userdata,
    .load_userdata = hashderiv_load_userdata,
    .key_align = 8,
    .key_size = bytes_key_size,
    .value_align = 8,
    .value_size = bytes_value_size,
    .key_hash = hashderiv_key_hash,
    .key_equal = hashderiv_key_equal,
    .sentinel = (const unsigned char *)&hashderiv_sentinel,
    .size = hashderiv_size,
    .capacity = hashderiv_capacity,
    .lookup_offset = hashderiv_lookup_offset,
    .location_key = hashderiv_location_key,
    .location_value = hashderiv_location_value,
    .assign_size = hashderiv_assign_size,
    .maybe_remove = 0,
#if INTSET_CONTRACTS
    .maybe_contract = contract_unit_test,
#else
    .maybe_contract = 0,
#endif
};

const hashtable_module hashtable_mod = &hashtable_mod_state;

regex_cache_t regex_cache_create(void) {
  regex_cache_t d;
  d.regex_to_derivatives = hashtable_create(hashtable_mod, 16);
  d.strtab = stringtable_create();
  return d;
}

void regex_cache_destroy(regex_cache_t d) {
  hashtable_destroy(hashtable_mod, d.regex_to_derivatives);
  stringtable_destroy(d.strtab);
}

bool regex_cache_valid(regex_cache_t d) {
  return hashtable_valid(hashtable_mod, d.regex_to_derivatives) &&
         stringtable_valid(d.strtab);
}

static stringtable_index_t strfail(void) {
  return (stringtable_index_t){
      .value = UINT64_MAX,
  };
}

// Fails if the regex is not canonical
stringtable_index_t
regex_cache_insert_regex_canonical_ptree(regex_cache_t *driver, ptree regex) {
  if (ptree_is_failure(regex) ||
      !regex_is_canonical(regex)) {
    return strfail();
  }

  return regex_insert_into_stringtable(&driver->strtab, regex);
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

stringtable_index_t
regex_cache_insert_regex_bytes(regex_cache_t *c, const char *bytes, size_t N) {
  ptree_context ptree_ctx = regex_ptree_create_context();

  ptree p = regex_from_char_sequence(ptree_ctx, bytes, N);
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
    regex_cache_t *driver, stringtable_index_t index, uint8_t ith,
    unsigned char *values) {

  // Invalidated by insert_regex at the end of this function so recompute here
  const char *const regex = stringtable_lookup(&driver->strtab, index);
  if (!regex) {
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

  size_t N = __builtin_strlen(regex);
  ptree tree = regex_from_char_sequence(ptree_ctx, regex, N);

  if (ptree_is_failure(tree)) {
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
  if (hashtable_available(hashtable_mod, driver->regex_to_derivatives) < 8) {
    if (!hashtable_rehash_double(hashtable_mod,
                                 &driver->regex_to_derivatives)) {
      return 0;
    }
  }

  hashtable_insert(hashtable_mod, &driver->regex_to_derivatives,
                   (unsigned char *)&index.value,
                   (unsigned char *)&empty_derivatives[0]);

  return hashtable_lookup_value(hashtable_mod, driver->regex_to_derivatives,
                                (unsigned char *)&index.value);
}

// Compute ith derivative, record it in the cache
stringtable_index_t regex_cache_calculate_derivative(regex_cache_t *driver,
                                                     stringtable_index_t index,
                                                     uint8_t ith) {

  const char *const regex = stringtable_lookup(&driver->strtab, index);
  if (!regex) {
    return strfail();
  }

  unsigned char *values =
      hashtable_lookup_value(hashtable_mod, driver->regex_to_derivatives,
                             (unsigned char *)&index.value);

  if (!values) {
    values = insert_missing_empty_table_entry(driver, index);
    if (!values) {
      return strfail();
    }
  }

  return regex_cache_calculate_derivative_given_hashtable_values(driver, index,
                                                                 ith, values);
}

static stringtable_index_t regex_cache_lookup_derivative_given_hashtable_values(regex_cache_t *driver,
                                                                         stringtable_index_t index,
                                                                         uint8_t ith,
                                                                         unsigned char * values)
{
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

  unsigned char *values =
      hashtable_lookup_value(hashtable_mod, driver->regex_to_derivatives,
                             (unsigned char *)&index.value);

  if (!values) {
    return strfail();
  }

  return regex_cache_lookup_derivative_given_hashtable_values(driver, index, ith, values);
}

bool regex_cache_all_derivatives_computed(regex_cache_t *driver,
                                          stringtable_index_t index) {

  if (!stringtable_contains(&driver->strtab, index)) {
    return false;
  }

  unsigned char *values =
      hashtable_lookup_value(hashtable_mod, driver->regex_to_derivatives,
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

bool regex_cache_calculate_all_derivatives(regex_cache_t *driver,
                                           stringtable_index_t index) {
  const char *const regex = stringtable_lookup(&driver->strtab, index);
  if (!regex) {
    return false;
  }

  unsigned char *values =
      hashtable_lookup_value(hashtable_mod, driver->regex_to_derivatives,
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
            driver, index, (uint8_t)i, values);
    if (ith.value == UINT64_MAX) {
      return false;
    }
  }
  return true;
}

int regex_cache_traverse(regex_cache_t * cache,
                         stringtable_index_t root,
                         int (*functor)(regex_cache_t *,
                                        stringtable_index_t,
                                        void*),
                         void* data)
{
  assert(stringtable_index_valid(root));
  stack_module stackmod = &stack_libc;
  intset_t set = intset_create(512);
  if (!intset_valid(set)) { return -1; }

  void *stack = stack_create(stackmod, 8);
  if (!stack) { intset_destroy(set); return -1; }

  int retval = 0;

  stack_push_assuming_capacity(stackmod, stack, root.value);
  
  for (uint64_t current_stack_size = 1; current_stack_size = stack_size(stackmod, stack), current_stack_size != 0;) {
    stringtable_index_t active = {.value = stack_pop(stackmod, stack),};
    current_stack_size--; // keep the local variable accurate
    assert(active.value != UINT64_MAX);
    if (intset_contains(set, active.value))
      {
        continue;
      }

    // Call it on the root
    int f = functor(cache, active, data);
    if (f != 0) {
      retval = f; goto done;
    }
    intset_insert(&set, active.value);

    // Check the derivatives are available
    if (!regex_cache_calculate_all_derivatives(cache,
                                               active))
      {
        retval = -1; goto done;
      }

    // Check allocations are sufficient
    if (intset_available(set) < 256) {
      if (!intset_rehash_double(&set)) {
        retval = -1; goto done;
      }
    }
    
    {
      void *s2 = stack_reserve(stackmod, stack, current_stack_size + 256);
      if (!s2) {
        stack_destroy(stackmod, stack);
        retval = -1; goto done;
      }
      stack = s2;
    }

    // Push all derivatives onto the stack in reverse order
    {
      assert (stringtable_contains(&cache->strtab, active));
      unsigned char *values =
        hashtable_lookup_value(hashtable_mod, cache->regex_to_derivatives,
                               (unsigned char *)&active.value);
      assert(values); // from calculate_all above

      for (unsigned i = 256; i--> 0; )
        {        
          stringtable_index_t ith_deriv =
            regex_cache_lookup_derivative_given_hashtable_values(cache, active, (uint8_t)i, values);
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
