#include "arena.h"

#include "EvilUnit/EvilUnit.h"

#include "arena.libc.h"

static const arena_module mod = &arena_libc;

static MODULE(create_destroy) {
  TEST("size 0") {
    arena_t a = arena_create(mod, 0);
    CHECK(arena_valid(mod, a));
    CHECK(arena_size(mod, a) == 0);
    CHECK(arena_capacity(mod, a) == 0);
    arena_destroy(mod, a);
  }

  TEST("non-zero") {
    arena_t a = arena_create(mod, 4);
    CHECK(arena_valid(mod, a));
    CHECK(arena_size(mod, a) == 0);
    CHECK(arena_capacity(mod, a) == 4);
    arena_destroy(mod, a);
  }

  TEST("non-zero, odd") {
    arena_t a = arena_create(mod, 5);
    CHECK(arena_valid(mod, a));
    CHECK(arena_size(mod, a) == 0);
    CHECK(arena_capacity(mod, a) == 5);
    arena_destroy(mod, a);
  }
}

MAIN_MODULE() {
  DEPENDS(create_destroy);
}
