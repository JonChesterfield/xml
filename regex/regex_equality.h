#ifndef REGEX_EQUALITY_H_INCLUDED
#define REGEX_EQUALITY_H_INCLUDED

#include "regex.h"
#include "regex.ptree.h"
#include "regex_cache.h"

typedef enum {
  regex_compare_out_of_memory,
  regex_compare_failure, // could be OOM, could be ill formed input
  regex_compare_equal,
  regex_compare_not_equal,
} regex_compare_t;

// A bool when the method does not allocate memory, otherwise an enum

bool regex_ptree_is_atomic(ptree);
bool regex_ptree_atomic_and_equal(ptree, ptree);

regex_compare_t regex_ptree_definitionally_equal(ptree, ptree);

regex_compare_t regex_ptree_similar(ptree, ptree);
regex_compare_t regex_ptree_equivalent(regex_cache_t *, ptree, ptree);

bool regex_canonical_is_atomic(regex_cache_t *, stringtable_index_t);

bool regex_canonical_atomic_and_equal(regex_cache_t *, stringtable_index_t,
                                      stringtable_index_t);

bool regex_canonical_definitionally_equal(regex_cache_t *, stringtable_index_t,
                                          stringtable_index_t);

regex_compare_t regex_canonical_similar(regex_cache_t *, stringtable_index_t,
                             stringtable_index_t);

// The two regex match exactly the same strings
regex_compare_t regex_canonical_equivalent(regex_cache_t *, stringtable_index_t,
                                stringtable_index_t);

// The two regex match different strings
bool regex_canonical_distinct(regex_cache_t *, stringtable_index_t,
                              stringtable_index_t);

// There are various generative queries one might do
// Find a string that matches the regex, enumerate all strings that match
// Test if there are no accepting states
//

// Find a string that one matches and the other does not
stringtable_index_t regex_canonical_distinguished(regex_cache_t *,
                                                  stringtable_index_t,
                                                  stringtable_index_t);

// equal to empty set
bool regex_canonical_matches_nothing(regex_cache_t *, stringtable_index_t);

// equal to .*
bool regex_canonical_matches_everything(regex_cache_t *, stringtable_index_t);

// test if cmp is a subset of base
bool regex_canonical_includes(regex_cache_t *, stringtable_index_t base,
                              stringtable_index_t cmp);

#endif
