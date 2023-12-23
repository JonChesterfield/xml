#include "io_buffer.h"
#include <stdio.h>
#include <stdbool.h>

static   const char * header =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "\n"
    "<RawText xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
    "\n"
    "<![CDATA[";

static   const char * footer =
  "]]></RawText>";

int main(void)
{
  io_buffer * in = file_to_io_buffer(stdin);
  if (!in) { return 1; }

  if  (fputs(header , stdout) == EOF) { return 1; }
  
  if (io_buffer_to_file(in, stdout) != 0) { return 2; }

  if (fputs(footer, stdout) == EOF) { return 3; }

  return 0;
}
