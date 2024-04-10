#include "regex.productions.h"
#include "regex.declarations.h"
#include "regex.ptree.h"

#include "regex.h"
#include "parse_hex.h"

ptree regex_custom_production_make_byte(ptree_context ctx ,token /*BYTE*/ x1)
{
  if (x1.width == 2)
    {
      uint8_t l = parse_hex(x1.value[0]);
      uint8_t r = parse_hex(x1.value[1]);

      if ((l < 16) && (r < 16))
        {
          uint8_t byte = (uint8_t)(l * 16) + r;
          return regex_grouping_single_from_byte(ctx, byte);
        }    
    }
  return ptree_failure();
}
