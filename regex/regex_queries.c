#include "regex_queries.h"
#include "regex_string.h"
#include "regex.h"

static int func(regex_cache_t * cache,
         stringtable_index_t r,
         void* arg_data)
{
  regex_query_state_count_t * data = (regex_query_state_count_t*)arg_data;

  ptree_context ctx = regex_ptree_create_context();

  // todo, would like nullable/empty etc available without allocating
  ptree p = regex_from_stringtable(&cache->strtab, r, ctx);
  if (regex_nullable_p(ctx, p)) {   
    data->accepting++;
  } else  if (regex_is_empty_set(p)) {
    data->rejecting++;
  } else {
    data->transition++;
  }

  regex_ptree_destroy_context(ctx);
  return 0;
}

regex_query_state_count_t
regex_query_states(regex_cache_t * cache,
                   stringtable_index_t r)
{
  regex_query_state_count_t counts = {0};
  int rc = regex_cache_traverse(cache,r,func, &counts);
  counts.success = (rc == 0);
  return counts;
}
