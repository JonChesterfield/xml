#include "regex_interpreter.h"
#include "regex.h"
#include "regex_cache.h"
#include "regex_string.h"

#include <assert.h>
#include "../tools/stack.libc.h"
#include "../tools/stringtable.h"

#include "interpreter.data"

bool regex_interpreter_match_failure(uint64_t x)
{
  return x == match_failure;
}
  
bool regex_interpreter_machine_failure(uint64_t x)
{
  return x == machine_failure;
}

bool regex_interpreter_regex_unrecognised(uint64_t x)
{
  return x == regex_unrecognised;
}

bool regex_interpreter_success(uint64_t x)
{
  switch(x)
    {
    case match_failure:
    case machine_failure:
    case regex_unrecognised:
      return false;
    default:
      return true;
    }
}


uint64_t regex_interpreter_with_context_string_matches(regex_cache_t *cache,
                                                       const unsigned char *regex,
                                                       size_t regex_len,
                                                       const unsigned char *target,
                                                       size_t target_len)
{
  if (!cache || !regex_cache_valid(*cache)) {
    return machine_failure;
  }

  stringtable_index_t current =
      regex_cache_insert_regex_bytes(cache, (const char *)regex, regex_len);
  if (!stringtable_index_valid(current)) {
    #if 0
    // Appears canonicalise fails to compute a fixpoint
    printf("not recognised: %lu\n", current.value);

    ptree_context ctx = regex_ptree_create_context();
    ptree p = regex_from_char_sequence(ctx, regex, regex_len);
    if (ptree_is_failure(p)) {
      printf("didn't look like a regex\n");
    } else {
      regex_ptree_as_xml(&stack_libc, stdout, p);
    }

    ptree c = regex_canonicalise(ctx, p);
    printf("canon:\n");
    regex_ptree_as_xml(&stack_libc, stdout, c);

    if (!ptree_is_failure(c)) {
      if (!regex_is_canonical(c)) {
        printf("non-canonical canonical\n");
      }
      current = regex_cache_insert_regex_canonical_ptree(cache, c);
      printf("not recognised2 ?: %lu\n", current.value);
    }
    
    #endif
    return regex_unrecognised;
  }


  
  // context handling here would be much improved by a clear-and-reuse model
  for (unsigned iter = 0; iter < target_len; iter++) {
    ptree_context ctx = regex_ptree_create_context();
    assert(regex_ptree_valid_context(ctx));
    {      
      ptree p = regex_from_stringtable(&cache->strtab, current, ctx);

      const char * r = stringtable_lookup(&cache->strtab, current);      
      
      if (regex_nullable_p(ctx, p)) {
        regex_ptree_destroy_context(ctx);
        printf("Iter %u, %s. Nullable\n", iter, r);
        return iter;
      }

      if (regex_is_empty_set(p)) {
        regex_ptree_destroy_context(ctx);
        printf("Iter %u, %s. Failure\n", iter, r);
        return match_failure;
      }
    }

    unsigned char byte = target[iter];
    current = regex_cache_calculate_derivative(cache, current, byte);

    regex_ptree_destroy_context(ctx);
  }


  return match_failure;
}

uint64_t regex_interpreter_string_matches(const unsigned char *regex,
                                          size_t regex_len,
                                          const unsigned char *target,
                                          size_t target_len) {

  regex_cache_t cache = regex_cache_create();
  if (!regex_cache_valid(cache)) {
    return machine_failure;
  }

  uint64_t res = regex_interpreter_with_context_string_matches(&cache,regex,regex_len,target,target_len);

  regex_cache_destroy(cache);

  return res;
}
