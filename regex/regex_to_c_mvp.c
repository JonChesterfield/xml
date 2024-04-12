#include "regex_to_c_mvp.h"
#include "regex.lexer.h"
#include "regex_cache.h"

/*
 * Generates clunky C representation of the arguments
 * Parameterised on a state struct which is passed to each transition
 * and a boolean function which mutates that struct as it wishes
 * Start of the generated C is said struct and boolean, then a potentially
 * large number of functions encoding the regex transitions, then a match()
 * function with a predictable name that calls the entry state
 * The generated lexer util is similar in structure - it builds a set of C
 * functions for each regex and dispatches to the requested one This is not the
 * efficient way to build a lexer, hence mvp
 */

typedef struct {
  const char *lang;
  const char *prefix;
  const char *state_type;
  const char *params;
  lexer_t lexer;
  lexer_token_t (*lexer_iterator_step)(lexer_t, lexer_iterator_t *);
} functor_state;

static int regex_to_c_mvp_cache(functor_state data, regex_cache_t *cache,
                                ptree_context context, ptree regex);

int regex_to_c_mvp(const char *lang, const char *prefix, ptree_context context,
                   ptree regex) {
  regex_cache_t cache = regex_cache_create();
  if (!regex_cache_valid(cache)) {
    return 1;
  }

  lexer_t lexer = regex_lexer_create();
  lexer_token_t (*lexer_iterator_step)(lexer_t, lexer_iterator_t *) =
      regex_lexer_iterator_step;
  if (!regex_lexer_valid(lexer)) {
    return 2;
  }

  char *state_type = 0;
  {
    const char *fmt = "struct %s_regex_state";
    int size = snprintf(NULL, 0, fmt, lang);
    state_type = malloc(size + 1);
    sprintf(state_type, fmt, lang);
  }

  char *params = 0;
  {
    const char *fmt = "const char* start, const char *iter, const char* end, "
                      "%s state";
    int size = snprintf(NULL, 0, fmt, state_type);
    params = malloc(size + 1);
    sprintf(params, fmt, state_type);
  }

  functor_state data = {
      .lang = lang,
      .prefix = prefix,
      .params = params,
      .state_type = state_type,
      .lexer = lexer,
      .lexer_iterator_step = lexer_iterator_step,
  };

  int rc = regex_to_c_mvp_cache(data, &cache, context, regex);
  free(params);
  free(state_type);
  regex_cache_destroy(cache);
  return rc;
}

static int declare(regex_cache_t *cache, stringtable_index_t current, void *d) {
  functor_state *data = (functor_state *)d;
  const char *prefix = data->prefix;
  const char *params = data->params;
  const char *state_type = data->state_type;
  printf("static %s %s_regex_%lu(%s);\n", state_type, prefix, current.value,
         params);
  return 0;
}

static int define(regex_cache_t *cache, stringtable_index_t current, void *d) {
  const bool use_ranges = true;
  const bool use_all_equal = true;

  functor_state *data = (functor_state *)d;
  const char *prefix = data->prefix;
  const char *lang = data->lang;
  const char *params = data->params;
  const char *state_type = data->state_type;

  const char *regstr = stringtable_lookup(&cache->strtab, current);
  size_t reglen = stringtable_lookup_size(&cache->strtab, current);

  enum regex_cache_lookup_properties props =
      regex_cache_lookup_properties(cache, current);
  if (regex_properties_is_failure(props)) {
    return 1;
  }

  bool nullable = regex_properties_is_nullable(props);
  bool empty_set = regex_properties_is_empty_set(props);

  // if nullable, set match_end
  // if empty_set, goto failure

  printf("static %s %s_regex_%lu(%s)\n"
         "{\n"
         "  // Compiled: %.*s\n"
         "  const bool is_nullable = %s;\n"
         "  const bool is_empty_set = %s;\n"
         "  const bool is_end_of_input = iter == end;\n"
         "\n"
         "  if (%s_regex_early_return(&state, is_nullable, is_empty_set, "
         "is_end_of_input))\n"
         "  {\n"
         "    return state;\n"
         "  }\n"
         "  const char* next = iter+1;\n"
         "",
         state_type, prefix, current.value, params, (int)reglen, regstr,
         nullable ? "true" : "false", empty_set ? "true" : "false", lang);

  stringtable_index_t zeroth = regex_cache_calculate_derivative_using_lexer(
      data->lexer, data->lexer_iterator_step, cache, current, (uint8_t)0);
  if (!stringtable_index_valid(zeroth)) {
    return 3;
  }

  if (use_all_equal) {
    bool all_equal = true;
    for (size_t i = 1; i < 256; i++) {
      stringtable_index_t ith = regex_cache_calculate_derivative_using_lexer(
          data->lexer, data->lexer_iterator_step, cache, current, (uint8_t)i);
      all_equal &= zeroth.value == ith.value;
    }

    if (all_equal) {
      printf("  return %s_regex_%lu(start, next, end, state);\n", prefix,
             zeroth.value);
      printf("}\n");
      return 0;
    }
  }

  {
    // hardcoding byte for now
    printf("  unsigned char cursor = *iter;\n"
           "  switch(cursor)\n"
           "  {\n");

    if (use_ranges) {
      size_t range_start = 0;
      for (size_t i = 0; i < 256; i++) {
        stringtable_index_t ith = regex_cache_calculate_derivative_using_lexer(
            data->lexer, data->lexer_iterator_step, cache, current, (uint8_t)i);
        stringtable_index_t next = regex_cache_calculate_derivative_using_lexer(
            data->lexer, data->lexer_iterator_step, cache, current,
            (uint8_t)(i + 1));

        if (i == 255 || (ith.value != next.value)) {
          if (range_start == i) {
            printf("    case %zu: return %s_regex_%lu(start, next, end, "
                   "state);\n",
                   i, prefix, ith.value);
          } else {
            printf("    case %zu ... %zu: return %s_regex_%lu(start, next, "
                   "end, state);\n",
                   range_start, i, prefix, ith.value);
          }
          range_start = i + 1;
        }
      }
    } else {

      for (size_t i = 0; i < 256; i++) {
        stringtable_index_t ith = regex_cache_calculate_derivative_using_lexer(
            data->lexer, data->lexer_iterator_step, cache, current, (uint8_t)i);

        stringtable_index_t next = regex_cache_calculate_derivative_using_lexer(
            data->lexer, data->lexer_iterator_step, cache, current,
            (uint8_t)(i + 1));

        if (!stringtable_index_valid(ith)) {
          return 3;
        }

        if (i == 255 || (ith.value != next.value)) {
          printf("    case %zu: return %s_regex_%lu(start, next, end, "
                 "state);\n",
                 i, prefix, ith.value);
        } else {
          printf("    case %zu:\n", i);
        }
      }
    }

    printf("  }\n  /* needs to be unreachable */ return state;\n");
  }

  printf("}\n");
  return 0;
}

