#ifndef LEXER_RE2_H_INCLUDED
#define LEXER_RE2_H_INCLUDED

#include "lexer.h"

#if LEXER_RE2_ENABLE
lexer_t lexer_re2_create(size_t N, const char** regexes);
void lexer_re2_destroy(lexer_t lex);
bool lexer_re2_valid(lexer_t lex);
lexer_token_t lexer_re2_iterator_step(lexer_t l, lexer_iterator_t* iter);
#endif

#endif
