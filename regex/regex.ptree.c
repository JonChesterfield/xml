#include "regex.h"
#include "regex.ptree.h"
#include "regex.declarations.h"
#include "../tools/ptree_malloc_allocator.h"
#include "regex.lexer.h"

#include <assert.h>

//
// This is a mix of wiring up an allocator and specialising to a language
// Likely to be simpler if the two dimensions are separated
//

static ptree_context regex_impl_ptree_create_context(void) {
  return ptree_malloc_ptree_create_context();
}

static bool regex_impl_ptree_valid_context(ptree_context ctx) {
  return ptree_malloc_ptree_valid_context(ctx);
}

static void regex_impl_ptree_destroy_context(ptree_context ctx) {
  ptree_malloc_ptree_destroy_context(ctx);
}

_Static_assert(regex_token_UNKNOWN == 0, "");

static bool regex_impl_ptree_identifier_valid_token(uint64_t id) {
  // Same as lexer, probably always must be
  return regex_lexer_identifier_valid_token(id);
}

static bool regex_impl_ptree_identifier_valid_expression(uint64_t id) {
  switch (id) {
#include "regex_grouping_byte_cases.data"
  case regex_grouping_empty_set:
  case regex_grouping_empty_string:
  case regex_grouping_any_char:
  case regex_grouping_concat:
  case regex_grouping_kleene:
  case regex_grouping_or:
  case regex_grouping_and:
  case regex_grouping_not: {
    return true;
  }
  default:
    return false;
  }

  return false;
}

static const char *regex_impl_ptree_identifier_token_maybe_name(uint64_t id) {
  if (regex_impl_ptree_identifier_valid_token(id))
    {
      return regex_token_names[id];      
    }
  else
    {
      return 0;
    }
}


enum {regex_ptree_byte_print_array_stride = 4};
static const char regex_ptree_byte_print_array[256 * regex_ptree_byte_print_array_stride];
#include "regex.ptree.byte_print_array.data"

static const char *
regex_impl_ptree_identifier_expression_maybe_name(uint64_t id) {
  switch (id) {   
  case regex_grouping_empty_set:
    return "empty_set";
  case regex_grouping_empty_string:
    return "empty_string";
  case regex_grouping_any_char:
    return "any";
  case regex_grouping_concat:
    return "cat";
  case regex_grouping_kleene:
    return "kleene";
  case regex_grouping_or:
    return "or";
  case regex_grouping_and:
    return "and";
  case regex_grouping_not:
    return "not";
// Could change .data to be X macro style (defaulting to empty) to fold this arithmetic
#include "regex_grouping_byte_cases.data"
   {
    size_t offset = regex_ptree_byte_print_array_stride * regex_grouping_extract_single_byte(id);
    return &regex_ptree_byte_print_array[offset];
  } 
  default:
    return 0;
  }
}

static size_t regex_impl_ptree_identifier_minimum_elements(uint64_t id) {
  return 0;
}
static size_t regex_impl_ptree_identifier_maximum_elements(uint64_t id) {
  return SIZE_MAX;
}

static uint64_t regex_impl_ptree_identifier(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->id;
}

static bool regex_impl_ptree_is_token(ptree arg) {
  if (ptree_is_failure(arg)) {
    return false;
  }
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->is_token;
}

static bool regex_impl_ptree_is_expression(ptree arg) {
  if (ptree_is_failure(arg)) {
    return false;
  }
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return !p->is_token;
}

static const char *regex_impl_ptree_token_value(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->token_value;
}
static size_t regex_impl_ptree_token_width(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->token_width;
}
static ptree regex_impl_ptree_from_token(ptree_context ctx, uint64_t id,
                                         const char *value, size_t width) {
  return ptree_malloc_ptree_to_ptree(
      make_ptree_malloc_ptree_from_token(ctx, id, value, width));
}
static size_t regex_impl_ptree_expression_elements(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->Nelements;
}

static ptree regex_impl_ptree_expression_element(ptree arg, size_t index) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->elements[index];
}

static inline ptree regex_impl_ptree_expression_create_uninitialised(ptree_context ctx,
                                                                     uint64_t id,
                                                                     uint64_t N) {
  ptree_malloc_ptree r = ptree_malloc_ptree_allocate(ctx, id, N);
  if (r) {
    r->is_token = false;
    for (size_t i = 0; i < N; i++) {
      assert(ptree_is_failure(r->elements[i]));
    }
  }
  return ptree_malloc_ptree_to_ptree(r);
}

static inline void regex_impl_ptree_expression_initialise_element(ptree base,
                                                                  size_t index,
                                                                  ptree elt)
{
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(base);
  p->elements[index] = elt;
}

static const ptree_module regex_module;

static ptree regex_impl_ptree_expression_append(ptree_context ctx,
                                                ptree basearg,
                                                ptree elementarg) {
  ptree_malloc_ptree base = ptree_to_ptree_malloc_ptree(basearg);

  uint64_t before = base->Nelements;
  ptree arr[before + 1];
  for (uint64_t i = 0; i < before; i++) {
    arr[i] = base->elements[i];
  }
  arr[before] = elementarg;

  return
    ptree_expression_construct(&regex_module, ctx, base->id, before + 1, arr);
}

static const ptree_module regex_module =
    PTREE_INSTANTIATE_MODULE_INITIALIZER(regex_impl);

PTREE_INSTANTIATE_DEFINE(regex, regex_module);



