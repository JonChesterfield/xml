#include "ascii.productions.h"
#include "ascii.declarations.h"
#include "ascii.ptree.h"

#include "regex.h"

ptree ascii_custom_production_result(ptree_context ctx ,token /*BYTE6b*/ x1)
{
  return ptree_failure();
}

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
  return ptree_failure();
}
