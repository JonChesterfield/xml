#ifndef TOOLS_STRINGTABLE_H_INCLUDED
#define TOOLS_STRINGTABLE_H_INCLUDED

#include "arena.h"
#include "hashtable.h"

// Stores and deduplicates strings.
// Currently null terminated, could be sized instead.

// index is an offset into an arena. That will survive reallocation
// whereas an absolute char* would not.

typedef struct {
  uint64_t value;
} stringtable_index_t;

static inline bool stringtable_index_valid(stringtable_index_t idx) {
  return idx.value != UINT64_MAX;
}

typedef struct {
  hashtable_t hash;
  arena_t arena;
} stringtable_t;

stringtable_t stringtable_create(void);
void stringtable_destroy(stringtable_t);
bool stringtable_valid(stringtable_t);

// only fails on out of memory
stringtable_index_t stringtable_insert(stringtable_t *, const char *);

// if index is from the table, does not fail
const char *stringtable_lookup(stringtable_t *tab, stringtable_index_t);

#endif
