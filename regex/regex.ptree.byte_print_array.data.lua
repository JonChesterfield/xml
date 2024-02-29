local strlit = "static const char regex_ptree_byte_print_array[256 * regex_ptree_byte_print_array_stride] = {"
for i = 0, 255 do
   local name = string.format("%02x\\0",i)

   strlit = string.format("%s'B',", strlit)   
   strlit = string.format("%s'%s',", strlit, name:sub(1,1))
   strlit = string.format("%s'%s',", strlit, name:sub(2,2))
   strlit = string.format("%s0,", strlit)
end

strlit = strlit .. "};"

print(strlit)
