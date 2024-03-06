-- Takes one argument, emits C code that translates that language
-- from bytes on stdin to xml on stdout

local args = {...}

if #args ~= 1 then
   os.exit(1)
end

local template = [=[#include "../tools/io_buffer.h"
#include <stdio.h>

#include "LANGNAME.h"
#include "LANGNAME.ptree.h"

#include "LANGNAME.lexer.h"

#include "LANGNAME_parser.lemon.h"
#include "LANGNAME_parser.lemon.t"

#include "../tools/stack.libc.h"
#include "../tools/lexer.h"

static int print_tokens(char *data, size_t N) {
  lexer_t lexer = LANGNAME_lexer_create();
  if (!LANGNAME_lexer_valid(lexer)) {
    return 2;
  }

  for (lexer_iterator_t lexer_iterator = lexer_iterator_t_create(data, N);
       !lexer_iterator_t_empty(lexer_iterator);) {
    lexer_token_t lexer_token =
        LANGNAME_lexer_iterator_step(lexer, &lexer_iterator);
    if (LANGNAME_lexer_discard_token(lexer_token.id)) {
      continue;
    }
    lexer_token_dump(lexer_token);
  }

  LANGNAME_lexer_destroy(lexer);
  return 0;
}

int main() {
  io_buffer *in = file_to_io_buffer(stdin);
  if (!in) {
    return 1;
  }

  ptree_context ctx = LANGNAME_ptree_create_context();

  lexer_t lexer = LANGNAME_lexer_create();
  if (!LANGNAME_lexer_valid(lexer)) {
    return 2;
  }

  LANGNAME_parser_lemon_state parser;
  LANGNAME_parser_lemon_initialize(&parser, ctx);

  for (lexer_iterator_t lexer_iterator =
           lexer_iterator_t_create(in->data, in->N);
       !lexer_iterator_t_empty(lexer_iterator);) {
    lexer_token_t lexer_token =
        LANGNAME_lexer_iterator_step(lexer, &lexer_iterator);
    if (!(lexer_token.id < LANGNAME_token_count) ||
        (lexer_token.id == LANGNAME_token_UNKNOWN)) {
      return 2;
    }

    if (LANGNAME_lexer_discard_token(lexer_token.id)) {
      continue;
    }

    token lemon_token = token_create(LANGNAME_token_names[lexer_token.id],
                                     lexer_token.value, lexer_token.width);

    if (!LANGNAME_ptree_identifier_valid_token(lexer_token.id)) {
      return 3;
    }

    LANGNAME_parser_lemon_parse(&parser, (int)lexer_token.id, lemon_token);
  }

  ptree res = LANGNAME_parser_lemon_tree(&parser);
  if (ptree_is_failure(res)) {
    int rc = print_tokens(in->data, in->N);
    if (rc != 0) {
      return rc;
    }
    return 4;
  }

  LANGNAME_ptree_as_xml(&stack_libc, stdout, res);

  LANGNAME_parser_lemon_finalize(&parser);
  LANGNAME_lexer_destroy(lexer);
  free(in);

  return 0;
}]=]

local res = template:gsub('LANGNAME', args[1])

print(res)
os.exit(0)
