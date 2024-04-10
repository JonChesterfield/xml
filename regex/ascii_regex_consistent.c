#include "../tools/EvilUnit/EvilUnit.h"

#include "ascii.parser_lemon.h"
#include "regex.parser_lemon.h"

#include "regex.ptree.h"
#include "regex_equality.h"

#include "../tools/stack.libc.h"

// Probably want an ascii.h defining things like the conversion to prefix string
#include "ascii_interpreter.h"


// while debugging
#include "ascii.lexer.h"
#include "../tools/lexer.h"

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

    if (ptree_is_failure(ascii)) {
      const char * data = cases[i].ascii;
       printf("Failed to parse case %zu, %s\n", i, data);
       
      lexer_t lexer = ascii_lexer_create();
      if (!ascii_lexer_valid(lexer)) { exit(1); }

      for (lexer_iterator_t lexer_iterator = lexer_iterator_t_create(data, N);
           !lexer_iterator_t_empty(lexer_iterator);) {
        lexer_token_t lexer_token =
          ascii_lexer_iterator_step(lexer, &lexer_iterator);
    
        token lemon_token = token_create(ascii_token_names[lexer_token.id],
                                         lexer_token.value, lexer_token.width);

        if (ascii_lexer_discard_token(lexer_token.id)) {
          printf("discard a token\n");
          token_dump(lemon_token);
          break;
        }

        
        // 0 is the unknown token                                 
        if (lexer_token.id == 0) {
          printf("token unknown: ");
          token_dump(lemon_token);
          break;
        }

        token_dump(lemon_token);
      }      
    }
    
    if (failed || differ) {
      printf("%s ?= %s. Failed %u %u, differ %u\n", cases[i].ascii, cases[i].regex,
             ptree_is_failure(ascii),
             ptree_is_failure(regex),
             differ
             );
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

  TEST("dump")
    {
      printf("Ascii set:\n");
      for (unsigned i = 0; i < ascii_token_count; i++)
        {
          printf("Regex [%u] = %s\n", i, ascii_regexes[i]);
        }
    }
  
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

  TEST("hex escapes") {
    // check all 256 single byte escapes
    const char tab[16] = "0123456789abcdef";

    char ascii[5] = {'\\', 'x', 0, 0, '\0'};
    char regex[3] = {0, 0, '\0'};
    bool ok = true;
    for (unsigned l = 0; l < 16; l++)
      {
        for (unsigned h = 0; h < 16; h++)
          {
            ascii[3] = tab[h];
            ascii[4] = tab[l];
            regex[0] = tab[h];
            regex[1] = tab[l];
            struct pair cases[1] = {
              {ascii, regex},
            };
            ok &= equivalent(cases, 1);
          }
      } 
    CHECK(ok);
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

  TEST("or") {
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
          "A|A",
          "(|4141)",
      },
    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };
    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }

  TEST("concat") {
    static struct pair cases[] = {
      {
          "A",
          "41",
      },
      {
          "A|B",
          "(:4142)",
      },
      {
          "A|A",
          "(:4141)",
      },
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

        // Negated charsets are a not of the charset. This is wrong - need to specify that it's a single byte
        {
            "[^A-C]",
            "(~(|41(|4243)))",
        },

        // C escaped slashes inside the regex
        {
          "[\\f\\n\\r]*", "(*(|0c(|0a0d)))",
        },

        // single whitespace character, exclusive of space, hex escaped in C
        {
          "[\x5c\x66\x5c\x6e\x5c\x72\x5c\x74\x5c\x76]",
          "(|0c(|0a(|0d(|090b))))",
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

  TEST("suffixes") {
    // todo: are things like A*+ or B+? legal in pcre?
    static struct pair cases[] = {
      {
          "A",
          "41",
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
      {
        "G?",
        "(|47_)",
      },
      {
        "(G)?",
        "(|47_)",
      },
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
