#include "../tools/EvilUnit/EvilUnit.h"

#include "ascii.parser_lemon.h"
#include "regex.parser_lemon.h"

#include "regex.ptree.h"
#include "regex_equality.h"

#include "../tools/stack.libc.h"

// Probably want an ascii.h defining things like the conversion to prefix string
#include "ascii_interpreter.h"

// while debugging
#include "../tools/lexer.h"
#include "ascii.lexer.h"

struct pair {
  const char *ascii;
  const char *regex;
};

static bool equivalent_impl(struct pair *cases, size_t N, bool verbose) {
  ptree_context ascii_context = regex_ptree_create_context();
  ptree_context regex_context = regex_ptree_create_context();

  bool equal = true;

  for (size_t i = 0; i < N; i++) {
    ptree ascii = ascii_parser_lemon_parse_cstr(ascii_context, cases[i].ascii);
    ptree regex = regex_parser_lemon_parse_cstr(regex_context, cases[i].regex);

    bool failed = ptree_is_failure(ascii) || ptree_is_failure(regex);

    regex_compare_t defn_equal = regex_ptree_definitionally_equal(ascii, regex);
    bool differ = defn_equal == regex_compare_not_equal;

    if (verbose) {
      printf("Case %zu, eq %d\n", i, defn_equal);
      printf("ascii %s:\n", cases[i].ascii);
      regex_ptree_as_xml(&stack_libc, stdout, ascii);
      printf("\n");
      printf("regex %s:\n", cases[i].regex);
      regex_ptree_as_xml(&stack_libc, stdout, regex);
      printf("\n");
    }

    if (differ) {
      equal = false;
    }

    if (ptree_is_failure(ascii)) {
      const char *data = cases[i].ascii;
      printf("Failed to parse case %zu, %s\n", i, data);

      lexer_t lexer = ascii_lexer_create();
      if (!ascii_lexer_valid(lexer)) {
        exit(1);
      }

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
      printf("%s ?= %s. Failed %u %u, differ %u\n", cases[i].ascii,
             cases[i].regex, ptree_is_failure(ascii), ptree_is_failure(regex),
             differ);
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

bool equivalent(struct pair *cases, size_t N) {
  return equivalent_impl(cases, N, false);
}

bool equivalent_verbose(struct pair *cases, size_t N) {
  return equivalent_impl(cases, N, true);
}

MODULE(ascii_regex_consistent) {

  TEST("dump") {
    if (0) {
      printf("Ascii set:\n");
      for (unsigned i = 0; i < ascii_token_count; i++) {
        printf("Regex [%u] = %s\n", i, ascii_regexes[i]);
      }
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

    char ascii[5] = {'\\', 'x', '_', '_', '\0'};
    char regex[3] = {0, 0, '\0'};
    bool ok = true;
    for (unsigned l = 0; l < 16; l++) {
      for (unsigned h = 0; h < 16; h++) {
        ascii[2] = tab[h];
        ascii[3] = tab[l];
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

  TEST("special characters") {
    // in order of ascii table. Some of them need to be escaped, some don't
    // should probably check all escaping patterns for each one, / prefixed etc
    static struct pair cases[] = {
      // start of printable characters
        {" ", "20"},
        {"[ ]", "20"},
        {"!", "21"},
        {"[!]", "21"},
        {"\"", "22"},
        {"[\"]", "22"},
        {"#", "23"},
        {"[#]", "23"},
        {"\\$", "24"},
        {"[$]", "24"},
        {"%", "25"},
        {"[%]", "25"},
        {"&", "26"},
        {"[&]", "26"},
        {"'", "27"},
        {"[']", "27"},
        {"\\(", "28"},
        {"[(]", "28"},
        {"\\)", "29"},
        {"[)]", "29"},
        {"\\*", "2a"},
        {"[*]", "2a"},
        {"\\+", "2b"},
        {"[+]", "2b"},
        {",", "2c"},
        {"[,]", "2c"},
        {"-", "2d"},
        {"[-]", "2d"},
        {".", "(&.(~0a))"}, // . excludes newline
        {"[.]", "2e"},
        {"/", "2f"},
        {"[/]", "2f"},

        // digits follow 2f
        {"0", "30",},
        {"[0]", "30",},
        {"1", "31",},
        {"[1]", "31",},
        {"2", "32",},
        {"[2]", "32",},
        {"3", "33",},
        {"[3]", "33",},
        {"4", "34",},
        {"[4]", "34",},
        {"5", "35",},
        {"[5]", "35",},
        {"6", "36",},
        {"[6]", "36",},
        {"7", "37",},
        {"[7]", "37",},
        {"8", "38",},
        {"[8]", "38",},
        {"9", "39",},
        {"[9]", "39",},

        // next block of punctuation
        {":", "3a",},
        {"[:]", "3a",},
        {";", "3b",},
        {"[;]", "3b",},
        {"<", "3c",},
        {"[<]", "3c",},
        {"=", "3d",},
        {"[=]", "3d",},
        {">", "3e",},
        {"[>]", "3e",},
        {"\\?", "3f",},
        {"[?]", "3f",},

        {"@", "40",},
        {"[@]", "40",},

        {"A", "41",},
        {"Z", "5a",},
        
        {"\\[", "5b",},
        {"[\\[]", "5b",},
        // {"[[]", "5b",}, // unimplemented
        {"\\\\", "5c",},
        {"[\\\\]", "5c",}, // A \\ in the character class, with C escapes
        {"\\]", "5d",},
        {"[\\]]", "5d",},
        // {"[]]", "5d",}, // should work but needs to be special cased
        {"^", "5e",},
        {"\\^", "5e",},                
        // "[^]" means negated empty bracket, but [k^] is supposed to match the literal ^
        {"[B^]", "(|5e42)",},
        {"_", "5f",},
        {"[_]", "5f",},
        {"`", "60",},
        {"[`]", "60",},

        {"a", "61",},
        {"z", "7a",},
        
        // trailing end of table
        {"{", "7b"},
        {"[{]", "7b"},
        // {"|", "7c"},
        {"[|]", "7c"},
        {"}", "7d"},
        {"[}]", "7d"},
        {"~", "7e"},
        {"[~]", "7e"},
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
            "AB",
            "(:4142)",
        },
        {
            "AA",
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

        {"[ABC]", "(|(|4142)43)"},
        {"[A-C]", "(|(|4142)43)"},

        {"[ABCD]", "(|(|(|4142)43)44)"},
        {"[A-D]", "(|(|(|4142)43)44)"},

        {"[b-e]", "(|(|(|6263)64)65)"},

        {"[A-Db-e]", "(|"
                     "(|(|(|4142)43)44)"
                     "(|(|(|6263)64)65)"
                     ")"},

        {"[^B]", "(&.(~42))"},

        // Negated charsets are a not of the charset. This is wrong - need to
        // specify that it's a single byte

        {
            "[^A-C]",
            "(&.(~(|(|4142)43)))",
        },

        // C escaped slashes inside the regex
        {
            "[\\f\\n\\r]*",
            "(*(|(|0c0a)0d))",
        },

        // single whitespace character, exclusive of space, hex escaped in C
        {
            "[\x5c\x66\x5c\x6e\x5c\x72\x5c\x74\x5c\x76]",
            "(|(|(|(|0c0a)0d)09)0b)",
        },

        // empty brackets.
        {
            "[]",
            "%", // failure - no single char will match anything in the brackets
        },
        {
            "[^]",
            ".", // the real any, inclusive of newline
        },

    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };

    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }

#if 1
  TEST("hyphen") {
    static struct pair cases[] = {
    // Hyphen is tricky
    // https://stackoverflow.com/questions/9589074/regex-should-hyphens-be-escaped

#if 0
> Outside of a character class (that's what the "square brackets" are called) the hyphen
> has no special meaning, and within a character class, you can place a hyphen as the first
> or last character in the range (e.g. [-a-z] or [0-9-]), OR escape it (e.g. [a-z\-0-9])
> in order to add "hyphen" to your class.
#endif

// Posix
// https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap09.html#tag_09_03_05
#if 0


In the POSIX locale, a range expression represents the set of collating elements that fall between two elements in the collation sequence, inclusive. In other locales, a range expression has unspecified behavior: strictly conforming applications shall not rely on whether the range expression is valid, or on the set of collating elements matched. A range expression shall be expressed as the starting point and the ending point separated by a <hyphen-minus> ( '-' ).

In the following, all examples assume the POSIX locale.

The starting range point and the ending range point shall be a collating element or collating symbol. An equivalence class expression used as a starting or ending point of a range expression produces unspecified results. An equivalence class can be used portably within a bracket expression, but only outside the range. If the represented set of collating elements is empty, it is unspecified whether the expression matches nothing, or is treated as invalid.

The interpretation of range expressions where the ending range point is also the starting range point of a subsequent range expression (for example, "[a-m-o]" ) is undefined.

The <hyphen-minus> character shall be treated as itself if it occurs first (after an initial '^', if any) or last in the list, or as an ending range point in a range expression. As examples, the expressions "[-ac]" and "[ac-]" are equivalent and match any of the characters 'a', 'c', or '-'; "[^-ac]" and "[^ac-]" are equivalent and match any characters except 'a', 'c', or '-'; the expression "[%--]" matches any of the characters between '%' and '-' inclusive; the expression "[--@]" matches any of the characters between '-' and '@' inclusive; and the expression "[a--@]" is either invalid or equivalent to '@', because the letter 'a' follows the symbol '-' in the POSIX locale. To use a <hyphen-minus> as the starting range point, it shall either come first in the bracket expression or be specified as a collating symbol; for example, "[][.-.]-0]", which matches either a <right-square-bracket> or any character or collating element that collates between <hyphen-minus> and 0, inclusive.

If a bracket expression specifies both '-' and ']', the ']' shall be placed first (after the '^', if any) and the '-' last within the bracket expression.
#endif

      {
          "-",
          "2d",
      },
      {
          "-*",
          "(*2d)",
      },
      {
          "[-]",
          "2d",
      },
      {
          "[-]*",
          "(*2d)",
      },
      {
          "\\-",
          "2d",
      },
      {
          "[\\-]",
          "2d",
      },
      {
          "[-\\-]",
          "(|2d2d)",
      },
      {
          "[\\--]",
          "(|2d2d)",
      },
      {
          "[-\\--]",
          "(|2d2d)",
      },
      {
          "[-\\-\\--]",
          "(|2d2d)",
      },
      {
          "[-\\-B\\--]",
          "(|2d(|2d42))",
      },
      
      {
          "[-B]",
          "(|2d42)",
      },
      {
          "[B-]",
          "(|2d42)",
      },
      {
          "[--]",
          "2d",
      },
      {
          "[-B-]",
          "(|2d42)",
      },

      {
          "[^-]",
          "(&.(~2d))",
      },
      {
          "[^--]",
          "(&.(~2d))",
      },
      {
          "[^-B]",
          "(&.(~(|2d42)))",
      },
      {
          "[^-B-]",
          "(&.(~(|2d42)))",
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
        // simplified from [0-9] to B
        {
            "[-]?B+B*",
            "(:(:(|2d_)(:42(*42)))(*42))",
        },

        // simplified from [0-9] to [0-2]
        {
            "[-]?[0-2]+[0-2]*",
            "(:(:(|2d_)(:(|(|3031)32)(*(|(|3031)32))))(*(|(|3031)32)))",
        },

        {
            // has a + suffix, splitting into two cases here
            "[\\f\\n\\r\\t\\v]",
            "(|(|(|(|0c0a)0d)09)0b)",
        },

        {"[ \\f]+", "(:(|200c)(*(|200c)))",},
    };
    enum {
      cases_size = sizeof(cases) / sizeof(cases[0]),
    };
    CHECK(equivalent(cases, sizeof(cases) / sizeof(cases[0])));
  }
}
