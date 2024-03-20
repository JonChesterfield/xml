-- Takes one argument, emits C code that translates that language
-- from bytes on stdin to xml on stdout

local args = {...}

if #args ~= 1 then
   os.exit(1)
end

local template = [=[#include "../tools/io_buffer.h"
#include <stdio.h>

// #include "LANGNAME.h"
#include "LANGNAME.ptree.h"

#include "LANGNAME.lexer.h"

#include "LANGNAME.parser_lemon.h"
#include "LANGNAME.parser_lemon.t"

#include "LANGNAME.parser_bison.h"

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

  // printf("Read %zu bytes\n", in->N);
  ptree_context ctx = LANGNAME_ptree_create_context();

  lexer_t lexer = LANGNAME_lexer_create();
  if (!LANGNAME_lexer_valid(lexer)) {
    printf("lexer creation failed\n");
    return 2;
  }

  LANGNAME_parser_lemon_state parser;
  LANGNAME_parser_lemon_initialize(&parser, ctx);

  LANGNAME_bison_pstate * bison_parser = LANGNAME_bison_pstate_new();
  ptree bison_arg = ptree_failure();

  unsigned iter_count = 0;
  for (lexer_iterator_t lexer_iterator =
           lexer_iterator_t_create(in->data, in->N);
       !lexer_iterator_t_empty(lexer_iterator);) {
    lexer_token_t lexer_token =
        LANGNAME_lexer_iterator_step(lexer, &lexer_iterator);
    if (!(lexer_token.id < LANGNAME_token_count) ||
        (lexer_token.id == LANGNAME_token_UNKNOWN)) {
      printf("Token %u unknown<%zu>:", iter_count++, lexer_token.id);
      lexer_token_dump(lexer_token);
      return 2;
    } else {
      printf("Token %u: ", iter_count++);
      lexer_token_dump(lexer_token);
    }

    if (LANGNAME_lexer_discard_token(lexer_token.id)) {
      continue;
    }

    token lemon_token = token_create(LANGNAME_token_names[lexer_token.id],
                                     lexer_token.value, lexer_token.width);

    if (!LANGNAME_lexer_identifier_valid_token(lexer_token.id)) {
      printf("Token invalid 0x%lx\n", lexer_token.id);
      return 3;
    }


    LANGNAME_parser_lemon_parse(&parser, (int)lexer_token.id, lemon_token);

    union UPPERNAME_BISON_STYPE tmp = {
      .token = lemon_token,
    };
    // Note: This requires bison to be using the same tokentype numbers as the lexer
    // i.e. [1, N) in definition order
    int bison_rc = LANGNAME_bison_push_parse(bison_parser, lexer_token.id, &tmp, &bison_arg, ctx);

    // possible this expects to be called as do {} while (rc == YYPUSH_MORE)
    switch(bison_rc)
    {
      case 0: break;
      case YYPUSH_MORE: break;
      case 1: {
        printf("bison syntax error on token %zu\n", lexer_token.id);
        lexer_token_dump(lexer_token);
        break;
      }
      default: {
        printf("bison rc %u on token %zu\n", bison_rc, lexer_token.id);
        break;
      }
    }
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

  {
    if (ptree_is_failure(bison_arg))
      {
        printf("Bison failed\n");
      }
    else
      {
        LANGNAME_ptree_as_xml(&stack_libc, stdout, bison_arg);
      }
  }
  

  LANGNAME_parser_lemon_finalize(&parser);
  LANGNAME_bison_pstate_delete(bison_parser);
  LANGNAME_lexer_destroy(lexer);
  free(in);

  return 0;
}]=]

local res = template:gsub('LANGNAME', args[1]):gsub('UPPERNAME', args[1]:upper())

print(res)
os.exit(0)
