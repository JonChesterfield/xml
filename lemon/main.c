#include "parse.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  int value;
  void *pParser;
  const char *c;
  size_t i = 0;
  struct SToken v[argc];

  if (2 > argc) {
    printf("Usage: %s <expression>\n", argv[0]);
    return 1;
  }

  pParser = (void *)ParseAlloc(malloc);
  for (i = 1; i < argc; ++i) {
    c = argv[i];
    v[i].token = c;
    switch (*c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      for (value = 0; *c && *c >= '0' && *c <= '9'; c++)
        value = value * 10 + (*c - '0');
      v[i].value = value;
      token tok = {
          .name = TOKEN_ID_INTEGER,
          .value_start = c,
          .value_end = c + strlen(c),
      };
      Parse(pParser, TOKEN_ID_INTEGER, tok);
      break;
    }

    case '+': {
      token tok = token_create_novalue(TOKEN_ID_PLUS);
      Parse(pParser, TOKEN_ID_PLUS, tok);
      break;
    }
    case '-': {
      token tok = token_create_novalue(TOKEN_ID_MINUS);
      Parse(pParser, TOKEN_ID_MINUS, tok);
      break;
    }
    case '*': {
      token tok = token_create_novalue(TOKEN_ID_TIMES);
      Parse(pParser, TOKEN_ID_TIMES, tok);
      break;
    }

    case '/': {
      token tok = token_create_novalue(TOKEN_ID_DIVIDE);
      Parse(pParser, TOKEN_ID_DIVIDE, tok);
      break;
    }
    case '(': {
      token tok = token_create_novalue(TOKEN_ID_LPAREN);
      Parse(pParser, TOKEN_ID_LPAREN, tok);
      break;
    }

    case ')': {
      token tok = token_create_novalue(TOKEN_ID_RPAREN);
      Parse(pParser, TOKEN_ID_RPAREN, tok);
      break;
    }

    default:
      fprintf(stderr, "Unexpected token %s\n", c);
    }
  }
  {
    token tok = token_create_novalue(0);
    Parse(pParser, 0, tok);
  }
  ParseFree(pParser, free);

  return 0;
}
