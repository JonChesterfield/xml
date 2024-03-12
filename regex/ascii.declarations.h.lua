local str = [[
#ifndef ASCII_DECLARATIONS_H_INCLUDED
#define ASCII_DECLARATIONS_H_INCLUDED
#include "regex.declarations.h"

// Currently only defines the bytes, can add to that if useful
// Force the 1:1 correspondance with whatever order regex.declarations.h chose
enum ascii_grouping {
]]

for i = 0, 255 do
   str = str .. string.format("\n  ascii_grouping_byte_%02x = regex_grouping_byte_%02x,",i,i)
end

str = str .. [[

};
#endif
]]

print(str)
