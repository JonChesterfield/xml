#ifndef PTREE_MALLOC_ALLOCATOR
#define PTREE_MALLOC_ALLOCATOR
#include "ptree.h"

// Simple malloc based allocator for ptree
#include <stdlib.h>

struct ptree_malloc_ptree_ty;
struct ptree_malloc_ptree_ty {
  uint64_t id;
  bool is_token;
  const char *token_value;
  size_t token_width;
  size_t Nelements;
  ptree elements[];
};

typedef struct ptree_malloc_ptree_ty *ptree_malloc_ptree;

static ptree_malloc_ptree ptree_to_ptree_malloc_ptree(ptree x) {
  ptree_malloc_ptree res;
  __builtin_memcpy(&res, &x.state, 8);
  return res;
}

static ptree ptree_malloc_ptree_to_ptree(ptree_malloc_ptree x) {
  ptree res;
  __builtin_memcpy(&res.state, &x, 8);
  return res;
}

// Contains the most recently allocated ptree.
struct ptree_malloc_ptree_context_ty {
  ptree_malloc_ptree root;
};

typedef struct ptree_malloc_ptree_context_ty *ptree_malloc_ptree_context;

_Static_assert(sizeof(ptree_context) == 8, "");
_Static_assert(sizeof(ptree_malloc_ptree_context) == 8, "");

static ptree_malloc_ptree_context
ptree_context_to_ptree_malloc_ptree_context(ptree_context ctx) {
  ptree_malloc_ptree_context res;
  __builtin_memcpy(&res, &ctx, 8);
  return res;
}

static ptree_context
ptree_malloc_ptree_context_to_ptree_context(ptree_malloc_ptree_context ctx) {
  ptree_context res;
  __builtin_memcpy(&res, &ctx, 8);
  return res;
}

static ptree_malloc_ptree ptree_malloc_ptree_allocate(uint64_t id, size_t N) {
  size_t size =
      sizeof(struct ptree_malloc_ptree_ty) + N * sizeof(ptree_malloc_ptree);
  ptree_malloc_ptree r = malloc(size);
  if (r) {
    r->id = id;
    r->token_value = "";
    r->token_width = 0;
    r->Nelements = N;
    for (size_t i = 0; i < N; i++) {
      r->elements[i] = ptree_malloc_ptree_to_ptree(0);
    }
  }
  return r;
}

static void ptree_malloc_ptree_deallocate(ptree_malloc_ptree p) {
  if (!p)
    return;
  // todo, doable in constant space
  for (size_t i = 0; i < p->Nelements; i++) {
    ptree_malloc_ptree_deallocate(ptree_to_ptree_malloc_ptree(p->elements[i]));
  }

  free(p);
}

static ptree_malloc_ptree
make_ptree_malloc_ptree_from_token(ptree_malloc_ptree_context ctx, uint64_t id,
                                   const char *token_value,
                                   size_t token_width) {
  ptree_malloc_ptree r = ptree_malloc_ptree_allocate(id, 0);
  if (r) {
    r->is_token = true;
    r->token_value = token_value;
    r->token_width = token_width;
    ctx->root = r;
  }
  return r;
}

static ptree_malloc_ptree
make_ptree_malloc_ptree_from_N_ptree(ptree_malloc_ptree_context ctx,
                                     uint64_t id, size_t N, ptree *elts) {
  ptree_malloc_ptree r = ptree_malloc_ptree_allocate(id, N);
  if (r) {
    r->is_token = false;
    for (size_t i = 0; i < N; i++) {
      r->elements[i] = elts[i];
    }
    ctx->root = r;
  }
  return r;
}

static ptree_context ptree_malloc_ptree_create_context(void) {
  ptree_malloc_ptree_context res =
      malloc(sizeof(struct ptree_malloc_ptree_context_ty));
  if (res) {
    res->root = 0;
  }
  return ptree_malloc_ptree_context_to_ptree_context(res);
}

static void ptree_malloc_ptree_destroy_context(ptree_context ctx) {
  ptree_malloc_ptree_context s =
      ptree_context_to_ptree_malloc_ptree_context(ctx);
  if (s) {
    ptree_malloc_ptree_deallocate(s->root);
    free(s);
  }
}

#endif
