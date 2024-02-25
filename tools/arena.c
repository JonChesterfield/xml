#include "arena.h"

#include "EvilUnit/EvilUnit.h"

#include "arena.libc.h"

static const struct arena_module_ty arena_libc_jmp =
    ARENA_MODULE_INIT(arena_libc, contract_unit_test);

static const arena_module mod = &arena_libc_jmp;

uint64_t codegen_sanity_check_allocate(arena_t *a, uint64_t bytes) {
  static const arena_module m = &arena_libc;

  return arena_allocate_into_existing_capacity(m, a, bytes);
}

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

  TEST("fail a precondition") {
    arena_t a = {0};
    CHECK(!arena_valid(mod, a));
    DEATH(arena_destroy(mod, a));
  }
}

static MODULE(arena_use_without_resizing) {
  arena_t arena = arena_create(mod, 4);
  CHECK(arena_valid(mod, arena));

  TEST("initial size") {
    CHECK(arena_size(mod, arena) == 0);
    CHECK(arena_capacity(mod, arena) == 4);
    CHECK(arena_available(mod, arena) == 4);
  }

  TEST("allocate zero does nothing") {
    uint64_t offset = arena_allocate_into_existing_capacity(mod, &arena, 0);
    CHECK(offset == 0);
    CHECK(arena_size(mod, arena) == 0);
    CHECK(arena_capacity(mod, arena) == 4);
    CHECK(arena_available(mod, arena) == 4);
  }

  TEST("allocate one byte") {
    uint64_t offset = arena_allocate_into_existing_capacity(mod, &arena, 1);
    CHECK(offset == 0);
    CHECK(arena_size(mod, arena) == 1);
    CHECK(arena_capacity(mod, arena) == 4);
    CHECK(arena_available(mod, arena) == 3);
  }

  TEST("allocate two bytes") {
    uint64_t offset = arena_allocate_into_existing_capacity(mod, &arena, 2);
    CHECK(offset == 0);
    CHECK(arena_size(mod, arena) == 2);
    CHECK(arena_capacity(mod, arena) == 4);
    CHECK(arena_available(mod, arena) == 2);
  }

  TEST("allocate whole capacity") {
    uint64_t offset = arena_allocate_into_existing_capacity(mod, &arena, 4);
    CHECK(offset == 0);
    CHECK(arena_size(mod, arena) == 4);
    CHECK(arena_capacity(mod, arena) == 4);
    CHECK(arena_available(mod, arena) == 0);
  }

  TEST("allocate beyond capacity is contract violation") {
    DEATH(arena_allocate_into_existing_capacity(mod, &arena, 5));
    CHECK(arena_size(mod, arena) == 0);
    CHECK(arena_capacity(mod, arena) == 4);
    CHECK(arena_available(mod, arena) == 4);
  }

  TEST("allocate multiple single bytes") {
    for (unsigned i = 0; i < 4; i++) {
      uint64_t offset = arena_allocate_into_existing_capacity(mod, &arena, 1);
      CHECK(offset == i);
      CHECK(arena_size(mod, arena) == i + 1);
      CHECK(arena_capacity(mod, arena) == 4);
      CHECK(arena_available(mod, arena) == 3 - i);
    }

    DEATH(arena_allocate_into_existing_capacity(mod, &arena, 1));
  }

  TEST("allocate two bytes twice") {
    for (unsigned i = 0; i < 4; i += 2) {
      uint64_t offset = arena_allocate_into_existing_capacity(mod, &arena, 2);
      CHECK(offset == i);
      CHECK(arena_size(mod, arena) == i + 2);
      CHECK(arena_capacity(mod, arena) == 4);
      CHECK(arena_available(mod, arena) == 2 - i);
    }
    DEATH(arena_allocate_into_existing_capacity(mod, &arena, 1));
  }

  arena_destroy(mod, arena);
}

static MODULE(change_capacity_of_empty_arena)
{
  arena_t arena = arena_create(mod, 4);
  CHECK(arena_valid(mod, arena));

  TEST("increase")
    {
      bool r = arena_change_capacity(mod, &arena, 7);
      CHECK(arena_size(mod, arena) == 0);
      if (r)
        {
          CHECK(arena_capacity(mod, arena) == 7);
          CHECK(arena_available(mod, arena) == 7);
        }
      else
        {
          CHECK(arena_capacity(mod, arena) == 4);
          CHECK(arena_available(mod, arena) == 4);
        }
    }

  TEST("decrease")
    {
      bool r = arena_change_capacity(mod, &arena, 3);
      CHECK(arena_size(mod, arena) == 0);
      if (r)
        {
          CHECK(arena_capacity(mod, arena) == 3);
          CHECK(arena_available(mod, arena) == 3);
        }
      else
        {
          CHECK(arena_capacity(mod, arena) == 4);
          CHECK(arena_available(mod, arena) == 4);
        }
    }
  
  arena_destroy(mod, arena);
}

