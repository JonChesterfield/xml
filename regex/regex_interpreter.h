#ifndef REGEX_INTERPRETER_H_INCLUDED
#define REGEX_INTERPRETER_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "regex_cache.h" // maybe forward declare the type

bool regex_interpreter_match_failure(uint64_t);
bool regex_interpreter_machine_failure(uint64_t);
bool regex_interpreter_regex_unrecognised(uint64_t);
bool regex_interpreter_success(uint64_t);

// Parse regex string, walk target a byte at a time, differentiating
// regex on the fly. Hence interpreter.
// Implicitly anchored at the start, Altnatively, seeks a match starting at 0.
// If success() on the result, return value is length of match.
uint64_t regex_interpreter_string_matches(const unsigned char *regex,
                                          size_t regex_len,
                                          const unsigned char *target,
                                          size_t target_len);

// As above but with a pre-existing cache of regex derivatives
uint64_t regex_interpreter_with_context_string_matches(regex_cache_t *context,
                                                       const unsigned char *regex,
                                                       size_t regex_len,
                                                       const unsigned char *target,
                                                       size_t target_len);

#endif
