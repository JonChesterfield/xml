#ifndef ARITH_PARSE_H_INCLUDED
#define ARITH_PARSE_H_INCLUDED


// Thinking of defining an interface here which is implemented
// in terms of lemon, bison or both

// TODO: Rename this yyParser type to be more obviously lemon
// extern struct yyParser arith_global_lemon_parser;

#include "../tools/token.h"
#include "../tools/list.h"

// It's built on global state at present. Otherwise needs a
// stack-or-otherwise allocated variable, where the type is 



// Opaque type
union arith_parser_u;
typedef union arith_parser_u arith_parser_type ;
enum {arith_parser_type_size = 4032, arith_parser_type_align = 8};
struct arith_parser_s {
    char _Alignas(arith_parser_type_align) data[arith_parser_type_size];
};


void arith_parser_initialize(arith_parser_type*);
void arith_parser_finalize(arith_parser_type*);

void arith_parser_parse(arith_parser_type*, int, token); // maybe implicit init
list arith_parser_tree(arith_parser_type*); // maybe fold with finalize



#endif
