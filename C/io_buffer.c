#include "io_buffer.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

io_buffer* file_to_io_buffer(FILE* f)
{
  // avoid fseek as it doens't work on stdin
  if (!f || ferror(f))
    {
      return NULL;
    }

  io_buffer* buffer = NULL;
  unsigned blocksize = 8192;
  unsigned base = sizeof(io_buffer) + 1;

  for (uint64_t blocks_read = 0;;)
    {
      // this is linear alloc, not unit tested
      {
        void* tmp = realloc(buffer, base + ((blocks_read + 1) * blocksize));
        if (!tmp)
          {
            free(buffer);
            return NULL;
          }
        buffer = tmp;
      }

      size_t r = fread(&buffer->data[blocksize * blocks_read], 1, blocksize, f);
      blocks_read++;

      assert(r <= blocksize);
      if (r < blocksize)
        {
          if (feof(f))
            {
              // reached end of file
              uint64_t N = ((blocks_read - 1) * blocksize) + r;
              buffer->N = N;
              buffer->data[N] = '\0';
              return buffer;  // realloc down?
            }
          else if (ferror(f))
            {
              free(buffer);
              return NULL;
            }
        }
    }
}

int io_buffer_to_file(const io_buffer* buf, FILE* out)
{
  // ret 0 on success
  size_t r = fwrite(buf->data, 1, buf->N, out);
  if (r < buf->N)
    {
      // Failed, somehow
      return 1;
    }
  return 0;
}
