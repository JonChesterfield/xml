#include "lexer.re2c.h"

typedef struct
{
#ifndef NDEBUG
  enum lexer_engines engine;
#else
  char unused;
#endif
} re2c_lexer;

static void set_engine(re2c_lexer* l)
{
#ifndef NDEBUG
  l->engine = lexer_engines_re2c;
#else
  (void)l;
#endif
}

static void check_engine(re2c_lexer* l)
{
#ifndef NDEBUG
  if (l != NULL)
    {
      unsigned num = l->engine;
      if (num != lexer_engines_re2c)
        {
          fprintf(stderr,
                  "Invalid magic number (%u) for lexer.re2c, aborting\n", num);
          abort();
        }
    }
#else
  (void)l;
#endif
}

static lexer_t from_re2c(re2c_lexer* l)
{
  check_engine(l);
  return (lexer_t){.data = (void*)l};
}

static re2c_lexer* to_re2c(lexer_t l)
{
  re2c_lexer* lexer = (re2c_lexer*)l.data;
  check_engine(lexer);
  return lexer;
}

lexer_t lexer_re2c_create(void)
{
  static re2c_lexer only = {0};
  set_engine(&only);
  check_engine(&only);
  return from_re2c(&only);
}

void lexer_re2c_destroy(lexer_t l)
{
  re2c_lexer* lexer = to_re2c(l);
  check_engine(lexer);
  (void)lexer;
}

bool lexer_re2c_valid(lexer_t l)
{
  re2c_lexer* lexer = to_re2c(l);
  (void)lexer;
  return true;
}
