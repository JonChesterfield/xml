#include "ascii.parser_lemon.h"
#include "regex.lexer.h"
#include "regex.ptree.h"
#include "regex_to_c_mvp.h"

int main(void) {
  ptree_context context = regex_ptree_create_context();

  int rc = regexes_to_c_lexer_mvp(
      context, "regex", ascii_parser_lemon_parse_cstr, regex_token_count,
      regex_token_names, regex_regexes);

  regex_ptree_destroy_context(context);
  return rc;
}
