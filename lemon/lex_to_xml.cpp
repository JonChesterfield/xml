#include "lexer_instance.hpp"
#include <vector>
#include <cassert>

enum { TOKEN_ID_UNKNOWN = 0 };

const char *token_names[] = {
    [TOKEN_ID_UNKNOWN] = "TOKEN_ID_UNKNOWN",
    [TOKEN_ID_PLUS] = "TOKEN_ID_PLUS",
    [TOKEN_ID_MINUS] = "TOKEN_ID_MINUS",
    [TOKEN_ID_TIMES] = "TOKEN_ID_TIMES",
    [TOKEN_ID_DIVIDE] = "TOKEN_ID_DIVIDE",
    [TOKEN_ID_MODULO] = "TOKEN_ID_MODULO",
    [TOKEN_ID_LPAREN] = "TOKEN_ID_LPAREN",
    [TOKEN_ID_RPAREN] = "TOKEN_ID_RPAREN",
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
    [TOKEN_ID_MODULO] = R"([%])",
    [TOKEN_ID_LPAREN] = R"(\()",
    [TOKEN_ID_RPAREN] = R"(\))",
    [TOKEN_ID_INTEGER] = R"(0|-*[1-9]+[0-9]*)",
    [TOKEN_ID_WHITESPACE] = R"([ \n\t\r\v]+)",
};
enum { regexes_size = sizeof(regexes) / sizeof(regexes[0]) };

static_assert((size_t)regexes_size == (size_t)token_names_size, "");

using LexerType = lexer_instance<regexes_size, token_names, regexes>;

std::vector<char> round_trip(const char * src)
{
  std::vector<char> res;
  LexerType lex (src);
  while (lex) {
    token n = lex.next();
    for (size_t i = 0; i < token_width(n); i++) {
      res.push_back(n.value_start[i]);
    }

    token_named_dump(n, token_names);
  }
  return res;
}


int main() {
  const bool verbose = false;
  const char *example = " (10 + 2 * (4 /\t2)    - 1 % 12)";

  size_t N = strlen(example);
  auto r = round_trip(example);

  assert(r.size() == N);
  for (size_t i = 0; i < N; i++) {
    assert(example[i] == r[i]);
  }

}

