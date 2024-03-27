#include "io_buffer.h"
#include <stdio.h>

static unsigned char hex_to_num(char x) {
  switch (x) {
  case '0':
    return 0;
  case '1':
    return 1;
  case '2':
    return 2;
  case '3':
    return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  case '8':
    return 8;
  case '9':
    return 9;
  case 'a':
  case 'A':
    return 10;
  case 'b':
  case 'B':
    return 11;
  case 'c':
  case 'C':
    return 12;
  case 'd':
  case 'D':
    return 13;
  case 'e':
  case 'E':
    return 14;
  case 'f':
  case 'F':
    return 15;

  default:
    return 255;
  }
}

int main(void) {
  io_buffer *in = file_to_io_buffer(stdin);
  if (!in) {
    return 1;
  }

  bool odd = (in->N % 2) == 1;

  if (odd) {
    char end = in->data[in->N - 1];
    // tolerate a trailing 0 or newline after otherwise valid input
    if (end == '\0' || end == '\n') {
      in->N--;
      odd = false;
    }
  }

  if (odd) {
    return 2;
  }

  io_buffer *out = new_io_buffer(in->N / 2);
  if (!out) {
    free(in);
    return 3;
  }

  for (size_t i = 0; i < in->N; i += 2) {
    unsigned char hi = hex_to_num(in->data[i]);
    unsigned char lo = hex_to_num(in->data[i + 1]);

    if (hi < 16 && lo < 16) {
      unsigned char both = hi * 16u + lo;
      out->data[i / 2] = both;
    } else {
      free(in);
      free(out);
      return 4;
    }
  }

  int rc = io_buffer_to_file(out, stdout);

  free(in);
  free(out);

  if (rc != 0) {
    return 5;
  }

  return 0;
}
