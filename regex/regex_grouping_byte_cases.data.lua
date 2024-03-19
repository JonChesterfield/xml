local str = ''

local sep = ''
for i = 0, 255 do
   str = str .. string.format("%s  case regex_grouping_byte_%02x:", sep,i)
   sep = "\n"
end

print(str)
