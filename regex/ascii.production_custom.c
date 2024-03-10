#include "ascii.productions.h"
#include "ascii.declarations.h"
#include "ascii.ptree.h"

#include "regex.h"

#include "../tools/stack.libc.h"

ptree ascii_custom_production_union_ctor(ptree_context ctx ,ptree /*result_RE*/ x1 ,token /*PIPE*/ x2 ,ptree /*simple_RE*/ x3)
{
  return regex_make_or(ctx, x1, x3);
}

ptree ascii_custom_production_concat_ctor(ptree_context ctx ,ptree /*simple_RE*/ x1 ,ptree /*basic_RE*/ x2)
{
  return regex_make_concat(ctx, x1, x2);
}

ptree ascii_custom_production_star_ctor(ptree_context ctx ,ptree /*elementary_RE*/ x1 ,token /*STAR*/ x2)
{
  return regex_make_kleene(ctx, x1);
}

ptree ascii_custom_production_plus_ctor(ptree_context ctx ,ptree /*elementary_RE*/ x1 ,token /*PLUS*/ x2)
{
  return regex_make_concat(ctx, x1, regex_make_kleene(ctx, x1));
}

ptree ascii_custom_production_set_item_multiple(ptree_context ctx ,ptree /*set_item*/ x1 ,ptree /*set_items*/ x2)
{
  return regex_make_or(ctx, x1, x2);
}

ptree ascii_custom_production_range_ctor(ptree_context ctx ,ptree /*char_RE*/ x1 ,token /*HYPHEN*/ x2 ,ptree /*char_RE*/ x3)
{
    if (!regex_is_single_byte(x1) ||
      !regex_is_single_byte(x3)) {
      printf("range ctor passed non-single bytes\n");
    return ptree_failure();    
  }

  uint8_t left = regex_grouping_extract_single_byte(regex_ptree_identifier(x1));
  uint8_t right = regex_grouping_extract_single_byte(regex_ptree_identifier(x3));

  if (left > right)
    {
      // Could be match nothing, could be an error
      printf("failing range ctor as left > right, %u %u\n", left, right);
      return ptree_failure();      
    }


  ptree cursor = regex_grouping_single_from_byte(ctx, right);

  while (left != right)
    {
      right--;
      ptree tmp = regex_grouping_single_from_byte(ctx, right);
      cursor = regex_make_or(ctx, tmp, cursor);      
    }
  
  return cursor;
}

ptree ascii_custom_production_from_decimal(ptree_context ctx ,token /*Decimal*/ x1)
{
  if (x1.width == 1)
    {
      uint8_t u = x1.value[0];
      if ((u >= 0x30u) && (u <= 0x39u))
        {
          return regex_grouping_single_from_byte(ctx, u);
        }
    }
   printf("failure from decimal\n");
   return ptree_failure();    
}

ptree ascii_custom_production_from_lowercase(ptree_context ctx ,token /*Lowercase*/ x1)
{
  if (x1.width == 1)
    {
      uint8_t u = x1.value[0];
      if ((u >= 0x61u) && (u <= 0x7au))
        {
          return regex_grouping_single_from_byte(ctx, u);
        }
    }
  printf("failure from lowercase\n");
  return ptree_failure();
}

ptree ascii_custom_production_from_uppercase(ptree_context ctx ,token /*Uppercase*/ x1)
{
  if (x1.width == 1)
    {
      uint8_t u = x1.value[0];
      if ((u >= 0x41u) && (u <= 0x5au))
        {
          return regex_grouping_single_from_byte(ctx, u);
        }
    }
  printf("failure from uppercase\n");
  return ptree_failure();
}
