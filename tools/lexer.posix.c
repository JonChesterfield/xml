#include "lexer.posix.h"

#include <assert.h>
#include <regex.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This interface can't tolerate null characters in the searched for string
// because regexec takes a null terminated string, not a sized one
//
// The regex.h interface doesn't seem to allow anchoring a regex to the start
// of the buffer which means it'll search the entire buffer looking for things
// like whitespace.
// TODO: Force a leading ^ to workaround that
// It also doesn't allow compiling multiple regex into a single DFA to suppport
// simultaneous match testing.
// Those combined should mean "slow" relative to re2 or similar but hopefully
// source code is usually small enough for that to not matter too much.


typedef struct
{
#ifndef NDEBUG
  enum lexer_engines engine;
#endif
  size_t size;
  regex_t pats[];
} posix_lexer;

static void set_engine(posix_lexer* l)
{
#ifndef NDEBUG
  l->engine = lexer_engines_posix;
#else
  (void)l;
#endif
}

static void check_engine(posix_lexer* l)
{
#ifndef NDEBUG
  if (l != NULL)
    {
      unsigned num = l->engine;
      if (num != lexer_engines_posix)
        {
          fprintf(stderr,
                  "Invalid magic number (%u) for lexer.posix, aborting\n", num);
          abort();
        }
    }
#else
  (void)l;
#endif
}

static lexer_t from_posix(posix_lexer* l)
{
  check_engine(l);
  return (lexer_t){.data = (void*)l};
}

static posix_lexer* to_posix(lexer_t l)
{
  posix_lexer* lexer = (posix_lexer*)l.data;
  check_engine(lexer);
  return lexer;
}

static lexer_t lexer_t_failed(void) { return from_posix(0); }

lexer_t lexer_posix_create(size_t N, const char** regexes)
{
  assert(N > 0);
  // Haven't found documentation for what an extended regex is, but without this
  // \( refuses to compile (and \\( doesn't match an open paren)) and r+ is not
  // recognised so it looks like "REG_EXTENDED" better correlates with my ideas
  // of whar a regex is than the default. This also means [[:space:]] works.
  const int cflags = REG_EXTENDED;

  posix_lexer* lexer = malloc(sizeof(posix_lexer) + sizeof(regex_t) * N);
  if (!lexer)
    {
      return lexer_t_failed();
    }
  set_engine(lexer);
  lexer->size = N;
  regex_t* compiled = &lexer->pats[0];

  for (size_t i = 0; i < N; i++)
    {
      if (regexes[i] == 0) {
        fprintf(stderr, "Error: Regex %zu of %zu is null\n", i, N);
      }
      assert(regexes[i] != 0);
    }

  for (size_t i = 0; i < N; i++)
    {
      int r = regcomp(&compiled[i], regexes[i], cflags);
      if (r == 0)
        {
        }
      else
        {
          char errbuf[64];
          size_t errbuf_size = sizeof(errbuf);
          regerror(r, &compiled[i], errbuf, errbuf_size);
          printf("posix regex compile failed on %s with %s\n", regexes[i],
                 errbuf);

          for (size_t j = 0; j < i; j++)
            {
              regfree(&compiled[i]);
            }
          free(lexer);
          return lexer_t_failed();
        }
    }

  return from_posix(lexer);
}

void lexer_posix_destroy(lexer_t lex)
{
  posix_lexer* lexer = to_posix(lex);
  regex_t* compiled = &lexer->pats[0];
  size_t N = lexer->size;
  for (size_t i = 0; i < N; i++)
    {
      regfree(&compiled[i]);
    }
  free(lexer);
}

bool lexer_posix_valid(lexer_t lex) { return to_posix(lex) != 0; }

static size_t ith_regex_matches_start(posix_lexer* lexer, const char* start,
                                      const char* end, size_t i)
{
  (void)end;

  regex_t* re = &lexer->pats[i];
  const int eflags = 0;

  regmatch_t match[1];

  int status = regexec(re, start, 1, match, eflags);
  if (status == 0)  // matched
    {
      if (match[0].rm_so == 0)  // matched at the start
        {
          return match[0].rm_eo;
        }
    }

  return 0;
}

lexer_token_t lexer_posix_iterator_step(lexer_t l, lexer_iterator_t* iter)
{
  assert(iter);
  assert(!lexer_iterator_t_empty(*iter));
  assert(lexer_posix_valid(l));

  posix_lexer* lexer = to_posix(l);
  size_t N = lexer->size;
  assert(N > 0);
  // Zeroth regex matches anything and returns width one
  bool zeroth_matches = ith_regex_matches_start(lexer, iter->cursor, iter->end, 0) == 1;
  if (!zeroth_matches) {
    size_t width = iter->end - iter->cursor;
    printf("Internal error. Zeroth regex did not match %s 0x%x. Iterator width %zu.\n", iter->cursor, (int)iter->cursor[0], width);
    if (width > 8) width = 8;
    for (size_t i = 0; i < width; i++) {
      printf("Char [%zu] = 0x%x (%c) \n", i, iter->cursor[i], iter->cursor[i]);
    }
  }
  assert(zeroth_matches);

  // Looking for longest match wins, with earliest match on tie break
  lexer_token_t result = {
    .id = 0,
    .value = iter->cursor,
    .width = 1,
  };
    
  for (size_t i = 1; i < N; i++)
    {
      size_t w = ith_regex_matches_start(lexer, iter->cursor, iter->end, i);
      if (w == 0) continue; // no match

      bool is_first_match = result.id == 0;
      bool is_longer_match = w > result.width;
      if (is_first_match | is_longer_match) {
        result.id = i;
        result.width = w;
      }
    }
  
  assert(result.value == iter->cursor);
  iter->cursor += result.width;
  return result;
}
