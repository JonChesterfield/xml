#include "stack.h"

#define EVILUNIT_USE_STDIO 1
#include "EvilUnit/EvilUnit.h"

#include <stdlib.h>

static void *stack_libc_create(void) {
  uint64_t *res = malloc(2 * sizeof(uint64_t));
  if (res) {
    res[0] = 0;
    res[1] = 0;
  }
  return res;
}

static void stack_libc_destroy(void *s) { free(s); }

static size_t stack_libc_size(void *s) {
  uint64_t *stack = (uint64_t *)s;
  return stack[1];
}

static size_t stack_libc_capacity(void *s) {
  uint64_t *stack = (uint64_t *)s;
  return stack[0];
}

static void *stack_libc_reserve(void *s, size_t N) {
  if (N <= stack_libc_capacity(s)) {
    return s;
  }
  uint64_t *r = realloc(s, (2 + N) * sizeof(uint64_t));
  if (r) {
    r[0] = N;
  }
  return r;
}

static void stack_libc_push(void *s, uint64_t v) {
  uint64_t *stack = (uint64_t *)s;
  size_t sz = stack_libc_size(s);
  stack[2 + sz] = v;
  stack[1] = sz + 1;
}

static uint64_t stack_libc_peek(void *s) {
  uint64_t *stack = (uint64_t *)s;
  size_t sz = stack_libc_size(s);
  return stack[2 + sz - 1];
}

static void stack_libc_drop(void *s) {
  uint64_t *stack = (uint64_t *)s;
  size_t sz = stack_libc_size(s);
  stack[1] = sz - 1;
}

const struct stack_module_ty stack_libc = {
    .create = stack_libc_create,
    .destroy = stack_libc_destroy,
    .size = stack_libc_size,
    .capacity = stack_libc_capacity,
    .reserve = stack_libc_reserve,
    .push = stack_libc_push,
    .peek = stack_libc_peek,
    .drop = stack_libc_drop,

};

static MODULE(create_destroy) {
  const stack_module mod = &stack_libc;
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
  const stack_module mod = &stack_libc;
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
