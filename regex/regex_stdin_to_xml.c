#include "../tools/io_buffer.h"
#include <stdio.h>

#include "regex.h"
#include "regex.ptree.h"
#include "regex_string.h"

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

    if (lexer_token.id == regex_token_WHITESPACE) {
      continue;
    }
    lexer_token_dump(lexer_token);
  }

  regex_lexer_destroy(lexer);
  return 0;
}

// Would like to read a string on stdin and print it as the xml representation
// of the parsed output
int main() {
  io_buffer *in = file_to_io_buffer(stdin);
  if (!in) {
    return 1;
  }

  ptree_context ctx = regex_ptree_create_context();
  ptree res = regex_from_char_sequence(ctx, in->data, in->N);
  if (ptree_is_failure(res)) {
    int rc = print_tokens(in->data, in->N);
    if (rc != 0) {
      return rc;
    }
    return 3;
  }

  regex_ptree_as_xml(&stack_libc, stdout, res);

  free(in);
  return 0;
}
