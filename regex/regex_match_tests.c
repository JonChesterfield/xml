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

static void dump_case(struct regex_case s, uint64_t res) {
#if 1
  printf("\n%s ", s.reg);
  for (size_t i = 0; i < s.sz_str; i++) {
    printf("%02x", s.str[i]);
  }
  printf(" -> %lu != %lu\n", s.expect, res);
#else
  printf("\n%s/%zu ", s.reg, s.sz_reg);
  for (size_t i = 0; i < s.sz_str; i++) {
    printf("%02x", s.str[i]);
  }
  printf("/%zu -> %lu != %lu\n", s.sz_str, s.expect, res);
#endif
}

// Mainly to allow embedded zeros
#define R(X) X, (sizeof(X) - 1)

MODULE(regex_match) {

  TEST("wip") {
    regex_cache_t cache = regex_cache_create();
    CHECK(regex_cache_valid(cache));
    struct regex_case cases[] = {
        {
            R("01"),
            R("\x01"),
            1,
        },
        {
            R("(:0102)"),
            R("\x01\x02"),
            2,
        },
        {
            R("(*02)"),
            R("\x02\x02\x33"),
            1,
        },
    };
    enum { size = sizeof(cases) / sizeof(cases[0]) };

    for (size_t i = 0; i < size; i++) {

      CHECK(regex_in_byte_representation(cases[i].reg, cases[i].sz_reg));
      uint64_t res = regex_interpreter_with_context_string_matches(
          &cache, (const unsigned char *)cases[i].reg, cases[i].sz_reg,
          (const unsigned char *)cases[i].str, cases[i].sz_str);

      dump_case(cases[i], res);

      CHECK(res != match_failure);
      CHECK(res != machine_failure);
      CHECK(res != regex_unrecognised);

      CHECK(res == cases[i].expect);
    }

    regex_cache_destroy(cache);
  }

  CHECK(1);
}
