local str = ''
for i = 0, 255 do
   local name = string.format("byte_%02x",i)

   str = str .. string.format([[
static inline ptree regex_make_%s(ptree_context ctx)
{
  return regex_ptree_expression0(ctx, regex_grouping_%s);
}
]], name, name)
   
end

print(str)
