// Not a normal header file

#include <stdlib.h>

// This file needs to be instantiated with a language prefix
#ifndef LEXER_LANGUAGE
#error "Require macro LEXER_LANGUAGE"
#endif

#define LEXER_PASTE2(X, Y) X##Y
#define LEXER_PASTE(X, Y) LEXER_PASTE2(X, Y)

lexer_t LEXER_PASTE(LEXER_LANGUAGE, _lexer_multi_create)(void);
void LEXER_PASTE(LEXER_LANGUAGE, _lexer_multi_destroy)(lexer_t lex);
bool LEXER_PASTE(LEXER_LANGUAGE, _lexer_multi_valid)(lexer_t lex);

lexer_token_t LEXER_PASTE(LEXER_LANGUAGE,
                          _lexer_multi_iterator_step)(lexer_t l,
                                                      lexer_iterator_t *iter);

#ifndef LEXER_POSIX_ENABLE
#error "Require macro LEXER_POSIX_ENABLE"
#endif

#ifndef LEXER_RE2_ENABLE
#error "Require macro LEXER_RE2_ENABLE"
#endif

#ifndef LEXER_RE2C_ENABLE
#error "Require macro LEXER_RE2C_ENABLE"
#endif

#define LEXER_MULTI_COUNT()                                                    \
  (LEXER_POSIX_ENABLE ? 1 : 0) + (LEXER_RE2_ENABLE ? 1 : 0) +                  \
      (LEXER_RE2C_ENABLE ? 1 : 0)

#if LEXER_MULTI_COUNT() == 0
#error "Require at least one lexer to compose into a multi lexer"
#endif

#if LEXER_POSIX_ENABLE
#include "lexer.posix.h"
#endif

#if LEXER_RE2_ENABLE
#include "lexer.re2.h"
#endif

#if LEXER_RE2C_ENABLE
#include "lexer.re2c.h"
#endif

typedef struct {
  enum lexer_engines engine;
#if LEXER_POSIX_ENABLE
  lexer_t posix;
#endif
#if LEXER_RE2_ENABLE
  lexer_t re2;
#endif
#if LEXER_RE2C_ENABLE
  lexer_t re2c;
#endif
} lexer_multi_t;

static const char * lexer_multi_components[LEXER_MULTI_COUNT()] = {
#if LEXER_POSIX_ENABLE
  "posix",
#endif
#if LEXER_RE2_ENABLE
  "re2",
#endif
#if LEXER_RE2C_ENABLE
  "re2c",
#endif
};

lexer_t LEXER_PASTE(LEXER_LANGUAGE, _lexer_multi_create)(void) {
  lexer_multi_t *multi = malloc(sizeof(lexer_multi_t));
  lexer_t result = {
      .data = multi,
  };
  if (!multi) {
    return result;
  }
  multi->engine = lexer_engines_multi;

#if LEXER_POSIX_ENABLE
  multi->posix = LEXER_PASTE(LEXER_LANGUAGE, _lexer_posix_create)();
#endif
#if LEXER_RE2_ENABLE
  multi->re2 = LEXER_PASTE(LEXER_LANGUAGE, _lexer_re2_create)();
#endif
#if LEXER_RE2C_ENABLE
  multi->re2c = LEXER_PASTE(LEXER_LANGUAGE, _lexer_re2c_create)();
#endif

  return result;
}

void LEXER_PASTE(LEXER_LANGUAGE, _lexer_multi_destroy)(lexer_t lex) {
  lexer_multi_t *multi = lex.data;
  if (!multi) {
    return;
  }
  assert(multi->engine == lexer_engines_multi);

#if LEXER_POSIX_ENABLE
  LEXER_PASTE(LEXER_LANGUAGE, _lexer_posix_destroy)(multi->posix);
#endif
#if LEXER_RE2_ENABLE
  LEXER_PASTE(LEXER_LANGUAGE, _lexer_re2_destroy)(multi->re2);
#endif
#if LEXER_RE2C_ENABLE
  LEXER_PASTE(LEXER_LANGUAGE, _lexer_re2c_destroy)(multi->re2c);
#endif
}

bool LEXER_PASTE(LEXER_LANGUAGE, _lexer_multi_valid)(lexer_t lex) {
  lexer_multi_t *multi = lex.data;
  if (!multi) {
    return false;
  }
  assert(multi->engine == lexer_engines_multi);
  bool valid = true;

#if LEXER_POSIX_ENABLE
  valid &= LEXER_PASTE(LEXER_LANGUAGE, _lexer_posix_valid)(multi->posix);
#endif
#if LEXER_RE2_ENABLE
  valid &= LEXER_PASTE(LEXER_LANGUAGE, _lexer_re2_valid)(multi->re2);
#endif
#if LEXER_RE2C_ENABLE
  valid &= LEXER_PASTE(LEXER_LANGUAGE, _lexer_re2c_valid)(multi->re2c);
#endif

  return valid;
}

lexer_token_t LEXER_PASTE(LEXER_LANGUAGE,
                          _lexer_multi_iterator_step)(lexer_t l,
                                                      lexer_iterator_t *iter) {
  enum { N = LEXER_MULTI_COUNT() };
  assert(LEXER_PASTE(LEXER_LANGUAGE, _lexer_multi_valid)(l));
  lexer_multi_t *multi = l.data;
  assert(multi);
  assert(multi->engine == lexer_engines_multi);
  lexer_iterator_t iters[N];
  lexer_token_t tokens[N];
  for (unsigned i = 0; i < N; i++) {
    iters[i] = *iter;
  }
  unsigned i = 0;

#if LEXER_POSIX_ENABLE
  tokens[i] = LEXER_PASTE(LEXER_LANGUAGE,
                          _lexer_posix_iterator_step)(multi->posix, &iters[i]);
  i++;
#endif
#if LEXER_RE2_ENABLE
  tokens[i] = LEXER_PASTE(LEXER_LANGUAGE, _lexer_re2_iterator_step)(multi->re2,
                                                                    &iters[i]);
  i++;
#endif
#if LEXER_RE2C_ENABLE
  tokens[i] = LEXER_PASTE(LEXER_LANGUAGE,
                          _lexer_re2c_iterator_step)(multi->re2c, &iters[i]);
  i++;
#endif

  for (unsigned i = 1; i < N; i++) {
    bool iter_cursor_ok = iters[i].cursor == iters[0].cursor;
    bool iter_end_ok = iters[i].end == iters[0].end;

    bool token_id_ok = tokens[i].id == tokens[0].id;
    bool token_value_ok = tokens[i].value == tokens[0].value;
    bool token_width_ok = tokens[i].width == tokens[0].width;

    bool all_ok = iter_cursor_ok & iter_end_ok & token_id_ok & token_value_ok &
                  token_width_ok;

    if (!all_ok) {
      fprintf(stdout, "Inconsistent lexer %s [%u %u %u %u %u]\n",lexer_multi_components[i],iter_cursor_ok,iter_end_ok,token_id_ok,token_value_ok,token_width_ok);
      lexer_token_dump(tokens[0]);
      lexer_token_dump(tokens[i]);      
    }
  }

  // first enabled one wins on mismatch
  *iter = iters[0];
  return tokens[0];
}
