#include <cassert>
#include <re2/re2.h>
#include <re2/set.h>
#include <vector>
#include <cstdlib>

#include "token.h"
#include "lexer.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "parse.h"

#ifdef __cplusplus
}
#endif


// lemon will let one add tokens that aren't used
// by the parser, and specify a prefix, but changing it
// to an enum trips up makeheader

// Lemon's assigned integers start at 1, can control the order

// the tables are relatively easily conjured from XML definitions
// the lexer can be templated on said tables

enum { TOKEN_ID_UNKNOWN = 0 };

const char *token_names[] = {
    [TOKEN_ID_UNKNOWN] = "TOKEN_ID_UNKNOWN",
    [TOKEN_ID_PLUS] = "TOKEN_ID_PLUS",
    [TOKEN_ID_MINUS] = "TOKEN_ID_MINUS",
    [TOKEN_ID_TIMES] = "TOKEN_ID_TIMES",
    [TOKEN_ID_DIVIDE] = "TOKEN_ID_DIVIDE",
    [TOKEN_ID_LPAR] = "TOKEN_ID_LPAR",
    [TOKEN_ID_RPAR] = "TOKEN_ID_RPAR",
    [TOKEN_ID_INTEGER] = "TOKEN_ID_INTEGER",
    [TOKEN_ID_WHITESPACE] = "TOKEN_ID_WHITESPACE",
};
enum { token_names_size = sizeof(token_names) / sizeof(token_names[0]) };

const char *regexes[] = {
    [TOKEN_ID_UNKNOWN] = ".",
    [TOKEN_ID_PLUS] = R"([+])",
    [TOKEN_ID_MINUS] = R"([-])",
    [TOKEN_ID_TIMES] = R"([*])",
    [TOKEN_ID_DIVIDE] = R"([/])",
    [TOKEN_ID_LPAR] = R"(\()",
    [TOKEN_ID_RPAR] = R"(\))",
    [TOKEN_ID_INTEGER] = R"(0|-*[1-9]+[0-9]*)",
    [TOKEN_ID_WHITESPACE] = R"([ \n\t\r\v]+)",
};
enum { regexes_size = sizeof(regexes) / sizeof(regexes[0]) };

static_assert((size_t)regexes_size == (size_t)token_names_size, "");

struct kill {
    kill(lexer_state &s) : s(s) {}
    lexer_state & s;
    ~kill() {lexer_destroy(s);}
};

int main() {
  const bool verbose = false;
  const char *example = " 10 + 2 * (4 /\t2)    - 1 ";

  re2::StringPiece cursor(example);

  lexer_state state = lexer_create(regexes_size, token_names, regexes);
  if (!lexer_success(state)) { return 1; }
  kill k_(state);


  void*    pParser = (void *) ParseAlloc(malloc);
  

  // ParseTrace(stdout, "");
  
  printf("Input %s\n", example);

  const char *sep = "";
  while (!cursor.empty()) {
    token tok = lexer_next(state, cursor.data(), cursor.data() + cursor.size());
    assert(!token_empty(tok));
    cursor.remove_prefix(token_width(tok));
    
    if (tok.name == TOKEN_ID_WHITESPACE) {
      // printf("re2: whitespace\n");
      continue;
    }

    if (tok.name == TOKEN_ID_INTEGER) {
      // lexer no longer does int->num, that's the parsers problem
      // printf("re2: integer "); token_dump(tok);
      Parse(pParser, tok.name, tok);
      continue;
    }

    // printf("re2: other "); token_dump(tok);
    Parse(pParser, tok.name, tok);
  }

  Parse(pParser, 0, token_create_novalue(0));
  ParseFree(pParser, free);

  return 0;
}
