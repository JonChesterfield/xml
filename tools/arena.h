#ifndef TOOLS_ARENA_H
#define TOOLS_ARENA_H

// Interface sketch only so far.

#define ARENA_CONTRACT() 0

struct arena_module_ty;
typedef const struct arena_module_ty *arena_module;

// Returns null on failure, arena with capacity >= N bytes on success
static inline void *arena_create(arena_module mod, size_t N);
static inline void arena_destroy(arena_module mod, void *s);

// Primary function, may increase size of backing storage
static inline void *arena_allocate(arena_module mod, void *s, uint64_t bytes);

// Bytes already allocated
static inline size_t arena_size(arena_module mod, void *s);

// Bytes available in total
static inline size_t arena_capacity(arena_module mod, void *s);

// Increase capacity
static inline void *arena_reserve(arena_module mod, void *s, size_t bytes);

// Allocate without the branch to reserve
static inline void *arena_allocate_assuming_capacity(arena_module mod, void *s, uint64_t bytes);

struct arena_module_ty {
  void *(*const create)(void);
  void (*const destroy)(void *);

  size_t (*const size)(void *);
  size_t (*const capacity)(void *);

  void *(*const reserve)(void *, size_t);

  void *(*const allocate)(void *, uint64_t);
  void *(*const allocate_assuming_capacity)(void *, uint64_t);

};

#endif
