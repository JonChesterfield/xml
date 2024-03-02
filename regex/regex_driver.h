#ifndef REGEX_DRIVER_H_INCLUDED
#define REGEX_DRIVER_H_INCLUDED

#include "../tools/hashtable.h"
#include "../tools/stringtable.h"

#include "regex_cache.h"

// true on success, false means invalid bytes or out of memory, or something
// broken internally
bool regex_driver_insert(regex_cache_t *driver, const char *bytes, size_t N);

bool regex_driver_regex_to_c(regex_cache_t *driver, stringtable_index_t index);

#endif
