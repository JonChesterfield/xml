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
    return regex_unrecognised;
  }

  const char * r = stringtable_lookup(&cache->strtab, current);
  printf("Matching regex %s/%s len %lu\n", regex,r,regex_len);
  
  // context handling here would be much improved by a clear-and-reuse model
  for (unsigned iter = 0; iter < target_len; iter++) {
    ptree_context ctx = regex_ptree_create_context();
    assert(regex_ptree_valid_context(ctx));

    bool current_nullable = false;
    
    {      
      ptree p = regex_from_stringtable(&cache->strtab, current, ctx);

      const char * r = stringtable_lookup(&cache->strtab, current);      

      if (regex_nullable_p(ctx, p)) {
        // return iter here is eager
        current_nullable = true;
      }

      bool empty_set = regex_is_empty_set(p);
      bool empty_string = regex_is_empty_string(p);
      if (empty_set) {assert(!empty_string);}
      if (empty_string) {assert(!empty_set);}
      
      if (empty_set) {
        regex_ptree_destroy_context(ctx);
        printf("Iter %u, %s. Failure\n", iter, r);
        return match_failure;
      }

      
      if (empty_string) {
        printf("Iter %u, %s. Empty string\n", iter, r);
        return iter;
      }

      printf("Iter %u, %s. Continue\n", iter, r);
        
      
    }

    unsigned char byte = target[iter];
    stringtable_index_t next = regex_cache_calculate_derivative(cache, current, byte);

    if (current_nullable)
      {
        ptree n = regex_from_stringtable(&cache->strtab, next, ctx);
        if (regex_is_empty_set(n)) {
          const char * r = stringtable_lookup(&cache->strtab, current);
          printf("Iter %u, %s. Next last nullable\n", iter, r);
          return iter;
        }
      }
    
    current = next;
    regex_ptree_destroy_context(ctx);
  }

  {
  ptree_context ctx = regex_ptree_create_context();
  bool nullable = regex_nullable_p(ctx, regex_from_stringtable(&cache->strtab, current, ctx));
  regex_ptree_destroy_context(ctx);
  
  if (nullable) {
    printf("reached end of input, current is nullable\n");
    return target_len;
  } else {
    printf("reached end of input, current is not nullable\n");
    return match_failure;
  }
  }
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
