#ifndef LEXER_RE2C_H_INCLUDED
#define LEXER_RE2C_H_INCLUDED

#include "lexer.h"

lexer_t lexer_re2c_create(size_t N, const char** regexes);
void lexer_re2c_destroy(lexer_t lex);
bool lexer_re2c_valid(lexer_t lex);
// Iterator step is generated per-regex-list for re2c

#endif
