#ifndef ARENA_LIBC_H_INCLUDED
#define ARENA_LIBC_H_INCLUDED

#include "arena.h"
#include "contract.h"
#include "arena.from_libc.h"

#include <stdlib.h>

static void* arena_libc_malloc(size_t N)
{
  return malloc(N);
}

static void* arena_libc_realloc(void* p, size_t N)
{
  return realloc(p, N);
}

static void arena_libc_free(void* p)
{
  free(p);
}

ARENA_FROM_LIBC(arena_libc, arena_libc_malloc, arena_libc_realloc, arena_libc_free)

static const struct arena_module_ty arena_libc = ARENA_MODULE_INIT(arena_libc, 0);

#endif
