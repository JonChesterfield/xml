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

// Manual header should be consistent with this one
#include "arith.parse.h"
  
// Removes LemonAlloc/LemonFree
#define arith_Lemon_ENGINEALWAYSONSTACK 1


 
#endif

// todo: make lemon.c self contained
#include "arith.lemon.h"

}

%code
{
  // Exposing the actual type is a nuisance.
  // However I suspect having a single instance is useful in itself.
  // That's sufficient to define the ALWAYSONSTACK macro which removes
  // the alloc/free interface.
  //  struct yyParser arith_global_lemon_parser;


  _Static_assert(sizeof(struct yyParser) == arith_parser_type_size,"");
  _Static_assert(_Alignof(struct yyParser) == arith_parser_type_align,"");

  _Static_assert(sizeof(struct arith_parser_s) == arith_parser_type_size,"");
  _Static_assert(_Alignof(struct arith_parser_s) == arith_parser_type_align,"");

  union arith_parser_u
  {
    struct yyParser lemon;
    struct arith_parser_s state;
  };

  _Static_assert(sizeof(union arith_parser_u) == arith_parser_type_size,"");
  _Static_assert(_Alignof(union arith_parser_u) == arith_parser_type_align,"");

  void arith_parser_initialize(arith_parser_type*p)
  {
    arith_LemonInit(&p->lemon);
  }
  
  void arith_parser_finalize(arith_parser_type*p)
  {
    arith_LemonFinalize(&p->lemon);
  }

  void arith_parser_parse(arith_parser_type*p, int id, token t)
  {
    assert(id > 0);
    arith_Lemon(&p->lemon, id, t, 0);
  }

  list arith_parser_tree(arith_parser_type*p)
  {
    list tmp;
    token tok = token_create_novalue("");
    arith_Lemon(&p->lemon, 0, tok, &tmp);
    return tmp;
  }

}

%name arith_Lemon
%start_symbol program // explicit
%type expr { list }
%type secondexpr { list }

%extra_argument {list* external_context}

%syntax_error { fprintf(stderr, "Syntax error\n"); }

// This is whatever object the lexer thinks a token should be
// I don't think the parser is likely to want to mutate it


// Tokens
%token_type { token }
%token_prefix TOKEN_ID_

 // Listing the tokens in the same order the lexer defines them means
 // the integer ids lemon uses are equal to those the lexer uses
%token PLUS.
%token MINUS.
%token TIMES.
%token DIVIDE.
%token MODULO.
%token LPAREN.
%token RPAREN.
%token One.
%token INTEGER.
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


// These would be easier to read/debug in the generated output if they
// were all typed functions that lemon merely builds the dispatch to

%include
{
  list PLUS_ctor(list B, list C)
  {
    list A = list_make_uninitialised("BinOpPlus", 2);
    A.elts[0] = B;
    A.elts[1] = C;
    return A;
  }
}

// BinopPlus
// 
expr(A) ::= expr(B) PLUS expr(C). {
  #if 0
  A = PLUS_ctor(B, C);
  #else
  A = list_make_uninitialised("BinOpPlus", 2);
  A.elts[0] = B;
  A.elts[1] = C;
  #endif
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

  
// Fun. Can discard whitespace in the parse instead of in the lexer.
expr(A) ::= WHITESPACE expr(B). { A = B; }
expr(A) ::= expr(B) WHITESPACE. { A = B; }

// This is not equivalent to the above two rules - syntax error - but not sure why
// expr ::= WHITESPACE.

