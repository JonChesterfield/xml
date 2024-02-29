local str = [[
#ifndef REGEX_DECLARATIONS_H_INCLUDED
#define REGEX_DECLARATIONS_H_INCLUDED

#include "regex.ptree.h"

enum regex_token {
  regex_token_UNKNOWN = 0,
};
enum {regex_token_count = 1 };
]]


str = str .. [[

enum regex_grouping {
  regex_grouping_empty_set = regex_token_count + 0,
  regex_grouping_empty_string = regex_token_count + 1,
  regex_grouping_concat = regex_token_count + 2,
  regex_grouping_kleene = regex_token_count + 3,
  regex_grouping_or = regex_token_count + 4,
  regex_grouping_and = regex_token_count + 5,
  regex_grouping_not = regex_token_count + 6,
  regex_grouping_range = regex_token_count + 7,]]

local bytecount = 8
for i = 0, 255 do
   str = str .. string.format("\n  regex_grouping_byte_%02x = regex_token_count + %s,",i,bytecount)
   bytecount = bytecount + 1
end

  
str = str .. string.format([[

};

enum { regex_grouping_count = regex_token_count + %s, };

]], bytecount)

str = str .. [[
static inline ptree regex_make_empty_set(ptree_context ctx)
{
  return regex_ptree_expression0(ctx, regex_grouping_empty_set);
}
static inline ptree regex_make_empty_string(ptree_context ctx)
{
  return regex_ptree_expression0(ctx, regex_grouping_empty_string);
}
static inline ptree regex_make_kleene(ptree_context ctx, ptree val)
{
  return regex_ptree_expression1(ctx, regex_grouping_kleene, val);
}
static inline ptree regex_make_not(ptree_context ctx, ptree val)
{
  return regex_ptree_expression1(ctx, regex_grouping_not, val);
}
static inline ptree regex_make_range(ptree_context ctx, ptree lhs, ptree rhs)
{
  // todo, force these to be bytes somewhere
  // uint64_t lhs_id = regex_ptree_identifier(lhs);
  // uint64_t rhs_id = regex_ptree_identifier(rhs);
  return regex_ptree_expression2(ctx, regex_grouping_range, lhs, rhs);
}

]]

for _, v in ipairs({'concat', 'or', 'and',}) do
   str = str .. string.format([[
static inline ptree regex_make_%s(ptree_context ctx, ptree lhs, ptree rhs)
{
  return regex_ptree_expression2(ctx, regex_grouping_%s, lhs, rhs);
}
]], v, v)
end


for i = 0, 255 do
   local name = string.format("byte_%02x",i)

   str = str .. string.format([[
static inline ptree regex_make_%s(ptree_context ctx)
{
  return regex_ptree_expression0(ctx, regex_grouping_%s);
}
]], name, name)
   
end

   
str = str .. [[

#endif
]]

print(str)
