local args = {...}

if #args ~= 1 then
   os.exit(1)
end

local template = [=[
#ifndef UPPERNAME_PARSER_LEMON_H_INCLUDED
#define UPPERNAME_PARSER_LEMON_H_INCLUDED

#include "../tools/token.h"
#include "../tools/ptree.h"

struct LANGNAME_parser_lemon_state;
typedef struct LANGNAME_parser_lemon_state LANGNAME_parser_lemon_state;

ptree LANGNAME_parser_lemon_tree(LANGNAME_parser_lemon_state *a);
void LANGNAME_parser_lemon_parse(LANGNAME_parser_lemon_state *a, int id,token t);
void LANGNAME_parser_lemon_finalize(LANGNAME_parser_lemon_state *a);
void LANGNAME_parser_lemon_initialize(LANGNAME_parser_lemon_state *a, ptree_context LANGNAME_ptree_context);

ptree LANGNAME_parser_lemon_parse_cstr(ptree_context context, const char * data);

#if __STDC_HOSTED__
int LANGNAME_parser_lemon_type_header(void);
#endif

#endif
]=]

local res = template:gsub('LANGNAME', args[1]):gsub('UPPERNAME', args[1]:upper())

print(res)
os.exit(0)
