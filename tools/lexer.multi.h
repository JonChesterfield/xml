#ifndef LEXER_MULTI_H_INCLUDED
#define LEXER_MULTI_H_INCLUDED

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

// Presently implemented in this header
static inline lexer_t lexer_multi_create(size_t N, const char **regexes);
static inline void lexer_multi_destroy(lexer_t lex);
static inline bool lexer_multi_valid(lexer_t lex);
static inline lexer_token_t lexer_multi_iterator_step(lexer_t l,
                                                      lexer_iterator_t *iter);

#ifndef LEXER_POSIX_ENABLE
#error "Require macro LEXER_POSIX_ENABLE"
#endif

#ifndef LEXER_RE2_ENABLE
#error "Require macro LEXER_RE2_ENABLE"
#endif

#define LEXER_MULTI_COUNT() \
  (LEXER_POSIX_ENABLE ? 1 : 0) + (LEXER_RE2_ENABLE ? 1 : 0)

#if LEXER_POSIX_ENABLE
#include "lexer.posix.h"
#endif

#if LEXER_RE2_ENABLE
#include "lexer.re2.h"
#endif

typedef struct
{
  enum lexer_engines engine;
#if LEXER_POSIX_ENABLE
  lexer_t posix;
#endif
#if LEXER_RE2_ENABLE
  lexer_t re2;
#endif
} lexer_multi_t;

static inline lexer_t lexer_multi_create(size_t N, const char **regexes)
{
  lexer_multi_t *multi = malloc(sizeof(lexer_multi_t));
  lexer_t result = {
      .data = multi,
  };
  if (!multi)
    {
      return result;
    }
  multi->engine = lexer_engines_multi;

#if LEXER_POSIX_ENABLE
  multi->posix = lexer_posix_create(N, regexes);
#endif
#if LEXER_RE2_ENABLE
  multi->re2 = lexer_re2_create(N, regexes);
#endif

  return result;
}

static inline void lexer_multi_destroy(lexer_t lex)
{
  lexer_multi_t *multi = lex.data;
  if (!multi)
    {
      return;
    }
  assert(multi->engine == lexer_engines_multi);

#if LEXER_POSIX_ENABLE
  lexer_posix_destroy(multi->posix);
#endif
#if LEXER_RE2_ENABLE
  lexer_re2_destroy(multi->re2);
#endif
}

static inline bool lexer_multi_valid(lexer_t lex)
{
  lexer_multi_t *multi = lex.data;
  if (!multi)
    {
      return false;
    }
  assert(multi->engine == lexer_engines_multi);
  bool valid = true;

#if LEXER_POSIX_ENABLE
  valid &= lexer_posix_valid(multi->posix);
#endif
#if LEXER_RE2_ENABLE
  valid &= lexer_re2_valid(multi->re2);
#endif

  return valid;
}

static inline lexer_token_t lexer_multi_iterator_step(lexer_t l,
                                                      lexer_iterator_t *iter)
{
  enum
  {
    N = LEXER_MULTI_COUNT()
  };
  assert(lexer_multi_valid(l));
  lexer_multi_t *multi = l.data;
  assert(multi);
  assert(multi->engine == lexer_engines_multi);
  lexer_iterator_t iters[N];
  lexer_token_t tokens[N];
  for (unsigned i = 0; i < N; i++)
    {
      iters[i] = *iter;
    }
  unsigned i = 0;

#if LEXER_POSIX_ENABLE
  tokens[i] = lexer_posix_iterator_step(multi->posix, &iters[i]);
  i++;
#endif
#if LEXER_RE2_ENABLE
  tokens[i] = lexer_re2_iterator_step(multi->re2, &iters[i]);
  i++;
#endif

  for (unsigned i = 1; i < N; i++)
    {
      bool iter_cursor_ok = iters[i].cursor == iters[0].cursor;
      bool iter_end_ok = iters[i].end == iters[0].end;

      bool token_id_ok = tokens[i].id == tokens[0].id;
      bool token_value_ok = tokens[i].value == tokens[0].value;
      bool token_width_ok = tokens[i].width == tokens[0].width;

      bool all_ok = iter_cursor_ok & iter_end_ok & token_id_ok &
                    token_value_ok & token_width_ok;

      if (!all_ok)
        {
          fprintf(stderr, "Inconsistent lexers\n");
        }
    }

  *iter = iters[0];
  return tokens[0];
}

#endif
