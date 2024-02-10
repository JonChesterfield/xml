#ifndef LEXER_INSTANCE_H_INCLUDED
#define LEXER_INSTANCE_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Expects the regex of interest to be enumerated from 1, with the
// zeroth index reserved for an unknown/non-match, which is returned
// as a token with id zero, width one, and value pointing to the char*
// that none of the regex match to

// A given lexer instance is specific to a sequence of regexes
// The interface can be implemented by various different regex engines

enum lexer_engines
{
  lexer_engines_unused = 0,
  lexer_engines_multi,
  lexer_engines_posix,
  lexer_engines_re2,
  lexer_engines_re2c,
};

// Defines here controls which lexers are used by the multiple engine
// Idea is roughly to have the macros set by something which knows
// whether the engines are available on the current system and also arrange
// to not build/link any which are unused. Multi will tend to mark all those
// that can be used as in use, but posix only needs libc.

#define LEXER_POSIX_ENABLE 1

#if __has_include(<re2/re2.h>) && __has_include(<re2/set.h>)
#define LEXER_RE2_ENABLE 1
#else
#define LEXER_RE2_ENABLE 0
#endif

#define LEXER_RE2C_ENABLE 1

typedef struct
{
  void *data;
} lexer_t;

// Not sure how best to specify the interface.
// Currently nothing implements these exact symbols, but posix/multi etc
// implement ones with related names and the language instantiations
// use similar interfaces e.g. lexer_t {$lang}_lexer_create(void);
// but where the regexes array is behind the interface.

#if 0
lexer_t lexer_t_create(size_t N, const char **regexes);
void lexer_t_destroy(lexer_t);
bool lexer_t_valid(lexer_t l);  // create succeeded
lexer_token_t lexer_iterator_t_step(lexer_t, lexer_iterator_t *);
#endif

// Each lexer uses the same iterator interface
typedef struct
{
  const char *cursor;
  const char *end;
} lexer_iterator_t;

static inline lexer_iterator_t lexer_iterator_t_create(const char *bytes,
                                                       size_t len_bytes)
{
  return (lexer_iterator_t){.cursor = bytes, .end = bytes + len_bytes};
}
static inline bool lexer_iterator_t_empty(lexer_iterator_t it)
{
  return it.cursor == it.end;
}

// The id in the token is an index into the sequence of regexes
typedef struct
{
  size_t id;
  const char *value;
  size_t width;
} lexer_token_t;

// From the non-empty iterator, gets the current token and then increments.
static inline bool lexer_token_t_empty(lexer_token_t token)
{
  return token.width == 0;
}

static inline void lexer_token_dump(lexer_token_t s)
{
  printf("<LexerToken ID = \"%zu\" value = \"", s.id);
  const char *c = s.value;
  size_t w = s.width;
  for (size_t i = 0; i < w; i++)
    {
      printf("%c", *c++);
    }

  printf("\" />\n");
}

#endif
