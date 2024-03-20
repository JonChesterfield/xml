#include "../tools/EvilUnit/EvilUnit.h"

#include "ascii.parser_lemon.h"
#include "regex.parser_lemon.h"

#include "regex.ptree.h"
#include "regex_equality.h"

#include "../tools/stack.libc.h"

// Probably want an ascii.h defining things like the conversion to prefix string
#include "ascii_interpreter.h"

struct pair {
  const char *ascii;
  const char *regex;
};

bool equivalent(struct pair *cases, size_t N) {
  ptree_context ascii_context = regex_ptree_create_context();
  ptree_context regex_context = regex_ptree_create_context();

  bool equal = true;

  for (size_t i = 0; i < N; i++) {
    ptree ascii = ascii_parser_lemon_parse_cstr(ascii_context, cases[i].ascii);
    ptree regex = regex_parser_lemon_parse_cstr(regex_context, cases[i].regex);

    bool failed = ptree_is_failure(ascii) || ptree_is_failure(regex);

    bool differ = !regex_ptree_definitionally_equal(ascii, regex);
    if (differ) {
      equal = false;
    }

    if (failed || differ) {
      printf("%s ?= %s\n", cases[i].ascii, cases[i].regex);
      if (0) {
        // Weird. The malloc'ed pointer prints correctly before it is returned,
        // and valgrind thinks the program is clean, but looking at the string
        // here segfaults. Leave that for now.
        char *as_prefix = ascii_regex_as_prefix_regex_c_string(cases[i].ascii);
        if (as_prefix) {
          printf("wot %s\n", cases[i].ascii);
          fflush(stdout);
          printf("prefix %u\n", as_prefix[0]);
          fflush(stdout);
          printf("%s => %s ?= %s\n", cases[i].ascii, as_prefix, cases[i].regex);

        } else {
          printf("%s => failed-to-parse ?= %s \n", cases[i].ascii,
                 cases[i].regex);
        }
        free(as_prefix);
      }

      printf("ascii:\n");
      regex_ptree_as_xml(&stack_libc, stdout, ascii);
      printf("\n");
      printf("regex:\n");
      regex_ptree_as_xml(&stack_libc, stdout, regex);
      printf("\n");
    }
  }

  regex_ptree_destroy_context(ascii_context);
  regex_ptree_destroy_context(regex_context);

  return equal;
}

MODULE(ascii_regex_consistent) {

  TEST("decimals") {
    static struct pair cases[] = {
        {"0", "30"}, {"1", "31"}, {"2", "32"}, {"3", "33"}, {"4", "34"},
        {"5", "35"}, {"6", "36"}, {"7", "37"}, {"8", "38"}, {"9", "39"},
    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };
    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }

  TEST("lowercase") {
    static struct pair cases[] = {
        {"a", "61"}, {"b", "62"}, {"c", "63"}, {"d", "64"}, {"e", "65"},
        {"f", "66"}, {"g", "67"}, {"h", "68"}, {"i", "69"}, {"j", "6a"},
        {"k", "6b"}, {"l", "6c"}, {"m", "6d"}, {"n", "6e"}, {"o", "6f"},
        {"p", "70"}, {"q", "71"}, {"r", "72"}, {"s", "73"}, {"t", "74"},
        {"u", "75"}, {"v", "76"}, {"w", "77"}, {"x", "78"}, {"y", "79"},
        {"z", "7a"},
    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };
    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }

  TEST("uppercase") {
    static struct pair cases[] = {
        {"A", "41"}, {"B", "42"}, {"C", "43"}, {"D", "44"}, {"E", "45"},
        {"F", "46"}, {"G", "47"}, {"H", "48"}, {"I", "49"}, {"J", "4a"},
        {"K", "4b"}, {"L", "4c"}, {"M", "4d"}, {"N", "4e"}, {"O", "4f"},
        {"P", "50"}, {"Q", "51"}, {"R", "52"}, {"S", "53"}, {"T", "54"},
        {"U", "55"}, {"V", "56"}, {"W", "57"}, {"X", "58"}, {"Y", "59"},
        {"Z", "5a"},
    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };
    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }

  TEST("escaped non-printing / whitespace") {
    // Double \\ is one for C, one for the regex
    static struct pair cases[] = {
        {"\\a", "07"}, // bell
        {"\\e", "1b"}, // escape
        {" ", "20"},   // space

        {"\\t", "09"}, {"\\n", "0a"}, {"\\v", "0b"},
        {"\\f", "0c"}, {"\\r", "0d"},
    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };
    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }

  TEST("any") {
    // ascii any excludes newline. Might end up excluding > 127 if
    // that's what other engines do on utf8
    static struct pair cases[] = {
        {".", "(&.(~0a))"},
    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };
    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }

  TEST("ranges") {
    static struct pair cases[] = {
        {"[A]", "41"},
        {"[A-A]", "41"},

        {"[AB]", "(|4142)"},
        {"[A-B]", "(|4142)"},

        {"[ABC]", "(|41(|4243))"},
        {"[A-C]", "(|41(|4243))"},

        {"[ABCD]", "(|41(|42(|4344)))"},
        {"[A-D]", "(|41(|42(|4344)))"},

        {"[b-e]", "(|62(|63(|6465)))"},

        {"[A-Db-e]", "(|"
                     "(|41(|42(|4344)))"
                     "(|62(|63(|6465)))"
                     ")"},

        // Negated charsets are a not of the charset
        {
            "[^A-C]",
            "(~(|41(|4243)))",
        },
    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };
    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }

#if 0
  TEST("hyphen")
    {
    static struct pair cases[] = {
// Hyphen is tricky
// https://stackoverflow.com/questions/9589074/regex-should-hyphens-be-escaped

#if 0
> Outside of a character class (that's what the "square brackets" are called) the hyphen
> has no special meaning, and within a character class, you can place a hyphen as the first
> or last character in the range (e.g. [-a-z] or [0-9-]), OR escape it (e.g. [a-z\-0-9])
> in order to add "hyphen" to your class.
#endif   
        {
          "-",
          "2d",
        },
        {
          "[-]",
          "2d",
        },
    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };
    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }
#endif

  TEST("ad hoc") {
    static struct pair cases[] = {
      {
          "A",
          "41",
      },
      {
          "A|B",
          "(|4142)",
      },
      {
          "AB",
          "(:4142)",
      },
      {
          "D*",
          "(*44)",
      },
      {
          "E+",
          "(:45(*45))",
      },
      {
          "(F)",
          "46",
      },
      {
          "(G*)",
          "(*47)",
      },
      {
          "((G*))",
          "(*47)",
      },
#if 0
        {
          "G?",
          "(|G_)",
        },
#endif
    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };
    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }

  TEST("arith lang regex") {
    // The regex used by arith.lang.xml at time of writing this test
    static struct pair cases[] = {
#if 0
        {
          "[-]?[0-9]+[0-9]*",
          "42",
        },
#endif
      {
          // has a + suffix, splitting into two cases here
          "[ \\f\\n\\r\\t\\v]",
          "(|20(|0c(|0a(|0d(|090b)))))",
      },
      {"[ \\f]+", "(:(|200c)(*(|200c)))"},
    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };
    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }
}
