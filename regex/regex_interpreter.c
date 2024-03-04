#include "regex_interpreter.h"
#include "regex.h"
#include "regex_cache.h"
#include "regex_string.h"

uint64_t regex_interpreter_string_matches(unsigned char *regex,
                                          size_t regex_len,
                                          unsigned char *target,
                                          size_t target_len) {
  const uint64_t failure = UINT64_MAX;
  regex_cache_t cache = regex_cache_create();
  if (!regex_cache_valid(cache)) {
    return failure;
  }

  stringtable_index_t current =
      regex_cache_insert_regex_bytes(&cache, (const char *)regex, regex_len);
  if (!stringtable_index_valid(current)) {
    regex_cache_destroy(cache);
    return failure;
  }

  for (unsigned iter = 0; iter < target_len; iter++) {
    ptree_context ctx = regex_ptree_create_context();

    {
      ptree p = regex_from_stringtable(&cache.strtab, current, ctx);
      if (regex_nullable_p(ctx, p)) {
        regex_ptree_destroy_context(ctx);
        return iter;
      }

      if (regex_is_empty_set(p)) {
        regex_ptree_destroy_context(ctx);
        return failure;
      }
    }

    unsigned char byte = target[iter];
    current = regex_cache_calculate_derivative(&cache, current, byte);

    regex_ptree_destroy_context(ctx);
  }

  regex_cache_destroy(cache);
  return failure;
}
