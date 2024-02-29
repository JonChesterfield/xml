#include "regex.h"
#include "regex.ptree.h"
#include "regex.declarations.h"

#include "../tools/ptree_malloc_allocator.h"

#include "regex.lexer.declarations.h"

static ptree_context regex_impl_ptree_create_context(void) {
  return ptree_malloc_ptree_create_context();
}

static void regex_impl_ptree_destroy_context(ptree_context ctx) {
  ptree_malloc_ptree_destroy_context(ctx);
}

_Static_assert(regex_token_UNKNOWN == 0, "");

static bool regex_impl_ptree_identifier_valid_token(uint64_t id) {
  // No tokens defined at present
  return (id > regex_token_UNKNOWN) && (id < regex_token_count);
}

static bool regex_impl_ptree_identifier_valid_expression(uint64_t id) {
  if (id < regex_token_count) {
    return false;
  }
  
  switch (id) {
  case regex_grouping_empty_set:
  case regex_grouping_empty_string:
  case regex_grouping_concat:
  case regex_grouping_kleene:
  case regex_grouping_or:
  case regex_grouping_and:
  case regex_grouping_not:
  case regex_grouping_range: {
    return true;
  }
  default:
    break;
  }

  if ((regex_grouping_byte_00 <= id) && (id <= regex_grouping_byte_ff)) {
    return true;
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
    return "Oset";
  case regex_grouping_empty_string:
    return "Ostr";
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
  default:
    break;
  }

  if (regex_grouping_id_is_single_byte(id)) {
    size_t offset = regex_ptree_byte_print_array_stride * regex_grouping_extract_single_byte(id);
    return &regex_ptree_byte_print_array[offset];
  }

  return 0;
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
  ptree_malloc_ptree_context s =
      ptree_context_to_ptree_malloc_ptree_context(ctx);
  return ptree_malloc_ptree_to_ptree(
      make_ptree_malloc_ptree_from_token(s, id, value, width));
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
  ptree_malloc_ptree_context s =
      ptree_context_to_ptree_malloc_ptree_context(ctx);
  ptree_malloc_ptree r = ptree_malloc_ptree_allocate(id, N);
  if (r) {
    r->is_token = false;
    for (size_t i = 0; i < N; i++) {
      r->elements[i] = ptree_failure();
    }
    s->root = r;
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

static inline ptree regex_impl_ptree_expression_construct(ptree_context ctx,
                                                          uint64_t id,
                                                          uint64_t N,
                                                          ptree *elts) {
  ptree_malloc_ptree_context s =
      ptree_context_to_ptree_malloc_ptree_context(ctx);
  ptree_malloc_ptree r = make_ptree_malloc_ptree_from_N_ptree(s, id, N, elts);
  return ptree_malloc_ptree_to_ptree(r);
}

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

  ptree res =
      regex_impl_ptree_expression_construct(ctx, base->id, before + 1, arr);
  if (ptree_to_ptree_malloc_ptree(res)) {
    for (uint64_t i = 0; i < before; i++) {
      base->elements[i] = ptree_malloc_ptree_to_ptree(0);
    }
    ptree_malloc_ptree_deallocate(base);
  }
  return res;
}

static const ptree_module regex_module =
    PTREE_INSTANTIATE_MODULE_INITIALIZER(regex_impl);

PTREE_INSTANTIATE_DEFINE(regex, regex_module);



