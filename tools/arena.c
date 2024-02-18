#include "arena.h"

#include <setjmp.h>

#include "EvilUnit/EvilUnit.h"

#include "arena.libc.h"


static jmp_buf death_buffer;

static const struct arena_module_ty arena_libc_jmp = ARENA_MODULE_INIT(arena_libc, contract_active, &death_buffer);

static const arena_module mod = &arena_libc_jmp;


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

  #if 0
  TEST("fail a precondition")
    {
      arena_t a = {0};
      DEATH(death_buffer, arena_destroy(mod, a));
    }
  #endif

  TEST("fail without the death macro")
    {
      arena_t a = {0};
      arena_destroy(mod, a);
    }

}

MAIN_MODULE() { DEPENDS(create_destroy); }
