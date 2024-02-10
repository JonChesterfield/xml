#include "arith.ptree.h"
#include "arith.declarations.h"

#include <stdlib.h>

struct arith_ptree_ty;
struct arith_ptree_ty {
  uint64_t id;
  bool is_token;
  const char *token_value;
  size_t token_width;
  size_t Nelements;
  ptree elements[]; // because arith_ptree_ty is still incomplete
};

typedef struct arith_ptree_ty *arith_ptree;

static arith_ptree ptree_to_arith_ptree(ptree x) {
  arith_ptree res;
  __builtin_memcpy(&res, &x.state, 8);
  return res;
}

static ptree arith_ptree_to_ptree(arith_ptree x) {
  ptree res;
  __builtin_memcpy(&res.state, &x, 8);
  return res;
}

// Contains the most recently allocated ptree.
struct arith_ptree_context_ty {
  arith_ptree root;
};

typedef struct arith_ptree_context_ty *arith_ptree_context;

_Static_assert(sizeof(ptree_context) == 8, "");
_Static_assert(sizeof(arith_ptree_context) == 8, "");

static arith_ptree_context
ptree_context_to_arith_ptree_context(ptree_context ctx) {
  arith_ptree_context res;
  __builtin_memcpy(&res, &ctx, 8);
  return res;
}

static ptree_context
arith_ptree_context_to_ptree_context(arith_ptree_context ctx) {
  ptree_context res;
  __builtin_memcpy(&res, &ctx, 8);
  return res;
}

static arith_ptree arith_ptree_allocate(uint64_t id, size_t N) {
  size_t size = sizeof(struct arith_ptree_ty) + N * sizeof(arith_ptree);
  arith_ptree r = malloc(size);
  if (r) {
    r->id = id;
    r->token_value = "";
    r->token_width = 0;
    r->Nelements = N;
    for (size_t i = 0; i < N; i++) {
      r->elements[i] = arith_ptree_to_ptree(0);
    }
  }
  return r;
}

static void arith_ptree_deallocate(arith_ptree p) {
  if (!p)
    return;
  // todo, doable in constant space
  for (size_t i = 0; i < p->Nelements; i++) {
    arith_ptree_deallocate(ptree_to_arith_ptree(p->elements[i]));
  }

  free(p);
}

static arith_ptree make_arith_ptree_from_token(arith_ptree_context ctx,
                                               uint64_t id,
                                               const char *token_value,
                                               size_t token_width) {
  arith_ptree r = arith_ptree_allocate(id, 0);
  if (r) {
    r->is_token = true;
    r->token_value = token_value;
    r->token_width = token_width;
    ctx->root = r;
  }
  return r;
}

static arith_ptree make_arith_ptree_from_N_ptree(arith_ptree_context ctx,
                                                 uint64_t id, size_t N,
                                                 ptree *elts) {
  arith_ptree r = arith_ptree_allocate(id, N);
  if (r) {
    r->is_token = false;
    for (size_t i = 0; i < N; i++) {
      r->elements[i] = elts[i];
    }
    ctx->root = r;
  }
  return r;
}

static ptree_context arith_impl_ptree_create_context(void) {
  arith_ptree_context res = malloc(sizeof(struct arith_ptree_context_ty));
  if (res) {
    res->root = 0;
  }
  return arith_ptree_context_to_ptree_context(res);
}

static void arith_impl_ptree_destroy_context(ptree_context ctx) {
  arith_ptree_context s = ptree_context_to_arith_ptree_context(ctx);
  if (s) {
    arith_ptree_deallocate(s->root);
    free(s);
  }
}

_Static_assert(arith_token_UNKNOWN == 0, "");

static bool arith_impl_ptree_identifier_is_token(uint64_t id) {
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

static bool arith_impl_ptree_identifier_is_expression(uint64_t id) {
  if (id == 0) {
    return false;
  }
  if (arith_impl_ptree_identifier_is_token(id)) {
    return false;
  }
  // todo: eumerate expressions from lemon/bison parser
  return true;
}

static size_t arith_impl_ptree_identifier_minimum_elements(uint64_t id) {
  return 0;
}
static size_t arith_impl_ptree_identifier_maximum_elements(uint64_t id) {
  return SIZE_MAX;
}

static uint64_t arith_impl_ptree_identifier(ptree arg) {
  arith_ptree p = ptree_to_arith_ptree(arg);
  return p->id;
}

static const char *arith_impl_ptree_token_value(ptree arg) {
  arith_ptree p = ptree_to_arith_ptree(arg);
  return p->token_value;
}
static size_t arith_impl_ptree_token_width(ptree arg) {
  arith_ptree p = ptree_to_arith_ptree(arg);
  return p->token_width;
}
static ptree arith_impl_ptree_from_token(ptree_context ctx, uint64_t id,
                                         const char *value, size_t width) {
  arith_ptree_context s = ptree_context_to_arith_ptree_context(ctx);
  return arith_ptree_to_ptree(make_arith_ptree_from_token(s, id, value, width));
}
static size_t arith_impl_ptree_expression_elements(ptree arg) {
  arith_ptree p = ptree_to_arith_ptree(arg);
  return p->Nelements;
}
static size_t arith_impl_ptree_expression_capacity(ptree arg) {
  arith_ptree p = ptree_to_arith_ptree(arg);
  return p->Nelements;
}
static ptree arith_impl_ptree_expression_element(ptree arg, size_t index) {
  arith_ptree p = ptree_to_arith_ptree(arg);
  return p->elements[index];
}

static inline ptree arith_impl_ptree_expression_construct(ptree_context ctx,
                                                          uint64_t id,
                                                          uint64_t N,
                                                          ptree *elts) {
  arith_ptree_context s = ptree_context_to_arith_ptree_context(ctx);
  arith_ptree r = make_arith_ptree_from_N_ptree(s, id, N, elts);
  return arith_ptree_to_ptree(r);
}

static ptree arith_impl_ptree_expression_append(ptree_context ctx,
                                                ptree basearg,
                                                ptree elementarg) {
  arith_ptree base = ptree_to_arith_ptree(basearg);

  uint64_t before = base->Nelements;
  ptree arr[before + 1];
  for (uint64_t i = 0; i < before; i++) {
    arr[i] = base->elements[i];
  }
  arr[before] = elementarg;

  ptree res =
      arith_impl_ptree_expression_construct(ctx, base->id, before + 1, arr);
  if (ptree_to_arith_ptree(res)) {
    for (uint64_t i = 0; i < before; i++) {
      base->elements[i] = arith_ptree_to_ptree(0);
    }
    arith_ptree_deallocate(base);
  }
  return res;
}

static const ptree_module arith_module =
    PTREE_INSTANTIATE_MODULE_INITIALIZER(arith_impl);

PTREE_INSTANTIATE_DEFINE(arith, arith_module);
