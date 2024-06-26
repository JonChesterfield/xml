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

  static int DIVIDE(int B, int C)
  {
    int A;
    if (C != 0)
      {
        A = B / C;
      }
    else
      {
        fprintf(stderr, "divide by 0\n");
        A = 0;
      }
    printf("%d = %d / %d\n", A, B, C);
    return A;
  }

  static int INTEGER(token B)
  {
    int A;
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
    return A;
  }
}

%syntax_error { fprintf(stderr, "Syntax error\n"); }

// This is whatever object the lexer thinks a token should be
// I don't think the parser is likely to want to mutate it

%token_type { token }

%token_prefix    TOKEN_ID_
%token PLUS MINUS TIMES DIVIDE MODULO.
%token LPAREN RPAREN INTEGER.

%token WHITESPACE.
// Maybe whitespace should have a parse rule for throw away

%type expr { int }

%left PLUS MINUS.
%left TIMES DIVIDE MODULO.

%nonassoc WHITESPACE.

program ::= expr(A). { printf("Result = %d\n", A); }
expr(A) ::= expr(B) PLUS expr(C). {A = B + C; printf("%d = %d + %d\n", A, B, C); }
expr(A) ::= expr(B) MINUS expr(C). {A = B - C;  printf("%d = %d - %d\n", A, B, C);}
expr(A) ::= expr(B) TIMES expr(C). {A = B * C;  printf("%d = %d * %d\n", A, B, C);}
expr(A) ::= expr(B) DIVIDE expr(C). {A = DIVIDE(B, C);} 

// Can give the terminals names as well, so this is essentially:
// token function(int B, token D, int C)
// where a terminal (or expr) with a name is discarded
expr(A) ::= expr(B) MODULO(D) expr(C).
{
    A = B % C;
    ParseTOKENTYPE thing = D;
    printf("%d = %d %% %d\n", A, B, C);
}

expr(A) ::= LPAREN expr(B) RPAREN. { A = B; printf("(%d = %d)\n", A, B); }


// Fun. Can discard whitespace in the parse instead of in the lexer.
expr(A) ::= WHITESPACE expr(B). { A = B; }
expr(A) ::= expr(B) WHITESPACE. { A = B; }

// This might be equivalent to the above two rules
// expr ::= WHITESPACE.

expr(A) ::= INTEGER(B).
{
  // B is a token
  // A is a int
  A = INTEGER(B);

}
