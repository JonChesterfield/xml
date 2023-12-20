#ifndef LEX_THEN_PARSE_HPP_INCLUDED
#define LEX_THEN_PARSE_HPP_INCLUDED

#include "lexer_instance.hpp"
extern "C" {
#include "parse.h"
}

// e.g. LexerType = lexer_instance<regexes_size, token_names, regexes>
template <typename LexerType> int lex_then_parse(const char *input, size_t N) {

  auto lexer = LexerType(input, N);
  if (!lexer) {
    return 1;
  }

  void *pParser = (void *)ParseAlloc(malloc);

  while (lexer) {
    token tok = lexer.next();
    if (token_empty(tok)) {
      return 2;
    }

    // This is the plan for all tokens. Whitespace is dealt with in the parser.
    Parse(pParser, tok.name, tok);
  }

  Parse(pParser, 0, token_create_novalue(0));

  ParseFree(pParser, free);

  return 0;
}

#endif
