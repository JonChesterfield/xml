#include "lexer.interp.h"
#include "../regex/regex_interpreter.h"

#include <assert.h>
#include <string.h>

typedef struct
{
#ifndef NDEBUG
  enum lexer_engines engine;
#endif
  regex_cache_t cache;
  size_t size;
  stringtable_index_t regexes[]; // size
} interp_lexer;

static void set_engine(interp_lexer* l)
{
#ifndef NDEBUG
  l->engine = lexer_engines_interp;
#else
  (void)l;
#endif
}

static void check_engine(interp_lexer* l)
{
#ifndef NDEBUG
  if (l != NULL)
    {
      unsigned num = l->engine;
      if (num != lexer_engines_interp)
        {
          fprintf(stderr,
                  "Invalid magic number (%u) for lexer.interp, aborting\n", num);
          abort();
        }
    }
#else
  (void)l;
#endif
}

static lexer_t from_interp(interp_lexer* l)
{
  check_engine(l);
  return (lexer_t){.data = (void*)l};
}

static interp_lexer* to_interp(lexer_t l)
{
  interp_lexer* lexer = (interp_lexer*)l.data;
  check_engine(lexer);
  return lexer;
}

static lexer_t lexer_t_failed(void) { return from_interp(0); }

lexer_t lexer_interp_create(size_t N, const char** regexes)
{
  const bool verbose = true;
  
  // Puts the top level regex into the interpreter but does not differentiate
  // any of them. This suffices to check the inputs are considered well formed
  // (and thus their derivative is expected to be computable successfully)
  // without an up front construction of all possible tables.
  
  interp_lexer* lexer = malloc(sizeof(interp_lexer) + sizeof(stringtable_index_t) * N);
  if (!lexer)
    {
      return lexer_t_failed();
    }
  set_engine(lexer);

  lexer->cache = regex_cache_create();
  if (!regex_cache_valid(lexer->cache)) { free(lexer); return lexer_t_failed(); }

  lexer->size = N;
  bool regex_ok = true;
  for (size_t i = 0; i < N; i++)
    {
      const char * r = regexes[i];
      if (r == 0) {
        regex_ok = false;        
      } else {
        size_t N = strlen(r);
        stringtable_index_t current =
          regex_cache_insert_regex_bytes(&lexer->cache, r, N);
        lexer->regexes[i] = current;
        if (!stringtable_index_valid(current)) {
          if (verbose) {
            printf("lexer.interp rejecting regex %zu %s\n", i, r);
          }
          regex_ok = false;
        }
      }
    }

  if (!regex_ok) {
    lexer_interp_destroy(from_interp(lexer));
    return lexer_t_failed();    
  }

  return from_interp(lexer);
}


void lexer_interp_destroy(lexer_t lex)
{
  interp_lexer* lexer = to_interp(lex);
  if (lexer)
    {
      regex_cache_destroy(lexer->cache);
      free(lexer);
    }
}

bool lexer_interp_valid(lexer_t lex) {
  interp_lexer* lexer = to_interp(lex);
  return (lexer!=0) && regex_cache_valid(lexer->cache);
}

static size_t ith_regex_matches_start(interp_lexer* lexer, const char* start,
                                      const char* end, size_t i)
{
  (void)end;
  uint64_t res = regex_interpreter_known_regex_matches(&lexer->cache,
                                                       lexer->regexes[i],
                                                       (const unsigned char*)start,
                                                       end-start);
  if (regex_interpreter_success(res)) {
    return res;
  } else {
    return 0;
  }   
}


// This is identical to lexer_posix_iterator_step, moduloe the regex matches start
// and the lexer type. should factor that out
lexer_token_t lexer_interp_iterator_step(lexer_t lex, lexer_iterator_t* iter)
{
  assert(iter);
  assert(!lexer_iterator_t_empty(*iter));
  assert(lexer_interp_valid(lex));

  interp_lexer* lexer = to_interp(lex);

  // Zeroth regex matches anything and returns width one
  assert(ith_regex_matches_start(lexer, iter->cursor, iter->end, 0) == 1);

  // Looking for longest match wins, with earliest match on tie break
  lexer_token_t result = {
    .id = 0,
    .value = iter->cursor,
    .width = 1,
  };

  size_t N = lexer->size;
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
