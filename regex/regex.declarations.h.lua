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
static inline bool regex_grouping_id_is_single_byte(uint64_t id)
{
  return (regex_grouping_byte_00 <= id) && (id <= regex_grouping_byte_ff);
}

static inline uint8_t regex_grouping_extract_single_byte(uint64_t id)
{
  // requires regex_grouping_id_is_single_byte(id)
  return id - regex_grouping_byte_00;
}

static inline ptree regex_grouping_single_from_byte(ptree_context ctx, uint8_t byte)
{
  uint64_t id = byte + regex_grouping_byte_00;
  return regex_ptree_expression0(ctx, id);
}
]]

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
