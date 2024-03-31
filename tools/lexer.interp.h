#ifndef LEXER_INTERP_H_INCLUDED
#define LEXER_INTERP_H_INCLUDED

#include "lexer.h"

lexer_t lexer_interp_create(size_t N, const char** regexes);
void lexer_interp_destroy(lexer_t lex);
bool lexer_interp_valid(lexer_t lex);
lexer_token_t lexer_interp_iterator_step(lexer_t l, lexer_iterator_t* iter);

#endif
