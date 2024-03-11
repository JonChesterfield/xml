#include "intstack.h"

#include "EvilUnit/EvilUnit.h"

#include "stack.h"
#include "stack.libc.h"

static stack_module mod = &stack_libc;

intstack_t intstack_create(uint64_t capacity) {
  return (intstack_t){
      .state = stack_create(mod, capacity),
  };
}

void intstack_destroy(intstack_t s) { stack_destroy(mod, s.state); }

bool intstack_valid(intstack_t s) { return s.state != 0; }

bool intstack_equal(intstack_t l, intstack_t r) {
  uint64_t N = intstack_size(l);
  if (N != intstack_size(r)) {
    return false;
  }
  bool eq = true;
  for (uint64_t i = 0; i < N; i++) {
    eq &= (*stack_impl_ith(l.state, i)) == (*stack_impl_ith(r.state, i));
  }
  return eq;
}

intstack_t intstack_clone(intstack_t s) {
  uint64_t N = intstack_size(s);
  intstack_t res = intstack_create(N);
  if (intstack_valid(res)) {
    for (uint64_t i = N; i-- > 0;) {
      stack_push(mod, res.state, *stack_impl_ith(s.state, i));
    }
  }
  return res;
}

uint64_t intstack_size(intstack_t s) { return stack_size(mod, s.state); }

uint64_t intstack_capacity(intstack_t s) {
  return stack_capacity(mod, s.state);
}

bool intstack_reserve(intstack_t *s, uint64_t n) {
  void *r = stack_reserve(mod, s->state, n);
  if (r) {
    s->state = r;
    return true;
  } else {
    return false;
  }
}

void intstack_push(intstack_t *s, uint64_t v) {
  stack_push_assuming_capacity(mod, s->state, v);
}

uint64_t intstack_peek(intstack_t s) { return stack_peek(mod, s.state); }

void intstack_drop(intstack_t *s) { stack_drop(mod, s->state); }

void intstack_dump(intstack_t s) {
  uint64_t N = intstack_size(s);
  printf("intstack[%lu] {\n", N);
  for (uint64_t i = 0; i < N; i++) {
    uint64_t k = *stack_impl_ith(s.state, i);
    printf("  [%lu] = %lu\n", i, k);
  }
  printf("}\n");
}

static MODULE(create_destroy) {
  TEST("size 0") {
    intstack_t s = intstack_create(0);
    CHECK(intstack_size(s) == 0);
    CHECK(intstack_capacity(s) == 0);
    intstack_destroy(s);
  }

  TEST("non-zero") {
    intstack_t s = intstack_create(4);
    CHECK(intstack_size(s) == 0);
    CHECK(intstack_capacity(s) >= 4);
    intstack_destroy(s);
  }

  TEST("non-zero, odd") {
    intstack_t s = intstack_create(5);
    CHECK(intstack_size(s) == 0);
    CHECK(intstack_capacity(s) >= 5);
    intstack_destroy(s);
  }
}

static MODULE(sequence) {
  intstack_t s = intstack_create(8);
  CHECK(intstack_valid(s));
  CHECK(intstack_size(s) == 0);
  CHECK(intstack_available(s) == 8);
  CHECK(intstack_capacity(s) == 8);

  TEST("seq") {
    intstack_push(&s, 42);

    CHECK(intstack_size(s) == 1);
    CHECK(intstack_available(s) == 7);
    CHECK(intstack_capacity(s) == 8);

    CHECK(intstack_peek(s) == 42);

    intstack_drop(&s);

    CHECK(intstack_size(s) == 0);
    CHECK(intstack_available(s) == 8);
    CHECK(intstack_capacity(s) == 8);
  }

  TEST("copy/equal") {
    intstack_push(&s, 42);
    intstack_push(&s, 81);
    CHECK(intstack_peek(s) == 81);

    intstack_t t = intstack_create(4);
    CHECK(intstack_valid(t));

    intstack_push(&t, 42);
    intstack_push(&t, 81);

    CHECK(intstack_size(s) == 2);
    CHECK(intstack_size(t) == 2);

    CHECK(intstack_equal(s, t));

    intstack_t v = intstack_clone(s);

    CHECK(intstack_size(v) == 2);

    if (intstack_valid(v)) {
      CHECK(intstack_equal(s, v));
      CHECK(intstack_peek(v) == 81);
      intstack_destroy(v);
    }

    intstack_destroy(t);
  }

  intstack_destroy(s);
}

MODULE(intstack) {
  DEPENDS(create_destroy);
  DEPENDS(sequence);
}
