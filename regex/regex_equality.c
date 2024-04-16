#include "regex_equality.h"
#include "../tools/intmap.h"
#include "../tools/intstack.h"
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
  case regex_grouping_any_char:
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

regex_compare_t regex_ptree_definitionally_equal(ptree x, ptree y) {

  // Alternative is to write the ptrees into strings and test the
  // strings for equality. Turning a ptree into a string is more
  // expensive than iterating over it. Compare requires non-failure input.

  if (ptree_is_failure(x) && ptree_is_failure(y)) {
    return regex_compare_equal;
  }
  if (ptree_is_failure(x) || ptree_is_failure(y)) {
    return regex_compare_not_equal;
  }

  enum ptree_compare_res res = regex_ptree_compare(&stack_libc, x, y);
  switch (res) {
  case ptree_compare_lesser:
  case ptree_compare_greater:
    return regex_compare_not_equal;
  case ptree_compare_equal:
    return regex_compare_equal;
  case ptree_compare_out_of_memory:
    return regex_compare_out_of_memory;
  }
}

regex_compare_t regex_ptree_similar(ptree x, ptree y) {
  regex_compare_t defn = regex_ptree_definitionally_equal(x, y);
  switch (defn) {
  case regex_compare_equal:
  case regex_compare_out_of_memory:
  case regex_compare_failure:
    return defn;
  case regex_compare_not_equal:
    break;
  }

  // Testing nullable is cheaper than canonicalising
  // (and possibly cheaper than the equality test)
  if (regex_nullable_p(x) != regex_nullable_p(y)) {
    return regex_compare_not_equal;
  }

  // Both nullable or both non-nullable, do the expensive canonicalise.

  ptree_context ctx = regex_ptree_create_context();
  if (!regex_ptree_valid_context(ctx)) {
    return regex_compare_out_of_memory;
  }

  ptree px = regex_copy_into_context(x, ctx);
  ptree py = regex_copy_into_context(y, ctx);
  if (ptree_is_failure(px) || ptree_is_failure(py)) {
    regex_ptree_destroy_context(ctx);
    return regex_compare_out_of_memory;
  }

  ptree cx = regex_canonicalise(ctx, px);
  ptree cy = regex_canonicalise(ctx, py);
  if (ptree_is_failure(cx) || ptree_is_failure(cy)) {
    // Either broken internally or out of memory
    regex_ptree_destroy_context(ctx);
    return regex_compare_out_of_memory;
  }

  regex_compare_t r = regex_ptree_definitionally_equal(cx, cy);
  regex_ptree_destroy_context(ctx);
  return r;
}

regex_compare_t regex_ptree_equivalent(regex_cache_t *cache, ptree x, ptree y) {
  stringtable_index_t xs = regex_insert_into_stringtable(&cache->strtab, x);
  stringtable_index_t ys = regex_insert_into_stringtable(&cache->strtab, y);
  if (stringtable_index_valid(xs) && stringtable_index_valid(ys)) {
    return regex_canonical_equivalent(cache, xs, ys);
  } else {
    return regex_compare_out_of_memory;
  }
}

bool regex_canonical_is_atomic(regex_cache_t *cache, stringtable_index_t x) {
  enum regex_cache_lookup_properties props =
      regex_cache_lookup_properties(cache, x);
  uint64_t xid = regex_properties_retrieve_root_identifier(props);
  return regex_id_is_atomic(xid);
}

bool regex_canonical_atomic_and_equal(regex_cache_t *cache,
                                      stringtable_index_t x,
                                      stringtable_index_t y) {

  enum regex_cache_lookup_properties xprops =
      regex_cache_lookup_properties(cache, x);
  enum regex_cache_lookup_properties yprops =
      regex_cache_lookup_properties(cache, y);
  uint64_t xid = regex_properties_retrieve_root_identifier(xprops);
  uint64_t yid = regex_properties_retrieve_root_identifier(yprops);
  return (xid == yid) && regex_id_is_atomic(xid);
}

