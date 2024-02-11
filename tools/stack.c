#include "stack.h"

#define STACK_FREESTANDING() 0

#if STACK_FREESTANDING()
#define EVILUNIT_USE_STDIO 0
#else
#define EVILUNIT_USE_STDIO 1
#endif

#include "EvilUnit/EvilUnit.h"

#include "stack.common.h"

#if !STACK_FREESTANDING()
#include "stack.libc.h"
#endif

static uint64_t syscall6(uint64_t n, uint64_t a0, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4, uint64_t a5) {
  uint64_t ret = 0;
  register uint64_t r10 __asm__("r10") = a3;
  register uint64_t r8 __asm__("r8") = a4;
  register uint64_t r9 __asm__("r9") = a5;

  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"(n), "D"(a0), "S"(a1), "d"(a2), "r"(r10), "r"(r8),
                     "r"(r9)
                   : "rcx", "r11", "memory");

  return ret;
}

static void *brk(void *addr) {
  uint64_t x;
  __builtin_memcpy(&x, &addr, 8);
  uint64_t n = 12;
  uint64_t u;
  u = *(&u); // suppress the uninitialized warning
  uint64_t r = syscall6(n, x, u, u, u, u, u);
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

#if STACK_FREESTANDING()
static const stack_module mod = &stack_brk;
#else
static const stack_module mod = &stack_libc;
#endif

static MODULE(create_destroy) {
  TEST("size 0") {
    void *s = stack_create(mod, 0);
    CHECK(stack_size(mod, s) == 0);
    CHECK(stack_capacity(mod, s) == 0);
    stack_destroy(mod, s);
  }

  TEST("non-zero") {
    void *s = stack_create(mod, 4);
    CHECK(stack_size(mod, s) == 0);
    CHECK(stack_capacity(mod, s) >= 4);
    stack_destroy(mod, s);
  }

  TEST("non-zero, odd") {
    void *s = stack_create(mod, 5);
    CHECK(stack_size(mod, s) == 0);
    CHECK(stack_capacity(mod, s) >= 5);
    stack_destroy(mod, s);
  }
}

static MODULE(push_pop_sequence) {
  void *s = stack_create(mod, 0);

  TEST("check size/cap") {
    CHECK(stack_size(mod, s) == 0);
    CHECK(stack_capacity(mod, s) == 0);
  }

  TEST("push one") {
    void *r = stack_push(mod, s, 42);
    if (r) {
      CHECK(stack_size(mod, r) == 1);
      CHECK(stack_peek(mod, r) == 42);
      CHECK(stack_pop(mod, r) == 42);
      CHECK(stack_size(mod, r) == 0);
      s = r;
    } else {
      CHECK(stack_size(mod, s) == 0);
    }
  }

  TEST("push two") {
    void *r = stack_reserve(mod, s, 2);
    if (r) {
      CHECK(stack_size(mod, r) == 0);
      CHECK(stack_capacity(mod, r) >= 2);

      CHECK(r == stack_push(mod, r, 5));
      CHECK(stack_size(mod, r) == 1);
      CHECK(r == stack_push(mod, r, 10));
      CHECK(stack_size(mod, r) == 2);

      CHECK(10 == stack_pop(mod, r));
      CHECK(stack_size(mod, r) == 1);

      CHECK(5 == stack_pop(mod, r));

      CHECK(stack_size(mod, r) == 0);
      s = r;
    }
  }

  TEST("push/pop multiple") {
    // test case assumes alloc succeeds
    s = stack_push(mod, s, 4);
    s = stack_push(mod, s, 5);
    s = stack_push(mod, s, 6);
    stack_pop(mod, s);
    s = stack_push(mod, s, 7);
    s = stack_push(mod, s, 8);
    stack_pop(mod, s);
    stack_pop(mod, s);
    stack_pop(mod, s);
    stack_pop(mod, s);
  }

  stack_destroy(mod, s);
}

MAIN_MODULE() {
  DEPENDS(create_destroy);
  DEPENDS(push_pop_sequence);
}
