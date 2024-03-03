#ifndef REGEX_INTERPRETER_H_INCLUDED
#define REGEX_INTERPRETER_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

// Parse regex string, walk target a byte at a time, differentiating
// regex on the fly. Hence interpreter.
// Implicitly anchored at the start, returns the number of bytes matched or
// UINT64_MAX on failure Also returns UINT64_MAX on out of memory at present
uint64_t regex_interpreter_string_matches(unsigned char *regex,
                                          size_t regex_len,
                                          unsigned char *target,
                                          size_t target_len);

#endif
