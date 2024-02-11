#include "arith.ptree.h"
#include "arith.declarations.h"
#include "../tools/ptree_malloc_allocator.h"

static ptree_context arith_impl_ptree_create_context(void) {
  return ptree_malloc_ptree_create_context();
}

static void arith_impl_ptree_destroy_context(ptree_context ctx) {
  ptree_malloc_ptree_destroy_context(ctx);
}

_Static_assert(arith_token_UNKNOWN == 0, "");

static bool arith_impl_ptree_identifier_valid_token(uint64_t id) {
  switch (id) {
  case 0: {
    return false;
  }
  case arith_token_PLUS:
  case arith_token_MINUS:
  case arith_token_TIMES:
  case arith_token_DIVIDE:
  case arith_token_MODULO:
  case arith_token_LPAREN:
  case arith_token_RPAREN:
  case arith_token_INTEGER:
  case arith_token_WHITESPACE: {
    return true;
  }
  default: {
    return false;
  }
  }
}

static bool arith_impl_ptree_identifier_valid_expression(uint64_t id) {
  if (id == 0) {
    return false;
  }
  if (arith_impl_ptree_identifier_valid_token(id)) {
    return false;
  }
  // todo: eumerate expressions from lemon/bison parser
  return true;
}

static const char * arith_impl_ptree_identifier_token_maybe_name(uint64_t id) {
  switch (id) {
  case arith_token_PLUS: return "PLUS";
  case arith_token_MINUS: return "MINUS";
  case arith_token_TIMES: return "TIMES";
  case arith_token_DIVIDE: return "DIVIDE";
  case arith_token_MODULO: return "MODULO";
  case arith_token_LPAREN: return "LPAREN";
  case arith_token_RPAREN: return "RPAREN";
  case arith_token_INTEGER: return "INTEGER";
  case arith_token_WHITESPACE: return "WHITESPACE";
  default: {
    return 0;
  }
  }
}

static const char * arith_impl_ptree_identifier_expression_maybe_name(uint64_t id) {
  if (arith_impl_ptree_identifier_valid_expression(id)) {    
    return "expr";
  }
  return 0;
}

static size_t arith_impl_ptree_identifier_minimum_elements(uint64_t id) {
  return 0;
}
static size_t arith_impl_ptree_identifier_maximum_elements(uint64_t id) {
  return SIZE_MAX;
}

static uint64_t arith_impl_ptree_identifier(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->id;
}

static bool arith_impl_ptree_is_token(ptree arg) {
  if (ptree_is_failure(arg)) { return false; }
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->is_token;
}

static bool arith_impl_ptree_is_expression(ptree arg) {
  if (ptree_is_failure(arg)) { return false; }
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return !p->is_token;
}

static const char *arith_impl_ptree_token_value(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->token_value;
}
static size_t arith_impl_ptree_token_width(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->token_width;
}
static ptree arith_impl_ptree_from_token(ptree_context ctx, uint64_t id,
                                         const char *value, size_t width) {
  ptree_malloc_ptree_context s = ptree_context_to_ptree_malloc_ptree_context(ctx);
  return ptree_malloc_ptree_to_ptree(make_ptree_malloc_ptree_from_token(s, id, value, width));
}
static size_t arith_impl_ptree_expression_elements(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->Nelements;
}

static ptree arith_impl_ptree_expression_element(ptree arg, size_t index) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->elements[index];
}

static inline ptree arith_impl_ptree_expression_construct(ptree_context ctx,
                                                          uint64_t id,
                                                          uint64_t N,
                                                          ptree *elts) {
  ptree_malloc_ptree_context s = ptree_context_to_ptree_malloc_ptree_context(ctx);
  ptree_malloc_ptree r = make_ptree_malloc_ptree_from_N_ptree(s, id, N, elts);
  return ptree_malloc_ptree_to_ptree(r);
}

static ptree arith_impl_ptree_expression_append(ptree_context ctx,
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
      arith_impl_ptree_expression_construct(ctx, base->id, before + 1, arr);
  if (ptree_to_ptree_malloc_ptree(res)) {
    for (uint64_t i = 0; i < before; i++) {
      base->elements[i] = ptree_malloc_ptree_to_ptree(0);
    }
    ptree_malloc_ptree_deallocate(base);
  }
  return res;
}

static const ptree_module arith_module =
    PTREE_INSTANTIATE_MODULE_INITIALIZER(arith_impl);

PTREE_INSTANTIATE_DEFINE(arith, arith_module);
