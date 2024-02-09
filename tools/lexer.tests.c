#include "lexer.h"

#include "../../../evilunit/EvilUnit.h"

typedef struct
{
  const char * seq;
  size_t index;
} token;

// Current thinking is a test case that looks roughly like the following.
// a working lexer will cut up a byte string into tokens that can be reassembled
// into an equal byte string, but at least some tests should also check that the
// tokens have the right integer and string.
// Probably need a bytevector implementation to write that sanely, or to generate
// the cases from another language.

// Also needs a set of regex.
// Inclined to think writing the tests relative to a specific language makes more
// sense, e.g. the arithmetic language, since the test setup needs a regex sequence
// anyway.
//
// Testing against multiple regex backends is probably a good idea too.

enum {PLUS, MINUS, INTEGER, WHITESPACE };
token example[] = {
  {"1", INTEGER},
  {"+", PLUS},
  {"  ", WHITESPACE},
  {"2", INTEGER},
};


MAIN_MODULE()
{
  CHECK(1);
}
