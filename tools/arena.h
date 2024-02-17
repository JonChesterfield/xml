#ifndef TOOLS_ARENA_H
#define TOOLS_ARENA_H

// Interface sketch only so far.
// Thinking a pointer to contiguous memory, where allocate is bump pointer
// Exceeding the initial allocated size could be an error or allocate more
// If allocating more, some implementations will want to change the base address
// and some others won't need to.
// Idea is reallocate as allocate + memcpy in some cases.
//
// Considering various backing memory types:
// 1. mmap
// 2. brk
// 3. .data objects
// 4. malloc/realloc
// All of these can do a (limited) sort of deallocation
// Most can be resized, some can't
// The base pointer might be invalidated on resize but might not be
//
// Most interfaces are more convenient in terms of a pointer but the
// resize-invalidates-pointer property argues for offsets.
//
// Alignment could be treated as a separate allocation of a few bytes,
// put it in the public API but not in the implementation API

#define ARENA_CONTRACT() 0

struct arena_module_ty;
typedef const struct arena_module_ty *arena_module;

// Returns null on failure, arena with capacity >= N bytes on success
static inline void *arena_create(arena_module mod, size_t N);
static inline void arena_destroy(arena_module mod, void *s);

// Bytes already allocated
static inline size_t arena_size(arena_module mod, void *s);

// Bytes available in total
static inline size_t arena_capacity(arena_module mod, void *s);

// True if this arena can be resized after allocation
static inline bool arena_resizable(arena_module mod);

// Get pointer to the current base of the memory of the arena
static inline void * arena_base_address(arena_module mod, void*);

// Get pointer to the next object that will be allocated
static inline void * arena_next_address(arena_module mod, void*);

// Allocate without reallocating, asserts on insufficient capacity, else UB. Doesn't change base_address.
static inline uint64_t arena_allocate_assuming_capacity(arena_module mod, void *s, uint64_t bytes);


// Primary function, may increase size of backing storage in which case base_address is invalidated
static inline uint64_t arena_allocate(arena_module mod, void *s, uint64_t bytes);


// Increase capacity
static inline void *arena_reserve(arena_module mod, void *s, size_t bytes);


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
