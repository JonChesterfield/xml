#ifndef STACK_BRK_H_INCLUDED
#define STACK_BRK_H_INCLUDED

#include "syscall.h"

#include "stack.common.h"

static void *brk(void *addr) {
  uint64_t x;
  __builtin_memcpy(&x, &addr, 8);
  uint64_t n = 12;
  uint64_t r = syscall1(n, x);
  __builtin_memcpy(&addr, &r, 8);
  return addr;
}

static void *brk_realloc(void *b, size_t n) {
  void *request = (char *)b + n;
  void *got = brk(request);
  if (got == request) {
    return got;
  }
  // Otherwise brk failed
  return 0;
}

static void *brk_malloc(size_t n) {
  void *base = brk(0);
  if (base == 0) {
    return 0;
  }
  return brk_realloc(base, n);
}

static void brk_free(void *b, size_t n) {
  void *request = (char *)b - n;
  void *got = brk(request);
  if (got == request) {
  }
}

static void *stack_brk_create(void) {
  uint64_t *alloc = brk_malloc(2 * sizeof(uint64_t));
  if (alloc) {
    alloc[cap_offset] = 0;
    alloc[size_offset] = 0;
  }
  return alloc;
}

static void stack_brk_destroy(void *s) {

  size_t cap = stack_impl_capacity(s);
  size_t bytes = 8 * (cap + 2);
  brk_free(s, bytes);
}

static void *stack_brk_reserve(void *s, size_t N) {
  if (N <= stack_impl_capacity(s)) {
    return s;
  }

  uint64_t *r = brk_realloc(s, (2 + N) * sizeof(uint64_t));
  if (r) {
    r[cap_offset] = N;
  }
  return r;
}

static const struct stack_module_ty stack_brk = {
    .create = stack_brk_create,
    .destroy = stack_brk_destroy,
    .size = stack_impl_size,
    .capacity = stack_impl_capacity,
    .reserve = stack_brk_reserve,
    .push = stack_impl_push,
    .peek = stack_impl_peek,
    .drop = stack_impl_drop,
};

#endif
