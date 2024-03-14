#include "regex_cache.h"
#include <assert.h>

#include "regex.h"
#include "regex.ptree.h"
#include "regex_string.h"

#include "../tools/intset.h"
#include "../tools/stack.libc.h"

bool regex_driver_insert(regex_cache_t *driver, const char *bytes, size_t N) {
  const bool verbose = false;
  
  printf("Driver insert\n");

  stringtable_index_t first = regex_cache_insert_regex_bytes(driver, bytes, N);
  if (!stringtable_index_valid(first)) {
    printf("Failed to insert initial bytes, immediately giving up\n");
    return false;
  }

  void *stack = stack_create(&stack_libc, 8);
  if (!stack) {
    return false;
  }
  stack_push_assuming_capacity(&stack_libc, stack, first.value);

  printf("Driver insert main loop\n");
  uint64_t counter = 0;
  while (stack_size(&stack_libc, stack) != 0) {
    if (verbose) printf("Driver insert iteration %lu\n", counter++);

    stringtable_index_t active = {
        .value = stack_pop(&stack_libc, stack),
    };

    if (regex_cache_all_derivatives_computed(driver, active)) {
      continue;
    }

    // Some were unknown, calculate them all
    if (!regex_cache_calculate_all_derivatives(driver, active)) {
      if (verbose) {
        printf("failed to calculate some derivative of %s:\n", stringtable_lookup(&driver->strtab, active));

      for (unsigned i = 0; i < 256; i++)
        {
          stringtable_index_t idx = regex_cache_calculate_derivative(driver, active, (uint8_t)i);
          if (stringtable_index_valid(idx)) {
            printf("  wrt %u: %s\n", i, stringtable_lookup(&driver->strtab, idx));
          } else {
            printf("  wrt %u: %s\n", i, "failure");
          }
        }
      }
      return false;
    }

    {
      void *r = stack_reserve(&stack_libc, stack,
                              stack_size(&stack_libc, stack) + 256);
      if (!r) {
        printf("out of stack\n");
        return false;
      }
      stack = r;
    }

    for (unsigned i = 0; i < 256; i++) {
      stringtable_index_t ith =
          regex_cache_lookup_derivative(driver, active, (uint8_t)i);
      if (ith.value == UINT64_MAX) {
        // Failed to calculate ith derivative
        printf("failed to lookup %u'th derivative\n", i);
        return false;
      }

      stack_push_assuming_capacity(&stack_libc, stack, ith.value);
    }
  }

  stack_libc_destroy(stack);

  printf("Claiming success\n");
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
  size_t regexN = stringtable_lookup_size(&cache->strtab, active);

  if (!regex) {
    return 1;
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

  {
    ptree_context ctx = regex_ptree_create_context();
    ptree px = regex_from_stringtable(&cache->strtab, active, ctx);

    if (regex_is_empty_set(px))
      {
        fprintf(out, "    // regex %s is empty set\n", regex);
        fprintf(out,
                "    return regex_state_reject(start, cursor, end, data);\n");
        fprintf(out, "}\n");
        regex_ptree_destroy_context(ctx);
        return 0;        
      }

    if (regex_nullable_p(ctx, px))
      {
        fprintf(out, "    // regex %s is nullable\n", regex);
        fprintf(out,
                "    return regex_state_accept(start, cursor, end, data);\n");
        fprintf(out, "}\n");
        regex_ptree_destroy_context(ctx);
        return 0;                
      }

    regex_ptree_destroy_context(ctx);           
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


  // todo, special case intset_size(data->set) == 1)

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
      size_t targetN = stringtable_lookup_size(&cache->strtab, current_deriv);

      {
        char *encoded = malloc_rewrite(target, targetN);
        assert(encoded);

        ptree_context ctx = regex_ptree_create_context();
        ptree px = regex_from_stringtable(&cache->strtab, current_deriv, ctx);
        bool nullable = regex_nullable_p(ctx, px);
        bool empty = regex_is_empty_set(px);

        const char * target = encoded;        
        if (nullable || empty) {
          const char * shortcase = nullable ? "nullable" : "empty set";
          fprintf(out, "    // regex %s is %s\n", target, shortcase);          
          target = nullable ? "accept" : "reject";
        }

        fprintf(
                out,
                "    case %u ... %u: return regex_state_%s(start, cursor, "
                "end, data);\n",
                current_deriv_start, i, target);

        regex_ptree_destroy_context(ctx);
        free(encoded);
      }

      // reset
      current_deriv = next_deriv;
      current_deriv_start = i + 1;
    }
  }
  fprintf(out, "  }\n");

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

  // lowercase t is not an identifier thus accept is not an encoding of a regex
  fprintf(instance.file,
          "void regex_state_accept(unsigned char * start, unsigned char * "
          "cursor, unsigned char * end, void*data);\n");
  fprintf(instance.file,
          "void regex_state_reject(unsigned char * start, unsigned char * "
          "cursor, unsigned char * end, void*data);\n");
  fprintf(instance.file, "\n");

  bool r = regex_cache_traverse(cache, index, regex_to_c_traverse_function,
                                (void *)&instance) == 0;
  intset_destroy(instance.set);
  return r;
}
