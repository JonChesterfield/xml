#include "interpreter.data"
#include "regex_interpreter.h"
#include "regex_string.h"

#include "../tools/EvilUnit/EvilUnit.h"

#include <string.h>

struct regex_case {
  const char *reg;
  size_t sz_reg;
  const char *str;
  size_t sz_str;
  uint64_t expect;
};

static void dump_val(uint64_t val) {
  const char * str = 0;
  switch(val)
    {
    case match_failure:
      str = "failure"; break;
    case machine_failure:
      str = "machine"; break;
    case regex_unrecognised:
      str = "invalid"; break;
    default:
      break;    
    }
  if (str) {
    printf("%s", str);
  } else {
    printf("%lu", val);
  }
}

static void dump_case(struct regex_case s, uint64_t res) {
  printf("reg %s sbj ", s.reg);
  for (size_t i = 0; i < s.sz_str; i++) {
    printf("%02x", s.str[i]);
  }
  printf(" -> [ex ");
  dump_val(s.expect);
  printf("] != [res ");
  dump_val(res);
  printf("]\n");

}

// Allows embedded 0s vs using strlen
#define R(X) X, (sizeof(X)-1)

MODULE(regex_match) {

  TEST("wip") {
    regex_cache_t cache = regex_cache_create();
    CHECK(regex_cache_valid(cache));
    printf("length of \x01 is %lu\n", strlen("\x01"));
    struct regex_case cases[] = {
        {
            R("01"),
            R("\x01"),
            1,
        },
        {
            R("01"),
            R("\x01\x02"),
            1,
        },
        {
            R("01"),
            R("\x01\x01"),
            1,
        },

        // This case was written in error (missing concat :)
        // but parsed as a valid regex 01, instead of rejecting the 0101 form
        {
            R("0101"),
            R("\x01"),
            regex_unrecognised,
        },
        {
            R("(:0101)"),
            R("\x01\x02"),
            match_failure,
        },
        
        {
            R("(:0102)"),
            R("\x01\x02\x03"),
            2,
        },
        {
            R("(*02)"),
            R("\x02\x02\x33"),
            2,
        },

        {
          R("(:6163)"),
          R("acd"),
          2,
        },
        
        // ~% is the obvious and wrong render of any
        // specifically ~% is any string, not any character
        // (|a(~a)) is a less obvious representation
        // Derivative of ~% is itself
        // Inevitably deriviative of (|a(~a)) is ~%
        
        {
          R("(:61(~%))"),
          R("abc"),
          3,
        },


        {
          R("(:61(|00(~00)))"),
          R("abc"),
          3,         
        },
        
        {
          R("(:(~%)62)"),
          R("abc"),
          2,
        },

        
        {
          R("(:61(:(~%)63))"),
          R("abcd"),
          3,
        },

        
    };
    enum { size = sizeof(cases) / sizeof(cases[0]) };

    for (size_t i = 0; i < size; i++) {
      printf("\n");
      CHECK(regex_in_byte_representation(cases[i].reg, cases[i].sz_reg));
      uint64_t res = regex_interpreter_with_context_string_matches(
          &cache, (const unsigned char *)cases[i].reg, cases[i].sz_reg,
          (const unsigned char *)cases[i].str, cases[i].sz_str);

      if (res != cases[i].expect) {
        dump_case(cases[i], res);
      }
      
      CHECK(res == cases[i].expect);
    }

    regex_cache_destroy(cache);
  }

  CHECK(1);
}
