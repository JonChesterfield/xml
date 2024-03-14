#ifndef PTREE_MALLOC_ALLOCATOR
#define PTREE_MALLOC_ALLOCATOR
#include "ptree.h"

#include "stack.libc.h"
#include "intstack.h"

// Simple malloc based allocator for ptree
#include <stdlib.h>

_Static_assert(sizeof(struct ptree_context_ty) == sizeof(intstack_t), "");

static intstack_t ptree_malloc_retrieve_context_stack(ptree_context ctx) {
  intstack_t stack;
  __builtin_memcpy( &stack.state, &ctx->state, sizeof(intstack_t));
  return stack;
}

static void  ptree_malloc_assign_context_stack(ptree_context ctx, intstack_t stack) {
 __builtin_memcpy( &ctx->state, &stack.state, sizeof(intstack_t));
}

// ptree context is a pointer to a struct containing a void*
// might make that uint64
static ptree_context ptree_malloc_ptree_create_context(void) {
  struct ptree_context_ty *res =
      malloc(sizeof(struct ptree_context_ty));
  if (res) {
    intstack_t stack = intstack_create(0);
    if (!intstack_valid(stack)) {
      free(res);
      return 0;
    }
    ptree_malloc_assign_context_stack(res, stack);   
  }
  return res;
}


static bool ptree_malloc_ptree_valid_context(ptree_context ctx) {
  return ctx != 0;
}

static void ptree_malloc_ptree_destroy_context(ptree_context ctx) {
  if (!ctx) {
    return;
  }
  if (!ctx->state) {
    return;
  }

  intstack_t stack = ptree_malloc_retrieve_context_stack(ctx);
  
  size_t size = intstack_size(stack);
  for (size_t i = 0; i < size; i++)
    {
      uint64_t p = intstack_peek(stack);
      intstack_drop(&stack);
      free((void*)p);
      
    }
  intstack_destroy(stack);
  free(ctx);  
}


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


static ptree_malloc_ptree ptree_malloc_ptree_allocate(ptree_context ctx,
                                                      uint64_t id, size_t N) {
  size_t size =
      sizeof(struct ptree_malloc_ptree_ty) + N * sizeof(ptree_malloc_ptree);
  ptree_malloc_ptree r = malloc(size);
  if (r) {
    {
      intstack_t stack = ptree_malloc_retrieve_context_stack(ctx);
      if (!intstack_reserve(&stack, intstack_size(stack) + 1))
        {
          printf("intstack reserve %zu failed, dying\n", intstack_size(stack) + 1);
          free(r);
          return 0;
        }
      intstack_push(&stack, (uint64_t)r);
      ptree_malloc_assign_context_stack(ctx, stack); // stack ptr may have been changed
    }
    
    r->id = id;
    r->is_token = false;
    r->token_value = "";
    r->token_width = 0;
    r->Nelements = N;
    for (size_t i = 0; i < N; i++) {
      r->elements[i] = ptree_failure();
    }
  }
  
  return r;
}


static ptree_malloc_ptree
make_ptree_malloc_ptree_from_token(ptree_context ctx, uint64_t id,
                                   const char *token_value,
                                   size_t token_width) {
  ptree_malloc_ptree r = ptree_malloc_ptree_allocate(ctx, id, 0);
  if (r) {
    r->is_token = true;
    r->token_value = token_value;
    r->token_width = token_width;
  }
  return r;
}


#endif