static int prolog(regex_cache_t *cache, stringtable_index_t current, void *d) {
  functor_state *data = (functor_state *)d;
  const char *prefix = data->prefix;
  const char *params = data->params;
  const char *state_type = data->state_type;

  printf("static %s %s_regex_match(%s);\n", state_type, prefix, params);
  return 0;
}

static int epilog(regex_cache_t *cache, stringtable_index_t current, void *d) {
  functor_state *data = (functor_state *)d;
  const char *prefix = data->prefix;
  const char *params = data->params;
  const char *state_type = data->state_type;
  printf("static %s %s_regex_match(%s)\n"
         "{\n"
         "  return %s_regex_%lu(start, start, end, state);\n"
         "}\n",
         state_type, prefix, params, prefix, current.value);

  return 0;
}

static int regex_to_c_mvp_cache(functor_state data, regex_cache_t *cache,
                                ptree_context context, ptree regex) {

  stringtable_index_t root =
      regex_cache_insert_regex_ptree(cache, context, regex);
  if (!stringtable_index_valid(root)) {
    return 2;
  }

  int rc = 0;

  rc = prolog(cache, root, &data);
  if (rc != 0) {
    return rc;
  }

  printf("\n");

  rc = regex_cache_traverse_using_lexer(data.lexer, data.lexer_iterator_step,
                                        cache, root, declare, &data);
  if (rc != 0) {
    return rc;
  }

  printf("\n");

  rc = regex_cache_traverse_using_lexer(data.lexer, data.lexer_iterator_step,
                                        cache, root, define, &data);
  if (rc != 0) {
    return rc;
  }

  printf("\n");

  rc = epilog(cache, root, &data);
  if (rc != 0) {
    return rc;
  }

  return 0;
}

int regexes_to_c_lexer_mvp(ptree_context context, const char *lang,
                           ptree (*parser)(ptree_context, const char *),
                           size_t token_count, const char **tokens,
                           const char **regexes) {

  printf("#include <stddef.h>\n");
  printf("#include \"%s.lexer.h\"\n", lang);

  printf("\n");
  printf("struct %s_regex_state\n"
         "{\n"
         "  size_t match_end;\n"
         "};\n",
         lang);
  printf("static bool %s_regex_early_return(struct %s_regex_state *state , "
         "bool nullable, bool empty_set, bool end_of_input)\n"
         "{\n"
         "  if (end_of_input) return true;\n"
         "  if (nullable) state->match_end = state->match_end+1;\n"
         "  if (empty_set) return true;\n"
         "  return false;\n"
         "}\n",
         lang, lang);
  printf("\n");

  // The first one will be UNKNOWN
  for (size_t i = 0; i < token_count; i++) {
    ptree regex = parser(context, regexes[i]);
    if (ptree_is_failure(regex)) {
      printf("Failed to parse regex %zu: %s\n", i, regexes[i]);
      return 1;
    }

    if (regex_to_c_mvp(lang, tokens[i], context, regex) != 0) {
      return 1;
    }
  }

  printf("size_t %s_ith_regex_matches_start(\n"
         "  const char* start, const char* end, size_t i)\n"
         "{\n"
         "  const char* iter = start;\n"
         "  struct %s_regex_state state = {0};\n"
         "  switch(i)\n"
         "  {\n",
         lang, lang);

  for (size_t i = 0; i < token_count; i++) {
    const char *n = tokens[i];
    printf("    case %s_token_%s:\n"
           "    {\n"
           "      state = %s_regex_match(start, iter, end, state);\n"
           "      break;\n"
           "    }\n",
           lang, n, n);
  }

  printf("    default: break; \n"
         "  }\n"
         "  return state.match_end;\n"
         "}\n");

  return 0;
}
