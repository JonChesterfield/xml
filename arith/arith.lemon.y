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
  
#include "../tools/token.h"
#include "../tools/list.h"
#endif

#include "arith.lemon.h"

}

%name arith_Lemon
%start_symbol program // explicit
%type expr { list }
%extra_argument {list* external_context}

%syntax_error { fprintf(stderr, "Syntax error\n"); }

// This is whatever object the lexer thinks a token should be
// I don't think the parser is likely to want to mutate it


// Tokens
%token_type { token }
%token_prefix TOKEN_ID_

%token PLUS MINUS TIMES DIVIDE MODULO.
%token LPAREN RPAREN INTEGER.
%token WHITESPACE.

// Precedences
%left PLUS MINUS.
%left TIMES DIVIDE MODULO.
%nonassoc WHITESPACE.

// Current thinking is to provide a different interface implemented
// in terms of the lemon one, much like the lexer interface is
// implemented on top of some regex engine
// Maybe also therefore generate a header/source pair for data
// descriptions (the regex/literal strings, enums for terminals
// and non-terminals etc)
// Can give the terminals names as well, PLUS(D) etc

program ::= expr(A). {
   printf("Invoking program\n");
   *external_context = A;
   printf("\n");
}

expr(A) ::= expr(B) PLUS expr(C). {
  A = list_make_uninitialised("BinOpPlus", 2);
  A.elts[0] = B;
  A.elts[1] = C;
}
  
expr(A) ::= expr(B) MINUS expr(C). {
  A = list_make_uninitialised("BinOpMinus", 2);
  A.elts[0] = B;
  A.elts[1] = C;
}
  
expr(A) ::= expr(B) TIMES expr(C). {
  A = list_make_uninitialised("BinOpTimes", 2);
  A.elts[0] = B;
  A.elts[1] = C;
}

expr(A) ::= expr(B) DIVIDE expr(C). {
  A = list_make_uninitialised("BinOpDivide", 2);
  A.elts[0] = B;
  A.elts[1] = C;
} 

expr(A) ::= expr(B) MODULO expr(C).
{
  A = list_make_uninitialised("BinOpModulo", 2);
  A.elts[0] = B;
  A.elts[1] = C;
}

expr(A) ::= LPAREN expr(B) RPAREN.
{
  A = list_make_uninitialised("Parenthesised", 1);
  A.elts[0] = B;
}

expr(A) ::= INTEGER(B).
{
  A = list_from_token(B);
}

// Using this as a hack to check precedence rules in the lexer
expr(A) ::= ONE(B).
{
  A = list_from_token(B);
}
  
// Fun. Can discard whitespace in the parse instead of in the lexer.
expr(A) ::= WHITESPACE expr(B). { A = B; }
expr(A) ::= expr(B) WHITESPACE. { A = B; }

// This is not equivalent to the above two rules - syntax error - but not sure why
// expr ::= WHITESPACE.

