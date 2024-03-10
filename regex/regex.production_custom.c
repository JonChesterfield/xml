#include "regex.productions.h"
#include "regex.declarations.h"
#include "regex.ptree.h"

#include "regex.h"

static uint8_t parse_hex(uint8_t x)
{
      switch(x)
        {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'a': return 10;
        case 'b': return 11;
        case 'c': return 12;
        case 'd': return 13;
        case 'e': return 14;
        case 'f': return 15;
        default: return 255;
        }
}

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
