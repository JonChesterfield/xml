#ifndef REGEX_CACHE_H_INCLUDED
#define REGEX_CACHE_H_INCLUDED

#include "../tools/hashtable.h"
#include "../tools/intmap.h"
#include "../tools/lexer.t"
#include "../tools/stringtable.h"
#include "regex.ptree.h"

// Stores regex in the string format, represented by stringtable_index_t
// instances. The hashtable is from one stringtable_index_t (8 bytes) to an
// array of 256 of them.
// Because the stringtable deduplicates and the hashtable stores integer
// offsets into the stringtab, the hash and key compare can be on the uint64_ts

// Stores canonical regex in char sequence form
typedef struct {
  hashtable_t regex_to_derivatives;
  intmap_t regex_to_properties;
  stringtable_t strtab;
} regex_cache_t;

regex_cache_t regex_cache_create(void);
void regex_cache_destroy(regex_cache_t);
bool regex_cache_valid(regex_cache_t);

// Converts the regex to canonical form first
stringtable_index_t regex_cache_insert_regex_ptree(regex_cache_t *,
                                                   ptree_context, ptree regex);

// Converts bytes to a canonical form. Unsigned?
stringtable_index_t regex_cache_insert_regex_bytes(regex_cache_t *,
                                                   const char *bytes, size_t N);

// assumes regex is canonical, fast path
stringtable_index_t regex_cache_insert_regex_canonical_ptree(regex_cache_t *,
                                                             ptree regex);

// Compute ith derivative, record it in the cache
stringtable_index_t regex_cache_calculate_derivative(regex_cache_t *,
                                                     stringtable_index_t,
                                                     uint8_t ith);

// Return it if already computed, otherwise failure
stringtable_index_t regex_cache_lookup_derivative(regex_cache_t *,
                                                  stringtable_index_t,
                                                  uint8_t ith);

enum regex_cache_lookup_properties {
  regex_cache_lookup_failure = 0, // no information available
  regex_cache_lookup_empty_string = 1,
  regex_cache_lookup_empty_set = 1 << 1,
  regex_cache_lookup_composite = 1 << 2, // not empty set or empty string
  regex_cache_lookup_nullable = 1 << 3,

  // Force enum to be 64 bits wide
  regex_cache_lookup_minimum_id = 1 << 32u,
  regex_cache_lookup_maximum_id = UINT32_MAX << 32u,
};

static inline bool
regex_properties_is_failure(enum regex_cache_lookup_properties props) {
  return props == 0;
}

static inline bool
regex_properties_is_empty_string(enum regex_cache_lookup_properties props) {
  props &= UINT32_MAX;
  return (props & regex_cache_lookup_empty_string) != 0;
}

static inline bool
regex_properties_is_empty_set(enum regex_cache_lookup_properties props) {
  props &= UINT32_MAX;
  return (props & regex_cache_lookup_empty_set) != 0;
}

static inline bool
regex_properties_is_nullable(enum regex_cache_lookup_properties props) {
  props &= UINT32_MAX;
  return (props & regex_cache_lookup_nullable) != 0;
}

static inline uint64_t regex_properties_retrieve_root_identifier(
    enum regex_cache_lookup_properties props) {
  props = props >> 32u;
  return props;
}

enum regex_cache_lookup_properties
regex_cache_lookup_properties(regex_cache_t *, stringtable_index_t);

// As above, but check all of them
bool regex_cache_all_derivatives_computed(regex_cache_t *, stringtable_index_t);

// True on success. Calculate the derivative with respect to each byte,
// non-recursive
bool regex_cache_calculate_all_derivatives(regex_cache_t *,
                                           stringtable_index_t);

// Call functor on every regex, root first, breadth first, without repetition
// until functor returns != 0. Will calculate derivatives and add to the cache
// as required.
int regex_cache_traverse(regex_cache_t *, stringtable_index_t root,
                         int (*functor)(regex_cache_t *, stringtable_index_t,
                                        void *data),
                         void *data);

// With lexer customisation

stringtable_index_t regex_cache_insert_regex_bytes_using_lexer(
    lexer_t, lexer_token_t (*lexer_iterator_step)(lexer_t, lexer_iterator_t *),
    regex_cache_t *, const char *bytes, size_t N);

stringtable_index_t regex_cache_calculate_derivative_using_lexer(
    lexer_t, lexer_token_t (*lexer_iterator_step)(lexer_t, lexer_iterator_t *),
    regex_cache_t *, stringtable_index_t, uint8_t ith);

bool regex_cache_calculate_all_derivatives_using_lexer(
    lexer_t, lexer_token_t (*lexer_iterator_step)(lexer_t, lexer_iterator_t *),
    regex_cache_t *, stringtable_index_t);

int regex_cache_traverse_using_lexer(
    lexer_t, lexer_token_t (*lexer_iterator_step)(lexer_t, lexer_iterator_t *),
    regex_cache_t *, stringtable_index_t root,
    int (*functor)(regex_cache_t *, stringtable_index_t, void *data),
    void *data);

#endif
