#ifndef REGEX_STRING_H_INCLUDED
#define REGEX_STRING_H_INCLUDED
#include "../tools/arena.h"
#include "../tools/ptree.h"

int regex_to_char_sequence(arena_module mod, arena_t *arena, ptree val);
ptree regex_from_char_sequence(ptree_context ctx, const char * bytes, size_t N);

#endif
