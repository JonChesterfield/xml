#ifndef REGEX_STRING_H_INCLUDED
#define REGEX_STRING_H_INCLUDED
#include "../tools/arena.h"
#include "../tools/ptree.h"

// construct into the passed context. N is exclusive of nul, no nul required
ptree regex_from_char_sequence(ptree_context ctx, const char * bytes, size_t N);

// append to arena. Maybe return bool? Should leave arena unchanged on failure
// todo: failures not well checked yet. Does not allocate a trailing 0.
int regex_to_char_sequence(arena_module mod, arena_t *arena, ptree val);

// returns 0 on failure, need to free() it on succes, nul terminated
char * regex_to_malloced_c_string(ptree val);

// bytes represent a valid regex
bool regex_in_byte_representation(const char * bytes, size_t N);

#endif
