#ifndef TOOLS_INTSET_UTIL_H_INCLUDED
#define TOOLS_INTSET_UTIL_H_INCLUDED

#include "hashtable.h"
#include "intset.h"
#include "arena.h"

enum { intset_util_struct_sizeof = 4 * 8 };

static hashtable_t intset_util_to_hash(intset_t s) {
  hashtable_t r;
  __builtin_memcpy(&r.state, &s.state, intset_util_struct_sizeof);
  return r;
}

static intset_t intset_util_to_set(hashtable_t s) {
  intset_t r;
  __builtin_memcpy(&r.state, &s.state, intset_util_struct_sizeof);
  return r;
}

static arena_t intset_util_hash_to_arena(hashtable_t h) {
  arena_t a;
  a.base = h.state[0];
  a.next = h.state[1];
  a.limit = h.state[2];
  return a;
}

static hashtable_t intset_util_arena_to_hash(arena_t a) {
  hashtable_t h;
  h.state[0] = a.base;
  h.state[1] = a.next;
  h.state[2] = a.limit;
  h.state[3] = 0;
  return h;
}



#endif
