#ifndef REGEX_PARSER_LEMON_H_INCLUDED
#define REGEX_PARSER_LEMON_H_INCLUDED

#include "../tools/token.h"
#include "../tools/ptree.h"

struct regex_parser_lemon_state;
typedef struct regex_parser_lemon_state regex_parser_lemon_state;

ptree regex_parser_lemon_tree(regex_parser_lemon_state *a);
void regex_parser_lemon_parse(regex_parser_lemon_state *a,int id,token t);
void regex_parser_lemon_finalize(regex_parser_lemon_state *a);
void regex_parser_lemon_initialize(regex_parser_lemon_state *a,ptree_context regex_ptree_context);

#if __STDC_HOSTED__
int regex_parser_lemon_type_header(void);
#endif

#endif
