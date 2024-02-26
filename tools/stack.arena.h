#ifndef STACK_ARENA_H_INCLUDED
#define STACK_ARENA_H_INCLUDED

#include "arena.h"
#include "stack.h"

static void * stack_create_from_arena(arena_module mod)
{
  arena_t a = arena_create(mod, 24);
  if (arena_valid(mod, a))
    {      
      char * base = (char*)arena_base_address(mod, a);
      arena_allocate_into_existing_capacity(mod, &a, 24);

      __builtin_memcpy(base, &a, 24);

      return base;      
    }
  
  return 0;
}

static arena_t stack_retrieve_underlying_arena(arena_module mod, void * stack)
{
  void * base = stack;
  arena_t a;
  __builtin_memcpy(&a, base, 24);
  arena_require(arena_valid(mod, a));
  return a;
}

#if 0

#include "arena.libc.h"

static MODULE(stack_arena)
{
  TEST("make")
    {
      const struct arena_module_ty * arena_mod = &arena_libc;
      void* s = stack_create_from_arena(arena_mod);
      CHECK(s != 0);

      arena_t a = stack_retrieve_underlying_arena(arena_mod, s);
      CHECK(arena_valid(arena_mod, a));
      CHECK(arena_size(arena_mod, a) == 24);
    }
}
#endif

#endif
