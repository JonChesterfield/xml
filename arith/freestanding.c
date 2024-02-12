#include "../tools/stack.brk.h"
#include "arith.declarations.h"
#include "arith.lemon.h"

#include <stddef.h>

#include </usr/include/x86_64-linux-gnu/asm/unistd_64.h>

enum {
  STDIN_FILENO = 0,
  STDOUT_FILENO = 1,
  STDERR_FILENO = 2,
};

int64_t read(int fd, void *buf, size_t count) {
  uint64_t ubuf;
  __builtin_memcpy(&ubuf, &buf, 8);
  return syscall6(__NR_read, fd, ubuf, count, 0, 0, 0);
}

int64_t write(int fd, void *buf, size_t count) {
  uint64_t ubuf;
  __builtin_memcpy(&ubuf, &buf, 8);
  return syscall6(__NR_write, fd, ubuf, count, 0, 0, 0);
}

void *read_all_stdin(void *base) {
  unsigned char *memory = base;
  unsigned char *cursor = base;

  // return 0 on failure, somewhat past the end on success
  enum { chunk = 64 };

  for (;;) {
    while ((memory - cursor) < chunk) {
      memory = brk_realloc(memory, chunk);
      if (!base) {
        return 0;
      }
    }

    int64_t r = read(STDIN_FILENO, cursor, chunk);
    if (r < 0) {
      return 0;
    }

    cursor = cursor + r;

    if (r == 0) {
      // reached end
      return cursor;
    }
  }
}

int main() {
  void *base = brk(0);
  if (!base) {
    return 1;
  }

  void *stdin_begins = base;

  void *after_stdin = read_all_stdin(base);
  if (!after_stdin) {
    return 1;
  }

  size_t stdin_length =
      (unsigned char *)after_stdin - (unsigned char *)stdin_begins;

  lexer_t lexer = arith_lexer_create();
  if (!arith_lexer_valid(lexer)) {
    return 1;
  }

  void *pParser = &arith_global_lemon_parser;
  arith_LemonInit(pParser);

  for (lexer_iterator_t lexer_iterator =
           lexer_iterator_t_create(stdin_begins, stdin_length);
       !lexer_iterator_t_empty(lexer_iterator);) {
    lexer_token_t lexer_token =
        arith_lexer_iterator_step(lexer, &lexer_iterator);
    if (!(lexer_token.id < arith_token_count)) {
      printf("Got ID %zu, largest known %u\n", lexer_token.id,
             arith_token_count);
    }
    assert(lexer_token.id < arith_token_count);

    token lemon_token = token_create(arith_token_names[lexer_token.id],
                                     lexer_token.value, lexer_token.width);

    if (lexer_token.id == arith_token_UNKNOWN) {
      return 4;
    }

    arith_Lemon(pParser, (int)lexer_token.id, lemon_token, NULL);
  }

  token tok = token_create_novalue("");

  arith_Lemon(pParser, 0, tok, NULL);

  arith_LemonFinalize(pParser);
  arith_lexer_destroy(lexer);

  return 0;
}
