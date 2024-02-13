// The gnu source needs to be before any includes to be effective
// probably means it should be restricted to a separate translation
// unit which only exports memfd, possibly wrapped in an interface
// that deals with write / fdopen etc

#define _GNU_SOURCE

#include "arith.declarations.h"
#include "arith.parse.h"

#include "arith.ptree.h"
#include "../tools/io_buffer.h"

#include "../tools/stack.libc.h"

#include <sys/mman.h>
#include <unistd.h>

const char test_input[] = " (4 * (12 - 1)   + 2 )";





// There is to be an instance of this parser thing.
static arith_parser_type * arith_global_parser(void)
{
  // Two different ways of allocating the incomplete type
#if 0
  extern arith_parser_type arith_global_lemon_parser;
  __asm__("\t.type arith_global_lemon_parser,@object\n"
          ".pushsection .data\n"
          "\t.global arith_global_lemon_parser\n"
          "\t.p2align 3\n"
          "arith_global_lemon_parser:\n"
          "\t.zero 4032\n"
          "\t.size arith_global_lemon_parser, 4032\n"
          ".popsection\n"
          );

  return &arith_global_lemon_parser;
#else
  static struct arith_parser_s arith_global_lemon_parser;

  return (arith_parser_type*) (&arith_global_lemon_parser);
#endif
}



int main()
{
  int fd = memfd_create("test_input", 0);

  // write test data to the the "file"
  write(fd, test_input,
        sizeof(test_input) - 1 /*lexer does not like the nul */);

  // reset file descriptor to the start of the "file"
  lseek(fd, 0, SEEK_SET);

  FILE *f = fdopen(fd, "r+");
  if (!f)
    {
      return 40;
    }
  if (ferror(f))
    {
      return 41;
    }

  ptree_context ptree_context = arith_ptree_create_context();
  arith_parser_initialize(arith_global_parser(), ptree_context);
  
  printf("calling file to io buffer\n");
  io_buffer *toplevel = file_to_io_buffer(f);
  if (!toplevel)
    {
      printf("toplevel null\n");
      return 42;
    }

  // has unknown at 0
  lexer_t lexer = arith_lexer_create();
  if (!arith_lexer_valid(lexer))
    {
      return 43;
    }

  for (lexer_iterator_t lexer_iterator =
           lexer_iterator_t_create(toplevel->data, toplevel->N);
       !lexer_iterator_t_empty(lexer_iterator);)
    {
      lexer_token_t lexer_token = arith_lexer_iterator_step(lexer, &lexer_iterator);
      if (!(lexer_token.id < arith_token_count))
        {
          printf("Got ID %zu, largest known %u\n", lexer_token.id,
                 arith_token_count);
        }
      assert(lexer_token.id < arith_token_count);

      token lemon_token = token_create(arith_token_names[lexer_token.id],
                                       lexer_token.value, lexer_token.width);

      if (lexer_token.id == arith_token_UNKNOWN)
        {
          printf("unknown token\n");
          token_as_xml(stdout, lemon_token);
          return 46;
        }
      else
        {
          token_as_xml(stdout, lemon_token);
          printf("\n");
        }
      arith_parser_parse(arith_global_parser(), (int)lexer_token.id, lemon_token);
    }

  printf("lexer done\n");
  ptree res = arith_parser_tree(arith_global_parser());

  printf("parser done\n");

  arith_ptree_as_xml(&stack_libc, stdout, res);
  printf("\n");

  // Will call deallocate on nodes
  arith_parser_finalize(arith_global_parser());
  arith_ptree_destroy_context(ptree_context);  
  arith_lexer_destroy(lexer);
  free(toplevel);

  fclose(f);
  close(fd);

  return 0;
}
