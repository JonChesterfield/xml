#include "arena.h"

#include <setjmp.h>

#include "EvilUnit/EvilUnit.h"

#include "arena.libc.h"


static jmp_buf death_buffer;

static const struct arena_module_ty arena_libc_jmp = ARENA_MODULE_INIT(arena_libc, contract_unit_test, &death_buffer);

static const arena_module mod = &arena_libc_jmp;


static MODULE(create_destroy) {

  TEST("sanity check jmp buf")
    {
      CHECK(contract_jmpbuf_is_zero(&death_buffer));
      contract_jmpbuf_set_zero(&death_buffer);
      CHECK(contract_jmpbuf_is_zero(&death_buffer));
    }
  
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

  TEST("fail a precondition")
    {
      arena_t a = {0};
      DEATH(death_buffer, arena_destroy(mod, a));
    }

#if 0
  TEST("fail without the death macro")
    {
      // This was checking the behaviour of failing a contract after forgetting
      // to wrap it in DEATH() - previously segv, now a stderr message that
      // a jmp_buf is zero and an early exit 1
      arena_t a = {0};
      arena_destroy(mod, a);
    }
#endif


}

MAIN_MODULE() { DEPENDS(create_destroy); }
