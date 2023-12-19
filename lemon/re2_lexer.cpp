#include <cassert>
#include <re2/re2.h>
#include <re2/set.h>
#include <vector>
#include <cstdlib>

#include "lexer.h"


// This is kind of a bug in makeheaders interacting with lemon
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
#endif

#include "types.h"

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
  

    if (verbose) {
      printf("Input %s\n", example);
      printf("Lexed {\n");
    }

  const char *sep = "";
  while (!cursor.empty()) {
    lexer_token token = lexer_next(state, cursor.data(), cursor.data() + cursor.size());
    assert(!lexer_token_empty(token));
    cursor.remove_prefix (lexer_token_width(token));
    
    const char * tag = token_names[token.name];
    std::string var(token.value_start, token.value_end);

    if (token.name == TOKEN_ID_WHITESPACE) {
      continue;
    }

    if (token.name == TOKEN_ID_INTEGER) {
      int value = 42;
      SToken tok = {value, .token = var.c_str()};
      Parse(pParser, (int)token.name, &tok);
      continue;
    }
    
    if (verbose) {
      printf("%s  {%-25s |%s| }", sep, tag, var.c_str());
      sep = ",\n";
    }

    // The printing uses %s so this is less confusing

    Parse(pParser, (int)token.name, NULL);
  }

  if (verbose) {
    printf(",\n}\n");
  }

  Parse(pParser, 0, NULL);
  ParseFree(pParser, free);

  return 0;
}
