local str = [[
#ifndef REGEX_DECLARATIONS_H_INCLUDED
#define REGEX_DECLARATIONS_H_INCLUDED

#include "regex.ptree.h"
#include "regex.lexer.declarations.h"

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

#endif
]]

print(str)