static MODULE(change_capacity_of_nonempty_arena)
{
  arena_t arena = arena_create(mod, 8);
  CHECK(arena_valid(mod, arena));

  uint64_t offset = arena_allocate_into_existing_capacity(mod, &arena, 3);
  CHECK(offset == 0);
  CHECK(arena_size(mod, arena) == 3);
  CHECK(arena_capacity(mod, arena) == 8);
  CHECK(arena_available(mod, arena) == 5);
  
  TEST("increase")
    {
      bool r = arena_change_capacity(mod, &arena, 13);
      CHECK(arena_size(mod, arena) == 3);
      if (r)
        {
          CHECK(arena_capacity(mod, arena) == 13);
          CHECK(arena_available(mod, arena) == 10);
        }
      else
        {
          CHECK(arena_capacity(mod, arena) == 8);
          CHECK(arena_available(mod, arena) == 8);
        }
    }

  TEST("decrease")
    {
      bool r = arena_change_capacity(mod, &arena, 5);
      CHECK(arena_size(mod, arena) == 3);
      if (r)
        {
          CHECK(arena_capacity(mod, arena) == 5);
          CHECK(arena_available(mod, arena) == 2);
        }
      else
        {
          CHECK(arena_capacity(mod, arena) == 4);
          CHECK(arena_available(mod, arena) == 4);
        }
    }
  
  arena_destroy(mod, arena);
}

static MODULE(allocate_align_one)
{
  arena_t arena = arena_create(mod, 8);
  CHECK(arena_valid(mod, arena));

  TEST("align zero dies")
    {
      DEATH(arena_allocate(mod, &arena, 0, 0));
      DEATH(arena_allocate(mod, &arena, 8, 0));
      DEATH(arena_allocate(mod, &arena, 16, 0));
    }
  
  TEST("allocate zero")
    {
      CHECK(arena_size(mod, arena) == 0);
      CHECK(arena_capacity(mod, arena) == 8);
      CHECK(arena_available(mod, arena) == 8);
      
      uint64_t offset = arena_allocate(mod, &arena, 0, 1);
      CHECK(offset == 0);

      CHECK(arena_size(mod, arena) == 0);
      CHECK(arena_capacity(mod, arena) == 8);
      CHECK(arena_available(mod, arena) == 8);
    }

  TEST("allocate within size")
    {
      CHECK(arena_size(mod, arena) == 0);
      CHECK(arena_capacity(mod, arena) == 8);
      CHECK(arena_available(mod, arena) == 8);
      
      uint64_t offset = arena_allocate(mod, &arena, 4, 1);
      CHECK(offset == 0);

      CHECK(arena_size(mod, arena) == 4);
      CHECK(arena_capacity(mod, arena) == 8);
      CHECK(arena_available(mod, arena) == 4);
    }

    TEST("allocate exactly size")
    {
      CHECK(arena_size(mod, arena) == 0);
      CHECK(arena_capacity(mod, arena) == 8);
      CHECK(arena_available(mod, arena) == 8);
      
      uint64_t offset = arena_allocate(mod, &arena, 8, 1);
      CHECK(offset == 0);

      CHECK(arena_size(mod, arena) == 8);
      CHECK(arena_capacity(mod, arena) == 8);
      CHECK(arena_available(mod, arena) == 0);
    }
  
    TEST("allocate exceeds size")
    {
      CHECK(arena_size(mod, arena) == 0);
      CHECK(arena_capacity(mod, arena) == 8);
      CHECK(arena_available(mod, arena) == 8);
      
      uint64_t offset = arena_allocate(mod, &arena, 16, 1);
      CHECK(offset == 0);

      CHECK(arena_size(mod, arena) == 16);
      CHECK(arena_capacity(mod, arena) == 16);
      CHECK(arena_available(mod, arena) == 0);
    }

    arena_destroy(mod, arena);
}

MAIN_MODULE() {
  DEPENDS(create_destroy);
  DEPENDS(arena_use_without_resizing);
  DEPENDS(change_capacity_of_empty_arena);
  DEPENDS(change_capacity_of_nonempty_arena);
  DEPENDS(allocate_align_one);
}
