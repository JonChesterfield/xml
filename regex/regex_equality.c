#include "regex_equality.h"
#include "../tools/intmap.h"
#include "../tools/stack.libc.h"
#include "regex_string.h"

#include <assert.h>

static bool regex_id_is_atomic(uint64_t id) {
  if (regex_grouping_id_is_single_byte(id)) {
    return true;
  }

  switch (id) {
  case regex_grouping_empty_set:
  case regex_grouping_empty_string:
    return true;
  default:
    return false;
  }
}

bool regex_ptree_is_atomic(ptree val) {
  uint64_t id = regex_ptree_identifier(val);
  return regex_id_is_atomic(id);
}

bool regex_ptree_atomic_and_equal(ptree x, ptree y) {
  uint64_t xid = regex_ptree_identifier(x);
  uint64_t yid = regex_ptree_identifier(y);
  return (xid == yid) && regex_id_is_atomic(xid);
}

bool regex_ptree_definitionally_equal(ptree x, ptree y) {
  stringtable_t strtab = stringtable_create();
  bool xfail = ptree_is_failure(x);
  bool yfail = ptree_is_failure(y);
  if (xfail && yfail) {
    return true;
  }
  if (xfail || yfail) {
    return false;
  }
  stringtable_index_t xs = regex_insert_into_stringtable(&strtab, x);
  stringtable_index_t ys = regex_insert_into_stringtable(&strtab, y);
  bool r = regex_canonical_definitionally_equal(&strtab, xs, ys);
  stringtable_destroy(strtab);
  return r;
}

bool regex_ptree_similar(ptree x, ptree y) {
  stringtable_t strtab = stringtable_create();
  stringtable_index_t xs = regex_insert_into_stringtable(&strtab, x);
  stringtable_index_t ys = regex_insert_into_stringtable(&strtab, y);
  bool r = regex_canonical_similar(&strtab, xs, ys);
  stringtable_destroy(strtab);
  return r;
}

bool regex_ptree_equivalent(regex_cache_t *cache, ptree x, ptree y) {
  stringtable_index_t xs = regex_insert_into_stringtable(&cache->strtab, x);
  stringtable_index_t ys = regex_insert_into_stringtable(&cache->strtab, y);
  bool r = regex_canonical_equivalent(cache, xs, ys);
  return r;
}

bool regex_canonical_is_atomic(regex_cache_t *cache, stringtable_index_t x) {
  enum regex_cache_lookup_properties props =
    regex_cache_lookup_properties(cache, x);
  uint64_t xid = regex_properties_retrieve_root_identifier(props);
  return regex_id_is_atomic(xid);
}

bool regex_canonical_atomic_and_equal(regex_cache_t *cache, stringtable_index_t x,
                                      stringtable_index_t y) {

  enum regex_cache_lookup_properties xprops =
    regex_cache_lookup_properties(cache, x);
  enum regex_cache_lookup_properties yprops =
    regex_cache_lookup_properties(cache, y);
  uint64_t xid = regex_properties_retrieve_root_identifier(xprops);
  uint64_t yid = regex_properties_retrieve_root_identifier(yprops);
  return (xid == yid) && regex_id_is_atomic(xid);
}

bool regex_canonical_definitionally_equal(stringtable_t *tab,
                                          stringtable_index_t x,
                                          stringtable_index_t y) {
  (void)tab;
  return stringtable_index_valid(x) && (x.value == y.value);
}

bool regex_canonical_similar(stringtable_t *tab, stringtable_index_t x,
                             stringtable_index_t y) {
  ptree_context ctx = regex_ptree_create_context();
  assert(regex_ptree_valid_context(ctx));
  ptree px = regex_from_stringtable(tab, x, ctx);
  ptree py = regex_from_stringtable(tab, y, ctx);
  ptree cx = regex_canonicalise(ctx, px);
  ptree cy = regex_canonicalise(ctx, py);
  bool r = regex_ptree_definitionally_equal(cx, cy);
  regex_ptree_destroy_context(ctx);
  return r;
}

