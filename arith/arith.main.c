#include "arith.parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  int value;
  void *pParser;
  const char *c;
  int i = 0;

  if (2 > argc) {
    printf("Usage: %s <expression>\n", argv[0]);
    return 1;
  }

  pParser = (void *)ParseAlloc(malloc);
  for (i = 1; i < argc; ++i) {
    c = argv[i];

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
      token tok = token_create(
          "INTEGER",
           c,
          strlen(c));
      
      Parse(pParser, TOKEN_ID_INTEGER, tok);
      break;
    }

    case '+': {
      token tok = token_create_novalue("PLUS");
      Parse(pParser, TOKEN_ID_PLUS, tok);
      break;
    }
    case '-': {
      token tok = token_create_novalue("MINUS");
      Parse(pParser, TOKEN_ID_MINUS, tok);
      break;
    }
    case '*': {
      token tok = token_create_novalue("TIMES");
      Parse(pParser, TOKEN_ID_TIMES, tok);
      break;
    }

    case '/': {
      token tok = token_create_novalue("DIVIDE");
      Parse(pParser, TOKEN_ID_DIVIDE, tok);
      break;
    }
    case '(': {
      token tok = token_create_novalue("LPAREN");
      Parse(pParser, TOKEN_ID_LPAREN, tok);
      break;
    }

    case ')': {
      token tok = token_create_novalue("RPAREN");
      Parse(pParser, TOKEN_ID_RPAREN, tok);
      break;
    }

    default:
      fprintf(stderr, "Unexpected token %s\n", c);
    }
  }
  {
    token tok = token_create_novalue("");
    Parse(pParser, 0, tok);
  }
  ParseFree(pParser, free);

  return 0;
}
