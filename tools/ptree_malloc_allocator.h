#ifndef PTREE_MALLOC_ALLOCATOR
#define PTREE_MALLOC_ALLOCATOR
#include "ptree.h"

#include "stack.libc.h"

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

_Static_assert(sizeof(ptree_context) == 8, "");


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


static ptree_malloc_ptree
make_ptree_malloc_ptree_from_token(ptree_context ctx, uint64_t id,
                                   const char *token_value,
                                   size_t token_width) {
  ptree_malloc_ptree r = ptree_malloc_ptree_allocate(id, 0);
  if (r) {
    r->is_token = true;
    r->token_value = token_value;
    r->token_width = token_width;

    if (stack_request_available(&stack_libc, &ctx->state, 1))
      {
        stack_push_assuming_capacity(&stack_libc, ctx->state, (uint64_t)r);
      }
    else
      {
        free(r);
        return 0;
      }
  }
  return r;
}

static ptree_malloc_ptree
make_ptree_malloc_ptree_from_N_ptree(ptree_context ctx,
                                     uint64_t id, size_t N, ptree *elts) {
  ptree_malloc_ptree r = ptree_malloc_ptree_allocate(id, N);
  if (r) {
    r->is_token = false;
    for (size_t i = 0; i < N; i++) {
      r->elements[i] = elts[i];
    }

    if (stack_request_available(&stack_libc, &ctx->state, 1))
      {
        stack_push_assuming_capacity(&stack_libc, ctx->state, (uint64_t)r);
      }
    else
      {
        free(r);
        return 0;
      } 
  }
  return r;
}

static ptree_context ptree_malloc_ptree_create_context(void) {
  struct ptree_context_ty *res =
      malloc(sizeof(struct ptree_context_ty));
  if (res) {
    res->state = stack_create(&stack_libc, 0);
  }
  return res;
}

static bool ptree_malloc_ptree_valid_context(ptree_context ctx) {
  return ctx;
}

static void ptree_malloc_ptree_destroy_context(ptree_context ctx) {
  if (!ctx) {
    return;
  }
  if (!ctx->state) {
    return;
  }

  // Leak for now, the transition to stack based tracking
  // to handle DAGs isn't passing valgrind
  return;
  
  size_t size = stack_size(&stack_libc, ctx->state);
  for (size_t i = 0; i < size; i++)
    {
      uint64_t p = stack_pop(&stack_libc, ctx->state);
      free((void*)p);
    }
  stack_destroy(&stack_libc, ctx->state);
  
}

#endif
