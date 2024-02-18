#include "arena.h"

#include <setjmp.h>

#include "EvilUnit/EvilUnit.h"

#include "arena.libc.h"

static const arena_module mod = &arena_libc;

static _Thread_local jmp_buf death_buffer;

void jump_to_death(void) {
  // passing 0 here is probably UB but might be treated as passing 1
  longjmp(death_buffer, 1);
}

#define DEATH(X)                                                               \
  if (setjmp(death_buffer) == 0) {                                             \
    /* Jump frame is set up. Do the task:*/                                    \
    int check_resolved_to_this = (X);                                          \
    (void)check_resolved_to_this;                                              \
    /* If we get here it failed to jump through the death buffer */            \
    evilunit_implementation_check(evilunit_internal_state, 0, __LINE__, #X);   \
  } else {                                                                     \
    /* If we got here, indicate success*/                                      \
    evilunit_implementation_check(evilunit_internal_state, 1, __LINE__, #X);   \
  }

static MODULE(create_destroy) {
  TEST("size 0") {
    arena_t a = arena_create(mod, 0);
    CHECK(arena_valid(mod, a));
    CHECK(arena_size(mod, a) == 0);
    CHECK(arena_capacity(mod, a) == 0);
    arena_destroy(mod, a);
  }

  TEST("try dying") {
    //     DEATH(1);// should fail, didn't jump to death

    DEATH((jump_to_death(), 1));
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

MAIN_MODULE() { DEPENDS(create_destroy); }
