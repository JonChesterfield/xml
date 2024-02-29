#include "regex_string.h"

#include "../tools/stack.libc.h"

#include "regex.declarations.h"
#include "regex.h"
#include "regex.lexer.declarations.h"

#include "regex_parser.lemon.h"

#include "regex_parser.lemon.t"

struct regex_to_char_sequence_type {
  arena_module mod;
  arena_t *arena;
};

static bool arena_push_char(arena_module mod, arena_t *arena, char c) {
  uint64_t pos = arena_allocate(mod, arena, 1, 1);
  if (pos == UINT64_MAX) {
    return false;
  }
  char *l = (char *)arena_base_address(mod, *arena) + pos;
  *l = c;
  return true;
}

static bool arena_push_char_string(arena_module mod, arena_t *arena,
                                   const char *c, size_t N) {
  uint64_t pos = arena_allocate(mod, arena, N, 1);
  if (pos == UINT64_MAX) {
    return false;
  }
  printf("push char string %s\n", c);
  char *l = (char *)arena_base_address(mod, *arena) + pos;
  memcpy(l, c, N);
  return true;
}

static int regex_to_char_sequence_pre(ptree tree, uint64_t depth, void *p) {
  struct regex_to_char_sequence_type *data =
      (struct regex_to_char_sequence_type *)p;
  (void)data;

  if (!arena_request_available(data->mod, data->arena, 8)) {
    return 1;
  }

  printf("in pre time\n");
  
  uint64_t id = regex_ptree_identifier(tree);
  
  if (regex_grouping_id_is_single_byte(id)) {
    uint8_t byte = regex_grouping_extract_single_byte(id);

    char tmp[8];
    int rc = snprintf(tmp, sizeof(tmp), "%02x", (unsigned)byte);
    if (rc < 0) {
      printf("snprintf fail\n");
      return 1;
    }

    printf("push char string 2\n");
    arena_push_char_string(data->mod, data->arena, tmp, 2);
    return 0;
  }

  switch (id) {
  case regex_grouping_empty_set: {
    arena_push_char(data->mod, data->arena,
                    regex_regexes[regex_token_EMPTYSET][1]);
    return 0;
  }

  case regex_grouping_empty_string: {
    arena_push_char(data->mod, data->arena,
                   regex_regexes[regex_token_EMPTYSTRING][1]);
    return 0;
  }

  case regex_grouping_concat:
  case regex_grouping_kleene:
  case regex_grouping_or:
  case regex_grouping_and:
  case regex_grouping_not: {
    arena_push_char(data->mod, data->arena, '(');

    switch (id) {
    case regex_grouping_concat: {
      arena_push_char(data->mod, data->arena,
                      regex_regexes[regex_token_CONCAT][1]);
      return 0;
    }

    case regex_grouping_kleene: {
      arena_push_char(data->mod, data->arena,
                      regex_regexes[regex_token_KLEENE][1]);
      return 0;
    }

    case regex_grouping_or: {
      arena_push_char(data->mod, data->arena, regex_regexes[regex_token_OR][1]);
      return 0;
    }

    case regex_grouping_and: {
      arena_push_char(data->mod, data->arena, regex_regexes[regex_token_AND][1]);
      return 0;
    }

    case regex_grouping_not: {
      arena_push_char(data->mod, data->arena, regex_regexes[regex_token_NOT][1]);
      return 0;
    }

    default:
      return 1;
    }
  }

  default:
    return 1;
  }
}

static int regex_to_char_sequence_elt(ptree tree, uint64_t depth, void *p) {
  struct regex_to_char_sequence_type *data =
      (struct regex_to_char_sequence_type *)p;
  (void)tree;
  (void)depth;
  (void)p;
  (void)data;
  return 0;
}

static int regex_to_char_sequence_post(ptree tree, uint64_t depth, void *p) {
  struct regex_to_char_sequence_type *data =
      (struct regex_to_char_sequence_type *)p;
  (void)data;

  uint64_t id = regex_ptree_identifier(tree);

  if (!arena_request_available(data->mod, data->arena, 8)) {
    return 1;
  }
  
  if (regex_grouping_id_is_single_byte(id)) {
    return 0;
  }

  switch (id) {
  case regex_grouping_empty_set:
  case regex_grouping_empty_string:
    return 0;

  case regex_grouping_concat:
  case regex_grouping_kleene:
  case regex_grouping_or:
  case regex_grouping_and:
  case regex_grouping_not: {

    arena_push_char(data->mod, data->arena, ')');
    return 0;
  }

  default:
    return 1;
  }
}

int regex_to_char_sequence(arena_module mod, arena_t *arena, ptree val) {

  const struct stack_module_ty *stackmod = &stack_libc;

  struct regex_to_char_sequence_type v = {
      .mod = mod,
      .arena = arena,
  };

  uint64_t depth = 0;
  int r = regex_ptree_traverse(stackmod, val, depth, regex_to_char_sequence_pre,
                               &v, regex_to_char_sequence_elt, &v,
                               regex_to_char_sequence_post, &v);

  if (!arena_request_available(mod, arena, 8)) {
    return 1;
  }

  char *next = arena_next_address(mod, *arena);

  arena_push_char(mod, arena, 0);
  
  return r;
}

ptree regex_from_char_sequence(ptree_context ctx, const char *bytes, size_t N) {
  const bool verbose = true;

  lexer_t lexer = regex_lexer_create();
  if (!regex_lexer_valid(lexer)) {
    if (verbose) {
      fprintf(stderr, "Failed to make lexer\n");
    }
    return ptree_failure();
  }

  ptree res = ptree_failure();

  struct regex_parser_s parser_state;
  regex_parser_type *parser = (regex_parser_type *)&parser_state;

  regex_parser_initialize(parser, ctx);

  for (lexer_iterator_t lexer_iterator = lexer_iterator_t_create(bytes, N);
       !lexer_iterator_t_empty(lexer_iterator);) {
    lexer_token_t lexer_token =
        regex_lexer_iterator_step(lexer, &lexer_iterator);
    if (!(lexer_token.id < regex_token_count) ||
        (lexer_token.id == regex_token_UNKNOWN)) {
      if (verbose) {
        fprintf(stderr, "Unknown lexer token %lu\n", lexer_token.id);
      }
      goto done;
    }

    token lemon_token = token_create(regex_token_names[lexer_token.id],
                                     lexer_token.value, lexer_token.width);

    if (!regex_ptree_identifier_valid_token(lexer_token.id)) {
      fprintf(stderr, "Invalid lexer token id %lu\n", lexer_token.id);
      goto done;
    }

    token_dump(lemon_token);

    regex_parser_parse(parser, (int)lexer_token.id, lemon_token);
  }

  fprintf(stderr, "parser tree done\n");
  res = regex_parser_tree(parser);

done:;
  regex_parser_finalize(parser);
  regex_lexer_destroy(lexer);

  return res;
}
