#ifndef ARENA_LIBC_H_INCLUDED
#define ARENA_LIBC_H_INCLUDED

#include "arena.h"
#include "contract.h"

#include <stdlib.h>


static  arena_t arena_libc_create(uint64_t N)
{
  arena_t r = {0};
  void * d = malloc(N);
  if (d)
    {
      r.base = (uint64_t)d;
      r.next = (uint64_t)d;
      r.limit = (uint64_t)d + N;
    }
  return r;
}

static  void arena_libc_destroy(arena_t a)
{
  void* d = (void*)a.base;
  free(d);
}
static  bool arena_libc_valid(arena_t a)
{
  return a.base != 0;
}

static  bool arena_libc_base_address_constant(void) {return false; }
static  bool arena_libc_limit_address_constant(void) { return false; }

static  uint64_t arena_libc_size(arena_t a)
{
  return a.next - a.base;
}

static  uint64_t arena_libc_capacity(arena_t a)
{
  return a.limit - a.base;
}

static  void * arena_libc_base_address(arena_t a)
{
  return (void*)a.base;
}
static  void * arena_libc_next_address(arena_t a)
{
  return (void*)a.next;
}

static  void * arena_libc_limit_address(arena_t a)
{
  return (void*)a.limit;
}
  
static  bool arena_libc_change_capacity(arena_t *a, uint64_t bytes)
{
  char* base = (char*)a->base;
  char* next = (char*)a->next;
  char* limit = (char*)a->limit;

  uint64_t next_offset = next - base;
  uint64_t capacity = limit - base;
  if (bytes == capacity) { return true; }

  char * r = realloc(base, bytes);
  if (!r) {
    return false;
  }

  a->base = (uint64_t)r;
  a->next = (uint64_t)r + next_offset;
  a->limit = (uint64_t)r + bytes;
  
  
  return true;
}

static  uint64_t arena_libc_allocate_into_existing_capacity(arena_t *a, uint64_t bytes)
{
  char* next = (char*)a->next;
  char* incr = next + bytes;
  a->next = (uint64_t)incr;
  return (uint64_t)next;
}


static const struct arena_module_ty arena_libc = ARENA_MODULE_INIT(arena_libc, 0, 0);

#endif
