#include "../tools/io_buffer.h"
#include <stdio.h>

#include "regex.h"
#include "regex.ptree.h"
#include "regex_string.h"

#include "../tools/stack.libc.h"

// Would like to read a string on stdin and print it as the xml representation
// of the parsed output
int main()
{
  __attribute__((__cleanup__(free)))
    io_buffer* in = file_to_io_buffer(stdin);
  if (!in)
    {
      return 1;
    }

  ptree_context ctx = regex_ptree_create_context();
  ptree res = regex_from_char_sequence(ctx, in->data, in->N);
  if (ptree_is_failure(res))
    {
      return 2;
    }

  regex_ptree_as_xml(&stack_libc, stdout, res);
  
  return 0;
}