bool regex_canonical_definitionally_equal(regex_cache_t *cache,
                                          stringtable_index_t x,
                                          stringtable_index_t y) {
  (void)cache;
  return stringtable_index_valid(x) && (x.value == y.value);
}

regex_compare_t regex_canonical_similar(regex_cache_t *cache,
                                        stringtable_index_t x,
                                        stringtable_index_t y) {
  if (regex_canonical_definitionally_equal(cache, x, y)) {
    // Cheapest test first
    return regex_compare_equal;
  }

  {
    // If one is nullable and the other is not, they aren't going to be similar
    enum regex_cache_lookup_properties left_props =
        regex_cache_lookup_properties(cache, x);
    enum regex_cache_lookup_properties right_props =
        regex_cache_lookup_properties(cache, y);

    if (regex_properties_is_nullable(left_props) !=
        regex_properties_is_nullable(right_props)) {
      return regex_compare_not_equal;
    }
  }

  ptree_context ctx = regex_ptree_create_context();
  if (!regex_ptree_valid_context(ctx)) {
    return regex_compare_out_of_memory;
  }

  ptree px = regex_from_stringtable(&cache->strtab, x, ctx);
  ptree py = regex_from_stringtable(&cache->strtab, y, ctx);
  if (ptree_is_failure(px) || ptree_is_failure(py)) {
    // It's either out of memory or the strings were missing or malformed
    regex_ptree_destroy_context(ctx);
    return regex_compare_failure;
  }

  regex_compare_t r = regex_ptree_similar(px, py);
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

static regex_compare_t regex_canonical_equivalent_with_structures(
    regex_cache_t *cache, stringtable_index_t x, stringtable_index_t y,
    intstack_t *stack_arg, intmap_t *map_arg) {

  // stringtable does not contain ptree_failure nodes
  intstack_t stack = *stack_arg;
  intmap_t map = *map_arg;

  put_smaller_on_left(&x, &y);
  intstack_push(&stack, y.value);
  intstack_push(&stack, x.value);

  for (uint64_t size = 2; size = intstack_size(stack), size != 0;) {
    stringtable_index_t left = {.value = intstack_pop(&stack)};
    stringtable_index_t right = {.value = intstack_pop(&stack)};
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
          return regex_compare_not_equal;
        }
      }

      if (regex_properties_is_nullable(left_props) !=
          regex_properties_is_nullable(right_props)) {
        return regex_compare_not_equal;
      }
    }

    if (intmap_lookup(map, left.value) == right.value) {
      // Already shown this pair to be equivalent
      continue;
    }

    if (!regex_cache_calculate_all_derivatives(cache, left) ||
        !regex_cache_calculate_all_derivatives(cache, right)) {
      return regex_compare_out_of_memory;
    }

    if (intmap_available(map) < 4) {
      if (!intmap_rehash_double(&map)) {
        return regex_compare_out_of_memory;
      }
      *map_arg = map;
    }

    {
      if (!intstack_reserve(&stack, intstack_size(stack) + 256)) {
        return regex_compare_out_of_memory;
      }
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
        intstack_push(&stack, right_deriv.value);
        intstack_push(&stack, left_deriv.value);
      }
    }
  }

  return regex_compare_equal;
}

regex_compare_t regex_canonical_equivalent(regex_cache_t *cache,
                                           stringtable_index_t x,
                                           stringtable_index_t y) {

  if (x.value == y.value) {
    return regex_compare_equal;
  }

  intstack_t stack = intstack_create(256);
  if (!intstack_valid(stack)) {
    return regex_compare_out_of_memory;
  }

  intmap_t map = intmap_create(16);
  if (!intmap_valid(map)) {
    intstack_destroy(stack);
    return regex_compare_out_of_memory;
  }

  regex_compare_t res =
      regex_canonical_equivalent_with_structures(cache, x, y, &stack, &map);
  intstack_destroy(stack);
  intmap_destroy(map);

  return res;
}

stringtable_index_t regex_canonical_distinguished(regex_cache_t *,
                                                  stringtable_index_t,
                                                  stringtable_index_t);
