#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include <stddef.h>

#include "token.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  void *data;
  size_t N;
  const char **token_names;
  const char **regexes;
} lexer_state;

bool lexer_success(lexer_state);
// 0 is reserved to be unknown and regex "."
// which makes the lexer complete and lines up with lemon
// treating 0 as reserved
lexer_state lexer_create(size_t N, const char **token_names,
                         const char **regexes);
void lexer_destroy(lexer_state);

token lexer_next(lexer_state s, const char *start, const char *end);

#ifdef __cplusplus
}
#endif

#endif
