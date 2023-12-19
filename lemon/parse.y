// From https://stackoverflow.com/questions/34918631/use-lemon-parserlalr-generate-a-calulator-how-to-get-param-from-expressions

%include
{

#include <assert.h>
#include <stdio.h> // parsetrace uses FILE
#include <stdlib.h>

#if INTERFACE
// Things to inject into makeheaders exported interface
#ifndef YYMALLOCARGTYPE
#define YYMALLOCARGTYPE size_t
#endif
  
#include "token.h"
#endif


#include "parse.h"
  
}

%syntax_error { fprintf(stderr, "Syntax error\n"); }

// This is whatever object the lexer thinks a token should be
// I don't think the parser is likely to want to mutate it

%token_type { token }

%token_prefix    TOKEN_ID_
%token PLUS MINUS TIMES DIVIDE.
%token LPAR RPAR INTEGER.

%token WHITESPACE.
// Maybe whitespace should have a parse rule for throw away

%type expr { int }

%left PLUS MINUS.
%left TIMES DIVIDE.

program ::= expr(A). { printf("Result = %d\n", A); }
expr(A) ::= expr(B) PLUS expr(C). {A = B + C; printf("%d = %d + %d\n", A, B, C); }
expr(A) ::= expr(B) MINUS expr(C). {A = B - C;  printf("%d = %d - %d\n", A, B, C);}
expr(A) ::= expr(B) TIMES expr(C). {A = B * C;  printf("%d = %d * %d\n", A, B, C);}
expr(A) ::= expr(B) DIVIDE expr(C).
{
  printf("%d = %d / %d\n", A, B, C);
  if (C != 0)
    {
      A = B / C;
    }
  else
    {
      fprintf(stderr, "divide by 0\n");
      A = 0;
    }
}
expr(A) ::= LPAR expr(B) RPAR. { A = B; printf("(%d = %d)\n", A, B); }

expr(A) ::= INTEGER(B).
{
  // B is a token
  // A is a int
  // printf("IntegerA on %s\n", B->value_start);
  const char* value_start = B.value_start;
  const char* value_end = B.value_end;
  size_t N = value_end - value_start;
  enum {limit = 10+1};
  char buf[limit] = {'\0'};
  if (N >= limit) {
    fprintf(stderr, "Too large\n");
    A = 0;
  } else {
    assert(N < limit);
    for (size_t i = 0; i < N; i++)
      {
        buf[i] = value_start[i];
      }
    // printf("IntegerB on %s\n", buf);
    
    A = atoi(buf);
  }
}
