#ifndef TOOLS_STRINGTABLE_H_INCLUDED
#define TOOLS_STRINGTABLE_H_INCLUDED

#include "arena.h"
#include "hashtable.h"

// Stores and deduplicates strings.

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

// only fails on out of memory.
stringtable_index_t stringtable_insert(stringtable_t *, const char *, size_t N);

// Inserts N bytes, but also arranges for it to be followed by 0 without including that
// in the size of the string, mainly for passing the result to printf
// This 0 is ignored wrt deduplication
stringtable_index_t stringtable_insert_with_trailing_nul(stringtable_t *, const char *, size_t N);


// if index was from the table, does not fail. Returns 0 if the index
// doesn't correspond to an entry
const char *stringtable_lookup(stringtable_t *tab, stringtable_index_t);

// returns SIZE_MAX if hte index doesn't correspond to an entry
size_t stringtable_lookup_size(stringtable_t *tab, stringtable_index_t);


bool stringtable_contains(stringtable_t *tab, stringtable_index_t index);

// Stringtable has an exposed arena that it appends to
// One can instead append a string to that arena directly,
// provided the number of bytes appended since
// the last call to insert or record is passed to know where to reset to if
// already present
stringtable_index_t stringtable_record(stringtable_t *,
                                       uint64_t bytes_appended);

stringtable_index_t stringtable_record_with_trailing_nul(stringtable_t *,
                                                          uint64_t bytes_appended);



#endif
