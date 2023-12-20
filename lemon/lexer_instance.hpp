#ifndef LEXER_INSTANCE_HPP_INCLUDED
#define LEXER_INSTANCE_HPP_INCLUDED

#include <cstring>

extern "C" {
#include "lexer.h"
#include "token.h"
}

template <size_t N, const char *(&token_names)[N], const char *(&regexes)[N]>
struct lexer_instance {

  lexer_instance(const char *start, const char *end)
      : state(lexer_create(N, token_names, regexes)), start(start), end(end) {
    if (!lexer_success(state)) {
      start = end;
    }
  }

  lexer_instance(const char *start) : lexer_instance(start, strlen(start)) {}
  lexer_instance(const char *start, size_t len)
      : lexer_instance(start, start + len) {}

  explicit operator bool() const { return start != end; }

  token next() {
    token res = lexer_next(state, start, end);
    start += token_width(res);
    return res;
  }

  ~lexer_instance() { lexer_destroy(state); }

private:
  lexer_state state;
  const char *start;
  const char *end;
};

#endif
