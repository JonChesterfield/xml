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

  arena_module arena_mod; // the one used by hash and arena
} stringtable_t;

stringtable_t stringtable_create(void);
void stringtable_destroy(stringtable_t);
bool stringtable_valid(stringtable_t);

// only fails on out of memory. The N includes the trailing null
stringtable_index_t stringtable_insert(stringtable_t *, const char *, size_t N);

// if index was from the table, does not fail. Returns 0 if the index
// doesn't correspond to an entry
const char *stringtable_lookup(stringtable_t *tab, stringtable_index_t);


static inline bool stringtable_contains(stringtable_t *tab, stringtable_index_t index)
{
  return stringtable_lookup(tab, index) != 0;
}

// Stringtable has an exposed arena that it appends to
// One can instead append a string to that arena directly, including the null
// (while this is null terminated), provided the number of bytes appended since
// the last call to insert or record is passed to know where to reset to if
// already present
stringtable_index_t stringtable_record(stringtable_t *,
                                       uint64_t bytes_appended);

#endif
