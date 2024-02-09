#ifndef ARITH_LEXER_H_INCLUDED
#define ARITH_LEXER_H_INCLUDED

#include <stdio.h>
#include "../tools/token.h"

#ifdef __cplusplus
extern "C" {
#endif

// Idea is to abstract over which regex engine is in use and over
// the list of regular expressions that define the lexer for the
// langauge "arith"
// Probably want an external iterator
// Might want an option to expose the list of integer ids in token
int arith_lexer_foreach_token_in_file(FILE *f, int (*func)(void*, token tok), void*);
int arith_lexer_to_xml(FILE *f);  
int arith_lexer_token_to_index(token tok); // -1 on unknown

#ifdef __cplusplus
}
#endif

#endif
