#include "../tools/EvilUnit/EvilUnit.h"

#include "ascii.parser_lemon.h"
#include "regex.parser_lemon.h"

#include "regex.ptree.h"
#include "regex_equality.h"

#include "../tools/stack.libc.h"

struct pair {
  const char *ascii;
  const char *regex;
};

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
};
enum {
  cases_size = sizeof(cases) / sizeof(cases[0]),
};

MODULE(ascii_regex_consistent) {
  ptree_context ascii_context = regex_ptree_create_context();
  ptree_context regex_context = regex_ptree_create_context();

  for (size_t i = 0; i < cases_size; i++) {
    ptree ascii = ascii_parser_lemon_parse_cstr(ascii_context, cases[i].ascii);
    ptree regex = regex_parser_lemon_parse_cstr(regex_context, cases[i].regex);
    CHECK(!ptree_is_failure(ascii));
    CHECK(!ptree_is_failure(regex));

    CHECK(regex_ptree_definitionally_equal(ascii, regex));
    
    printf("ascii:\n");
    regex_ptree_as_xml(&stack_libc, stdout, ascii);
    printf("\n");
    printf("regex:\n");
    regex_ptree_as_xml(&stack_libc, stdout, regex);
    printf("\n");
  }

  regex_ptree_destroy_context(ascii_context);
  regex_ptree_destroy_context(regex_context);
}
