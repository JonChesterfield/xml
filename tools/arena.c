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

static MODULE(increment_for_alignment) {
  TEST("zero realignment needed") {
    CHECK(0 == arena_increment_needed_for_alignment(mod, 0, 1));
    CHECK(0 == arena_increment_needed_for_alignment(mod, 1, 1));
    CHECK(0 == arena_increment_needed_for_alignment(mod, 2, 1));
    CHECK(0 == arena_increment_needed_for_alignment(mod, 3, 1));

    CHECK(0 == arena_increment_needed_for_alignment(mod, 0, 2));
    CHECK(0 == arena_increment_needed_for_alignment(mod, 2, 2));
    CHECK(0 == arena_increment_needed_for_alignment(mod, 4, 2));

    CHECK(0 == arena_increment_needed_for_alignment(mod, 0, 4));
    CHECK(0 == arena_increment_needed_for_alignment(mod, 4, 4));
    CHECK(0 == arena_increment_needed_for_alignment(mod, 8, 4));
  }

  TEST("fail on non-power two align") {
    DEATH(arena_increment_needed_for_alignment(mod, 0, 0));
    DEATH(arena_increment_needed_for_alignment(mod, 4, 0));
    DEATH(arena_increment_needed_for_alignment(mod, 0, 3));
    DEATH(arena_increment_needed_for_alignment(mod, 4, 3));
    DEATH(arena_increment_needed_for_alignment(mod, 0, 5));
    DEATH(arena_increment_needed_for_alignment(mod, 4, 5));
    DEATH(arena_increment_needed_for_alignment(mod, 0, 6));
    DEATH(arena_increment_needed_for_alignment(mod, 4, 6));
    DEATH(arena_increment_needed_for_alignment(mod, 0, 7));
    DEATH(arena_increment_needed_for_alignment(mod, 4, 7));
    DEATH(arena_increment_needed_for_alignment(mod, 0, 9));
    DEATH(arena_increment_needed_for_alignment(mod, 4, 9));
  }

  TEST("parametric") {
    for (uint64_t base = 0; base < 100; base++) {
      for (unsigned a = 0; a < 6; a++) {
        uint64_t align = UINT64_C(1) << a;
        uint64_t incr = arena_increment_needed_for_alignment(mod, base, align);

        CHECK(incr < align);
        CHECK(((base + incr) % align) == 0);
      }
    }
  }
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

static MODULE(change_capacity_of_empty_arena) {
  arena_t arena = arena_create(mod, 4);
  CHECK(arena_valid(mod, arena));

  TEST("increase") {
    bool r = arena_change_capacity(mod, &arena, 7);
    CHECK(arena_size(mod, arena) == 0);
    if (r) {
      CHECK(arena_capacity(mod, arena) == 7);
      CHECK(arena_available(mod, arena) == 7);
    } else {
      CHECK(arena_capacity(mod, arena) == 4);
      CHECK(arena_available(mod, arena) == 4);
    }
  }

  TEST("decrease") {
    bool r = arena_change_capacity(mod, &arena, 3);
    CHECK(arena_size(mod, arena) == 0);
    if (r) {
      CHECK(arena_capacity(mod, arena) == 3);
      CHECK(arena_available(mod, arena) == 3);
    } else {
      CHECK(arena_capacity(mod, arena) == 4);
      CHECK(arena_available(mod, arena) == 4);
    }
  }

  arena_destroy(mod, arena);
}

static MODULE(change_capacity_of_nonempty_arena) {
  arena_t arena = arena_create(mod, 8);
  CHECK(arena_valid(mod, arena));

  uint64_t offset = arena_allocate_into_existing_capacity(mod, &arena, 3);
  CHECK(offset == 0);
  CHECK(arena_size(mod, arena) == 3);
  CHECK(arena_capacity(mod, arena) == 8);
  CHECK(arena_available(mod, arena) == 5);

  TEST("increase") {
    bool r = arena_change_capacity(mod, &arena, 13);
    CHECK(arena_size(mod, arena) == 3);
    if (r) {
      CHECK(arena_capacity(mod, arena) == 13);
      CHECK(arena_available(mod, arena) == 10);
    } else {
      CHECK(arena_capacity(mod, arena) == 8);
      CHECK(arena_available(mod, arena) == 8);
    }
  }

  TEST("decrease") {
    bool r = arena_change_capacity(mod, &arena, 5);
    CHECK(arena_size(mod, arena) == 3);
    if (r) {
      CHECK(arena_capacity(mod, arena) == 5);
      CHECK(arena_available(mod, arena) == 2);
    } else {
      CHECK(arena_capacity(mod, arena) == 4);
      CHECK(arena_available(mod, arena) == 4);
    }
  }

  arena_destroy(mod, arena);
}

static MODULE(allocate_align_one) {
  arena_t arena = arena_create(mod, 8);
  CHECK(arena_valid(mod, arena));

  TEST("align zero dies") {
    DEATH(arena_allocate(mod, &arena, 0, 0));
    DEATH(arena_allocate(mod, &arena, 8, 0));
    DEATH(arena_allocate(mod, &arena, 16, 0));
  }

  TEST("allocate zero") {
    CHECK(arena_size(mod, arena) == 0);
    CHECK(arena_capacity(mod, arena) == 8);
    CHECK(arena_available(mod, arena) == 8);

    uint64_t offset = arena_allocate(mod, &arena, 0, 1);
    CHECK(offset == 0);

    CHECK(arena_size(mod, arena) == 0);
    CHECK(arena_capacity(mod, arena) == 8);
    CHECK(arena_available(mod, arena) == 8);
  }

  TEST("allocate within size") {
    CHECK(arena_size(mod, arena) == 0);
    CHECK(arena_capacity(mod, arena) == 8);
    CHECK(arena_available(mod, arena) == 8);

    uint64_t offset = arena_allocate(mod, &arena, 4, 1);
    CHECK(offset == 0);

    CHECK(arena_size(mod, arena) == 4);
    CHECK(arena_capacity(mod, arena) == 8);
    CHECK(arena_available(mod, arena) == 4);
  }

  TEST("allocate exactly size") {
    CHECK(arena_size(mod, arena) == 0);
    CHECK(arena_capacity(mod, arena) == 8);
    CHECK(arena_available(mod, arena) == 8);

    uint64_t offset = arena_allocate(mod, &arena, 8, 1);
    CHECK(offset == 0);

    CHECK(arena_size(mod, arena) == 8);
    CHECK(arena_capacity(mod, arena) == 8);
    CHECK(arena_available(mod, arena) == 0);
  }

  TEST("allocate exceeds size") {
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

static MODULE(allocate_multiple) {
  TEST("alloc ascending") {
    arena_t arena = arena_create(mod, 0);
    for (unsigned A = 0; A < 5; A++) {
      uint64_t align = 1 << A;
      for (uint64_t v = 0; v < 32; v++) {
        uint64_t r = arena_allocate(mod, &arena, v, align);
        CHECK(r != ~UINT64_C(0));
      }
    }
    arena_destroy(mod, arena);
  }

  TEST("alloc descending") {
    arena_t arena = arena_create(mod, 0);
    for (unsigned A = 0; A < 5; A++) {
      uint64_t align = 1 << A;
      for (uint64_t v = 32; v-- > 0;) {
        uint64_t r = arena_allocate(mod, &arena, v, align);
        CHECK(r != ~UINT64_C(0));
      }
    }
    arena_destroy(mod, arena);
  }

  TEST("align N") {
    for (unsigned A = 0; A < 4; A++) {
      uint64_t align = 1 << A;

      uint64_t vals[] = {1, 3, 2, 0, 5, 7, 13, 17, 4, 1, 7};
      size_t N = sizeof(vals) / sizeof(vals[0]);

      arena_t arena = arena_create(mod, 0);

      for (size_t i = 0; i < N; i++) {
        arena_allocate(mod, &arena, vals[i], align);
      }

      arena_destroy(mod, arena);
    }
  }
}

MODULE(convenience) {
  TEST("req available") {
    arena_t arena = arena_create(mod, 0);
    CHECK(arena_valid(mod, arena));

    for (unsigned i = 0; i < 20; i += 3) {
      CHECK(arena_request_available(mod, &arena, i));
      CHECK(arena_available(mod, arena) >= i);
    }

    arena_destroy(mod, arena);
  }

  TEST("pad to alignment") {
    arena_t arena = arena_create(mod, 0);
    CHECK(arena_valid(mod, arena));

    CHECK(arena_pad_to_alignment(mod, &arena, 1));
    CHECK(0 == arena_size(mod, arena));

    CHECK(arena_pad_to_alignment(mod, &arena, 8));
    CHECK(0 == arena_size(mod, arena));

    CHECK(0 == arena_allocate(mod, &arena, 3, 1));
    CHECK(3 == arena_size(mod, arena));

    CHECK(arena_pad_to_alignment(mod, &arena, 2));
    CHECK(4 == arena_size(mod, arena));

    CHECK(arena_pad_to_alignment(mod, &arena, 4));
    CHECK(4 == arena_size(mod, arena));

    CHECK(arena_pad_to_alignment(mod, &arena, 8));
    CHECK(8 == arena_size(mod, arena));

    arena_destroy(mod, arena);
  }

  TEST("append bytes") {
    arena_t arena = arena_create(mod, 0);
    CHECK(arena_valid(mod, arena));

    unsigned char buf[7] = "foobar?";

    CHECK(arena_append_bytes(mod, &arena, buf, sizeof(buf)));
    CHECK(7 == arena_size(mod, arena));

    CHECK(__builtin_memcmp(buf, arena_base_address(mod, arena), 7) == 0);

    for (unsigned i = 0; i < 20; i += 3) {
      CHECK(arena_request_available(mod, &arena, i));
      CHECK(arena_available(mod, arena) >= i);
    }

    arena_destroy(mod, arena);
  }
}

MAIN_MODULE() {
  DEPENDS(increment_for_alignment);
  DEPENDS(create_destroy);
  DEPENDS(arena_use_without_resizing);
  DEPENDS(change_capacity_of_empty_arena);
  DEPENDS(change_capacity_of_nonempty_arena);
  DEPENDS(allocate_align_one);
  DEPENDS(allocate_multiple);
  DEPENDS(convenience);
}
