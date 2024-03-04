#ifndef REGEX_QUEURIES_H_INCLUDED
#define REGEX_QUEURIES_H_INCLUDED

#include "regex_cache.h"

typedef struct
{
  uint64_t transition;
  uint64_t accepting;
  uint64_t rejecting;
  bool success; // true if other fields are valid, false if something went wrong
} regex_query_state_count_t;

regex_query_state_count_t
regex_query_states(regex_cache_t *,
                   stringtable_index_t);

uint64_t regex_query_total_states(regex_cache_t * c,
                                  stringtable_index_t r)
{
  regex_query_state_count_t res = regex_query_states(c, r);
  return res.success ? (res.transition + res.accepting + res.rejecting) : UINT64_MAX;
}

static inline uint64_t regex_query_transition_states(regex_cache_t * c,
                                      stringtable_index_t r)
{
  regex_query_state_count_t res = regex_query_states(c, r);
  return res.success ? res.transition : UINT64_MAX;
}

static inline uint64_t regex_query_accepting_states(regex_cache_t * c,
                                      stringtable_index_t r)
{
  regex_query_state_count_t res = regex_query_states(c, r);
  return res.success ? res.accepting : UINT64_MAX;

}

static inline uint64_t regex_query_rejecting_states(regex_cache_t * c,
                                      stringtable_index_t r)
{
  regex_query_state_count_t res = regex_query_states(c, r);
  return res.success ? res.rejecting : UINT64_MAX;

}


#endif
