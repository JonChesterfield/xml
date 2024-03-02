#include "regex_cache.h"
#include <assert.h>

#include "regex.h"
#include "regex.ptree.h"
#include "regex_string.h"

#include "../tools/stack.libc.h"

// temporarily non-static from cache
extern const hashtable_module hashtable_mod;

bool regex_driver_insert(regex_cache_t *driver, const char *bytes, size_t N) {
  printf("Driver insert\n");


  stringtable_index_t first = regex_cache_insert_regex_bytes(driver, bytes, N);
  if (!stringtable_index_valid(first)) {
    return false;
  }

  void *stack = stack_create(&stack_libc, 8);
  if (!stack) {
    return false;
  }
  stack_push_assuming_capacity(&stack_libc, stack, first.value);

  printf("Driver insert main loop\n");
  // uint64_t counter = 0;
  while (stack_size(&stack_libc, stack) != 0) {
    // printf("Driver insert iteration %lu\n", counter++);

    stringtable_index_t active = {
        .value = stack_pop(&stack_libc, stack),
    };

    if (regex_cache_all_derivatives_computed(driver,active)) {
      continue;
    }

    // Some were unknown, calculate them all
    if (!regex_cache_calculate_all_derivatives(driver, active)) {
      return false;
    }

    {
      void *r = stack_reserve(&stack_libc, stack,
                              stack_size(&stack_libc, stack) + 256);
      if (!r) {
        return false;
      }
      stack = r;
    }      

    for (unsigned i = 0; i < 256; i++)
      {
        stringtable_index_t ith = regex_cache_lookup_derivative(driver, active, (uint8_t)i);
        if (ith.value == UINT64_MAX) {
          // Failed to calculate ith derivative
          return false;
        }

        stack_push_assuming_capacity(&stack_libc, stack, ith.value);        
      }

  }

  stack_libc_destroy(stack);

  return true;
}

bool regex_driver_regex_to_c(regex_cache_t *driver, stringtable_index_t index) {
  stringtable_index_t derivatives[256];

  const char *regex = stringtable_lookup(&driver->strtab, index);
  if (!regex) {
    return false;
  }

  printf("// regex %s\n", regex);

  unsigned char *values =
      hashtable_lookup_value(hashtable_mod, driver->regex_to_derivatives,
                             (unsigned char *)&index.value);
  assert(values);

  __builtin_memcpy(&derivatives[0], values, 256 * 8);
  printf("{\n");

  for (unsigned i = 0; i < 256; i++) {
    bool emit =
        (i == 255) || (derivatives[i].value != derivatives[i + 1].value);
    const char *regex = stringtable_lookup(&driver->strtab, derivatives[i]);
    if (emit) {
      printf("  case %u: return %s;\n", i, regex);
    } else {
      printf("  case %u:\n", i);
    }
  }
  printf("}\n");

  return true;
}
