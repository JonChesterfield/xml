#ifndef REGEX_DRIVER_H_INCLUDED
#define REGEX_DRIVER_H_INCLUDED

#include "../tools/stringtable.h"
#include "../tools/hashtable.h"

// Stores regex in the string format, represented by stringtable_index_t
// instances. The hashtable is from one stringtable_index_t (8 bytes) to an
// array of 256 of them.
// Because the stringtable deduplicates and the hashtable stores integer
// offsets into the stringtab, the hash and key compare can be on the uint64_ts

typedef struct {
  hashtable_t regex_to_derivatives;
  stringtable_t strtab;
} regex_driver_t;

regex_driver_t regex_driver_create(void);
void regex_driver_destroy(regex_driver_t);
bool regex_driver_valid(regex_driver_t);

// true on success, false means invalid bytes or out of memory, or something broken internally
bool regex_driver_insert(regex_driver_t * driver, const char * bytes, size_t N);

bool regex_driver_regex_to_c(regex_driver_t * driver, stringtable_index_t index);

#endif
