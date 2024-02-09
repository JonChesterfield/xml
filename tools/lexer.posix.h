#ifndef LEXER_POSIX_H_INCLUDED
#define LEXER_POSIX_H_INCLUDED

#include "lexer.h"

lexer_t lexer_posix_create(size_t N, const char** regexes);
void lexer_posix_destroy(lexer_t lex);
bool lexer_posix_valid(lexer_t lex);
lexer_token_t lexer_posix_iterator_step(lexer_t l, lexer_iterator_t* iter);

#endif
