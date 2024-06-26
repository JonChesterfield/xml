#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED

#include <stdbool.h>
#include <stddef.h> // size_t
#include <stdio.h>

#include "../tools/ascii.h"

#ifdef __cplusplus
extern "C" {
#endif

// Tokens are assigned values in [1, N)
// with 0 used as a default catch/all
// Lemon expects this to be an int, e.g. the second argument to parse is an int
// It should probably be unsigned but leaving it as-is for now
// The integer is used as a shorthand to refer to the ith token definition
typedef int token_index;

typedef struct {
  token_index name;
  // Non-owning pointers into the contiguous memory representing whatever
  // is being lexed. The re2 regex engine works on a string view based on char*.
  const char *value_start;
  const char *value_end;
} token;

static inline token token_create(token_index name, const char *start,
                                 const char *end) {
  return (token){
      .name = name,
      .value_start = start,
      .value_end = end,
  };
}

static inline token token_create_novalue(token_index name) {
  static const char empty[1] = {'\0'};
  return token_create(name, empty, empty);
}

static inline size_t token_width(token s) {
  return s.value_end - s.value_start;
}

static inline bool token_empty(token s) { return token_width(s) == 0; }

static inline bool token_chars_all_alphanumeric(token s)
{
  size_t w = token_width(s);
  bool all = true;
  for (size_t i = 0; i < w; i++)
    {
      char c = s.value_start[i];
      all &= ascii_char_to_type(c) == alphanumeric;
    }
  return all;
}
  
static inline void token_dump(token s) {
  printf("<Token name = %d value = \"", s.name);
  const char *c = s.value_start;
  while (c != s.value_end) {
    printf("%c", *c++);
  }
  printf("\" />\n");
}

static inline void token_named_dump(token s, const char **token_names) {
  printf("<Token name = \"%s\" value = \"", token_names[s.name]);
  const char *c = s.value_start;
  while (c != s.value_end) {
    printf("%c", *c++);
  }
  printf("\" />\n");
}

#ifdef __cplusplus
}
#endif

#endif
