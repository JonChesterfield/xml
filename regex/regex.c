#include "regex.ptree.h"
#include "regex.declarations.h"

#include "../tools/stack.libc.h"

#include <stdio.h>

int main(void)
{
  ptree_context ctx = regex_ptree_create_context();
  ptree expr = regex_make_and(ctx,
                              regex_make_byte_00(ctx),
                              regex_make_byte_cd(ctx));
  
  regex_ptree_as_xml(&stack_libc, stdout, expr);
  fprintf(stdout, "\n");

  
  
  regex_ptree_destroy_context(ctx);
}
