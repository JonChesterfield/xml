#ifndef IO_BUFFER_H_INCLUDED
#define IO_BUFFER_H_INCLUDED

// Thinking of inlining them here
// got a fread wrapper that runs in chunks, fwrite one currently doesn't

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  size_t N;
  char data[];
} io_buffer;

// returned buffer is null if anything went wrong
// otherwise it will need to be free'd

io_buffer *file_to_io_buffer(FILE * /*open, rb*/);
int io_buffer_to_file(const io_buffer *,
                      FILE * /*open, wb*/);  // ret 0 on success


#endif
