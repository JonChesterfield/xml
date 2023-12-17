// From https://stackoverflow.com/questions/34918631/use-lemon-parserlalr-generate-a-calulator-how-to-get-param-from-expressions

%include
{
#include "types.h"

#include "assert.h"
}
%syntax_error { fprintf(stderr, "Syntax error\n"); }
%token_type { struct SToken* }
%type expr { int }
%left PLUS MINUS.
%left TIMES DIVIDE.
program ::= expr(A). { printf("Result = %d\n", A); }
expr(A) ::= expr(B) PLUS expr(C). {A = B + C; }
expr(A) ::= expr(B) MINUS expr(C). {A = B - C; }
expr(A) ::= expr(B) TIMES expr(C). {A = B * C; }
expr(A) ::= expr(B) DIVIDE expr(C).
{
    if (C != 0)
    {
        A = B / C;
    }
    else
    {
        fprintf(stderr, "divide by 0");
    }
}
expr(A) ::= LPAR expr(B) RPAR. { A = B; }
expr(A) ::= INTEGER(B).
{
    A = B->value;
    printf("Passed argument: %s\n", B->token);
}
