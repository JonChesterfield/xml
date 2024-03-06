#ifndef LEXER_INSTANCE_T_INCLUDED
#define LEXER_INSTANCE_T_INCLUDED

#include <stddef.h>

// The id in the token is an index into the sequence of regexes
typedef struct
{
  size_t id;
  const char *value;
  size_t width;
} lexer_token_t;

// Each lexer uses the same iterator interface
typedef struct
{
  const char *cursor;
  const char *end;
} lexer_iterator_t;

typedef struct
{
  void *data;
} lexer_t;

#endif
