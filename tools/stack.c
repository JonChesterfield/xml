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

#include "stack.brk.h"

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
