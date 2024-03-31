#include "regex_interpreter.h"
#include "regex.h"
#include "regex_cache.h"
#include "regex_string.h"

#include "../tools/stack.libc.h"
#include "../tools/stringtable.h"
#include <assert.h>

#include "interpreter.data"

bool regex_interpreter_match_failure(uint64_t x) { return x == match_failure; }

bool regex_interpreter_machine_failure(uint64_t x) {
  return x == machine_failure;
}

bool regex_interpreter_regex_unrecognised(uint64_t x) {
  return x == regex_unrecognised;
}

bool regex_interpreter_success(uint64_t x) {
  switch (x) {
  case match_failure:
  case machine_failure:
  case regex_unrecognised:
    return false;
  default:
    return true;
  }
}

uint64_t regex_interpreter_known_regex_matches(regex_cache_t *cache,
                                               stringtable_index_t regex,
                                               const unsigned char *target,
                                               size_t target_len) {
  const bool verbose = false;

  if (!cache || !regex_cache_valid(*cache)) {
    return machine_failure;
  }

  if (!stringtable_index_valid(regex)) {
    return regex_unrecognised;
  }

  stringtable_index_t current = regex;

  const char *r = stringtable_lookup(&cache->strtab, current);
  if (r == 0) {
    return regex_unrecognised;
  }
  size_t N = stringtable_lookup_size(&cache->strtab, current);

  if (verbose)
    printf("Matching regex %.*s len %lu\n", (int)N, r, N);

  // Probably handle longest-wins usng a local variable
  uint64_t match_end = match_failure;

  // context handling here would be much improved by a clear-and-reuse model
  for (unsigned iter = 0; iter < target_len; iter++) {
    ptree_context ctx = regex_ptree_create_context();
    assert(regex_ptree_valid_context(ctx));

    unsigned char byte = target[iter];
    current = regex_cache_calculate_derivative(cache, current, byte);

    enum regex_cache_lookup_properties props =
        regex_cache_lookup_properties(cache, current);

    bool nullable = regex_properties_is_nullable(props);
    bool empty_set = regex_properties_is_empty_set(props);
    // bool empty_string = regex_properties_is_empty_string(props);

    {
      const char *r = "";

      if (nullable || empty_set) {
        r = stringtable_lookup(&cache->strtab, current);
      }

      if (nullable) {
        // return here would be eager / shorted wins
        if (verbose)
          printf("Iter %u, %s. Nullable\n", iter, r);
        match_end = iter + 1; // returns one past the last char in the match
      }

      if (empty_set) {
        regex_ptree_destroy_context(ctx);
        if (verbose)
          printf("Iter %u, %s. Failure\n", iter, r);
        break;
      }
    }

    regex_ptree_destroy_context(ctx);
  }

  return match_end;
}

uint64_t regex_interpreter_with_context_string_matches(
    regex_cache_t *cache, const unsigned char *regex, size_t regex_len,
    const unsigned char *target, size_t target_len) {
  if (!cache || !regex_cache_valid(*cache)) {
    return machine_failure;
  }

  stringtable_index_t current =
      regex_cache_insert_regex_bytes(cache, (const char *)regex, regex_len);
  if (!stringtable_index_valid(current)) {
    return regex_unrecognised;
  }

  return regex_interpreter_known_regex_matches(cache, current, target,
                                               target_len);
}

uint64_t regex_interpreter_string_matches(const unsigned char *regex,
                                          size_t regex_len,
                                          const unsigned char *target,
                                          size_t target_len) {

  regex_cache_t cache = regex_cache_create();
  if (!regex_cache_valid(cache)) {
    return machine_failure;
  }

  uint64_t res = regex_interpreter_with_context_string_matches(
      &cache, regex, regex_len, target, target_len);

  regex_cache_destroy(cache);

  return res;
}