static void put_smaller_on_left(stringtable_index_t *x,
                                stringtable_index_t *y) {
  if (y->value < x->value) {
    uint64_t tmp = x->value;
    x->value = y->value;
    y->value = tmp;
  }
}

static bool regex_canonical_equivalent_with_structures(regex_cache_t *cache,
                                                       stringtable_index_t x,
                                                       stringtable_index_t y,
                                                       void **stack_arg,
                                                       intmap_t *map_arg) {

  // stringtable does not contain ptree_failure nodes
  
  void *stack = *stack_arg;
  intmap_t map = *map_arg;

  put_smaller_on_left(&x, &y);
  stack_push_assuming_capacity(&stack_libc, stack, y.value);
  stack_push_assuming_capacity(&stack_libc, stack, x.value);

  for (uint64_t size = 2; size = stack_size(&stack_libc, stack), size != 0;) {
    stringtable_index_t left = {.value = stack_pop(&stack_libc, stack)};
    stringtable_index_t right = {.value = stack_pop(&stack_libc, stack)};
    size = size - 2;

    assert(left.value < right.value);

    enum regex_cache_lookup_properties left_props =
      regex_cache_lookup_properties(cache, left);
    enum regex_cache_lookup_properties right_props =
      regex_cache_lookup_properties(cache, right);

    
    {
      uint64_t xid = regex_properties_retrieve_root_identifier(left_props);
      uint64_t yid = regex_properties_retrieve_root_identifier(right_props);
      
      if (regex_id_is_atomic(xid) && regex_id_is_atomic(yid)) {
        if (xid == yid) {
          continue;
        }
        if (xid != yid) {
          return false;
        }
      }

      bool left_nullable_is_empty_string = regex_properties_is_nullable(left_props);
      bool right_nullable_is_empty_string = regex_properties_is_nullable(right_props);

      if (left_nullable_is_empty_string != right_nullable_is_empty_string)
        {
          return false;
        }
      
    }

    if (intmap_lookup(map, left.value) == right.value) {
      // Already shown this pair to be equivalent
      continue;
    }

    if (!regex_cache_calculate_all_derivatives(cache, left) ||
        !regex_cache_calculate_all_derivatives(cache, right)) {
      return false;
    }

    if (intmap_available(map) < 4) {
      if (!intmap_rehash_double(&map)) {
        return false;
      }
      *map_arg = map;
    }

    {
      void *r = stack_reserve(&stack_libc, stack,
                              stack_size(&stack_libc, stack) + 256);
      if (!r) {
        return false;
      }
      stack = r;
      *stack_arg = stack;
    }

    intmap_insert(&map, left.value, right.value);

    for (unsigned i = 0; i < 256; i++) {
      stringtable_index_t left_deriv =
          regex_cache_lookup_derivative(cache, left, (uint8_t)i);
      stringtable_index_t right_deriv =
          regex_cache_lookup_derivative(cache, right, (uint8_t)i);

      if (left_deriv.value != right_deriv.value) {
        put_smaller_on_left(&left_deriv, &right_deriv);
        stack_push_assuming_capacity(&stack_libc, stack, right_deriv.value);
        stack_push_assuming_capacity(&stack_libc, stack, left_deriv.value);
      }
    }
  }

  return true;
}

// returns "false" on out of memory, might want to change that
bool regex_canonical_equivalent(regex_cache_t *cache, stringtable_index_t x,
                                stringtable_index_t y) {

  if (x.value == y.value) {
    return true;
  }

  void *stack = stack_create(&stack_libc, 256);
  if (!stack) {
    return false;
  }

  intmap_t map = intmap_create(16);
  if (!intmap_valid(map)) {
    stack_destroy(&stack_libc, stack);
    return false;
  }

  bool res =
      regex_canonical_equivalent_with_structures(cache, x, y, &stack, &map);
  stack_destroy(&stack_libc, stack);
  intmap_destroy(map);

  return res;
}

stringtable_index_t regex_canonical_distinguished(regex_cache_t *,
                                                  stringtable_index_t,
                                                  stringtable_index_t);
