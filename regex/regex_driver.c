#include "regex_cache.h"
#include <assert.h>

#include "regex.h"
#include "regex.ptree.h"
#include "regex_string.h"

#include "../tools/intset.h"
#include "../tools/stack.libc.h"

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

    if (regex_cache_all_derivatives_computed(driver, active)) {
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

    for (unsigned i = 0; i < 256; i++) {
      stringtable_index_t ith =
          regex_cache_lookup_derivative(driver, active, (uint8_t)i);
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

struct to_c_func {
  stringtable_index_t derivs[256];
  intset_t set;
  FILE *file;
};

static char *malloc_rewrite(const char *str, size_t N) {
  char *res = malloc(N + 1);
  if (res) {
    for (size_t i = 0; i < N; i++) {
      res[i] = regex_syntax_byte_to_c_identifier_byte(str[i]);
    }
    res[N] = '\0';
  }
  return res;
}

static int regex_to_c_traverse_function(regex_cache_t *cache,
                                        stringtable_index_t active,
                                        void *arg_data) {
  struct to_c_func *data = (struct to_c_func *)arg_data;

  FILE *out = data->file;

  const char *regex = stringtable_lookup(&cache->strtab, active);
  size_t regexN = __builtin_strlen(regex);

  if (!regex) {
    return 1;
  }

  intset_clear(&data->set);

  for (unsigned i = 0; i < 256; i++) {
    stringtable_index_t tmp =
        regex_cache_lookup_derivative(cache, active, (uint8_t)i);
    if (!stringtable_index_valid(tmp)) {
      return 1;
    }
    data->derivs[i] = tmp;
    intset_insert(&data->set, tmp.value);
  }

  {
    char *encoded = malloc_rewrite(regex, regexN);
    assert(encoded);
    fprintf(
        out,
        "void regex_state_%s(unsigned char * start, unsigned char * cursor, "
        "unsigned char * end, void*data)\n{\n",
        encoded);
    free(encoded);
  }

  if (intset_size(data->set) == 1) {
    const char *target = stringtable_lookup(&cache->strtab, data->derivs[0]);
    size_t targetN = __builtin_strlen(target);

    {
      char *encoded = malloc_rewrite(target, targetN);
      assert(encoded);
      fprintf(out, "  // regex \"%s\" unconditionally calls \"%s\"\n", regex,
              target);
      fprintf(out, "  return regex_state_%s(start, cursor+1, end, data);\n",
              encoded);
      free(encoded);
    }
  } else {
    fprintf(out, "  // regex \"%s\", distinct edges %zu\n", regex,
            intset_size(data->set));
    fprintf(out, "  switch(*cursor)\n  {\n");
    unsigned current_deriv_start = 0;
    stringtable_index_t current_deriv = data->derivs[0];

    for (unsigned i = 0; i < 256; i++) {
      bool last_iter = i == 255;

      stringtable_index_t next_deriv = last_iter ?
      (stringtable_index_t){.value = 0,} :
      data->derivs[i+1];

      if (last_iter || (next_deriv.value != current_deriv.value)) {

        const char *target = stringtable_lookup(&cache->strtab, current_deriv);
        size_t targetN = __builtin_strlen(target);

        {
          char *encoded = malloc_rewrite(target, targetN);
          assert(encoded);

          fprintf(
              out,
              "    case %u ... %u: return %s(start, cursor+1, end, data);\n",
              current_deriv_start, i, encoded);
          free(encoded);
        }

        // reset
        current_deriv = next_deriv;
        current_deriv_start = i + 1;
      }
    }
    fprintf(out, "  }\n");
  }

  fprintf(out, "}\n");

  return 0;
}

bool regex_driver_regex_to_c(regex_cache_t *cache, stringtable_index_t index) {
  struct to_c_func instance = {0};
  instance.set = intset_create(512);
  instance.file = stdout;

  if (!intset_valid(instance.set)) {
    return false;
  }
  bool r = regex_cache_traverse(cache, index, regex_to_c_traverse_function,
                                (void *)&instance) == 0;
  intset_destroy(instance.set);
  return r;
}
