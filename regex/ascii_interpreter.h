#ifndef ASCII_INTERPRETER_H_INCLUDED
#define ASCII_INTERPRETER_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool ascii_interpreter_match_failure(uint64_t);
bool ascii_interpreter_machine_failure(uint64_t);
bool ascii_interpreter_regex_unrecognised(uint64_t);
bool ascii_interpreter_success(uint64_t);

// Parse ascii regex string, walk target a byte at a time, differentiating
// regex on the fly. Hence interpreter.
// Implicitly anchored at the start, Altnatively, seeks a match starting at 0.
// If success() on the result, return value is length of match.
uint64_t ascii_interpreter_string_matches(const char *regex,
                                          unsigned char *target,
                                          size_t target_len);


// 0 on failure, null terminated malloc'ed memory on success
char * ascii_regex_as_prefix_regex_c_string(const char *regex);

#endif
