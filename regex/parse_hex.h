#ifndef REGEX_PARSE_HEX_INCLUDED
#define REGEX_PARSE_HEX_INCLUDED
#include <stdint.h>

static inline uint8_t parse_hex(uint8_t x)
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

#endif
