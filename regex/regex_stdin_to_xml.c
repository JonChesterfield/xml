#include "../tools/io_buffer.h"
#include <stdio.h>

#include "regex.h"
#include "regex.ptree.h"

#include "regex.lexer.declarations.h"

#include "regex_parser.lemon.h"
#include "regex_parser.lemon.t"

#include "../tools/stack.libc.h"

static int print_tokens(char *data, size_t N) {
  lexer_t lexer = regex_lexer_create();
  if (!regex_lexer_valid(lexer)) {
    return 2;
  }

  for (lexer_iterator_t lexer_iterator = lexer_iterator_t_create(data, N);
       !lexer_iterator_t_empty(lexer_iterator);) {
    lexer_token_t lexer_token =
        regex_lexer_iterator_step(lexer, &lexer_iterator);
    if (regex_lexer_discard_token(lexer_token.id)) {
      continue;
    }
    lexer_token_dump(lexer_token);
  }

  regex_lexer_destroy(lexer);
  return 0;
}

int main() {
  io_buffer *in = file_to_io_buffer(stdin);
  if (!in) {
    return 1;
  }

  ptree_context ctx = regex_ptree_create_context();

  lexer_t lexer = regex_lexer_create();
  if (!regex_lexer_valid(lexer)) {
    return 2;
  }

  struct regex_parser_s parser_state;
  regex_parser_type *parser = (regex_parser_type *)&parser_state;
  regex_parser_initialize(parser, ctx);

  for (lexer_iterator_t lexer_iterator =
           lexer_iterator_t_create(in->data, in->N);
       !lexer_iterator_t_empty(lexer_iterator);) {
    lexer_token_t lexer_token =
        regex_lexer_iterator_step(lexer, &lexer_iterator);
    if (!(lexer_token.id < regex_token_count) ||
        (lexer_token.id == regex_token_UNKNOWN)) {
      return 2;
    }

    if (regex_lexer_discard_token(lexer_token.id)) {
      continue;
    }

    token lemon_token = token_create(regex_token_names[lexer_token.id],
                                     lexer_token.value, lexer_token.width);

    if (!regex_ptree_identifier_valid_token(lexer_token.id)) {
      return 3;
    }

    regex_parser_parse(parser, (int)lexer_token.id, lemon_token);
  }

  ptree res = regex_parser_tree(parser);
  if (ptree_is_failure(res)) {
    int rc = print_tokens(in->data, in->N);
    if (rc != 0) {
      return rc;
    }
    return 4;
  }

  regex_ptree_as_xml(&stack_libc, stdout, res);

  regex_parser_finalize(parser);
  regex_lexer_destroy(lexer);
  free(in);

  return 0;
}
