#ifndef REGEX_EQUALITY_H_INCLUDED
#define REGEX_EQUALITY_H_INCLUDED

#include "regex.h"
#include "regex.ptree.h"
#include "regex_cache.h"

bool regex_ptree_is_atomic(ptree);
bool regex_ptree_atomic_and_equal(ptree, ptree);
bool regex_ptree_definitionally_equal(ptree, ptree);
bool regex_ptree_similar(ptree, ptree);
bool regex_ptree_equivalent(regex_cache_t*,ptree, ptree);


bool regex_canonical_is_atomic(stringtable_t*,
                               stringtable_index_t);

bool regex_canonical_atomic_and_equal(stringtable_t*,
                                      stringtable_index_t,
                                      stringtable_index_t);

bool regex_canonical_definitionally_equal(stringtable_t*,
                                          stringtable_index_t,
                                          stringtable_index_t);

bool regex_canonical_similar(stringtable_t*,
                             stringtable_index_t,
                             stringtable_index_t);

// The two regex match exactly the same strings
bool regex_canonical_equivalent(regex_cache_t*,
                                stringtable_index_t,
                                stringtable_index_t);

// The two regex match different strings
bool regex_canonical_distinct(regex_cache_t*,
                              stringtable_index_t,
                              stringtable_index_t);

// Find a string that one matches and the other does not
stringtable_index_t
regex_canonical_distinguished(regex_cache_t*,
                              stringtable_index_t,
                              stringtable_index_t);

// equal to empty set
bool regex_canonical_matches_nothing(regex_cache_t*,
                                     stringtable_index_t);

// equal to .*
bool regex_canonical_matches_everything(regex_cache_t*,
                                        stringtable_index_t);


// test if cmp is a subset of base
bool regex_canonical_includes(regex_cache_t*,
                              stringtable_index_t base,
                              stringtable_index_t cmp);


#endif
