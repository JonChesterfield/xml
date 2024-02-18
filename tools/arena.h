#ifndef TOOLS_ARENA_H_INCLUDED
#define TOOLS_ARENA_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include <unistd.h>
#include <stdlib.h>

#include "contract.h"

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

#define ARENA_CONTRACT() 1

#if ARENA_CONTRACT()
#include <setjmp.h>
#endif

struct arena_module_ty;
typedef const struct arena_module_ty *arena_module;


#define arena_require(X) do {if (arena_contract_active(mod)) { mod->maybe_contract(X, mod->maybe_jmp_buf, contract_message(X), sizeof(contract_message(X))-1); }} while (0)


//
// Construct and destroy an arena
//

// Arena is essentially three pointers
// where allocate/decallocate moves next up and down
// and resizing moves limit. The values in this struct
// may not be meaningful for a given module.
// [   ] <- base
// [   ]
// [   ]
// [   ] <- next
// [   ]
// [   ]
// [   ]
// [   ] <- limit
typedef struct
{
  uint64_t base;
  uint64_t next;
  uint64_t limit;
} arena_t;

// Returns arena that fails the valid? test on failure, arena with capacity >= N bytes on success
static inline arena_t arena_create(arena_module mod, uint64_t N);
static inline void arena_destroy(arena_module mod, arena_t a);
static inline bool arena_valid(arena_module mod, arena_t a);

//
// Module level queries
//

// Alignment values must be all be a power of two
static bool arena_power_of_two_p(uint64_t x) { return x && !(x & (x - 1)); }

// True if all calls to base_address return the same value
static inline bool arena_base_address_constant(arena_module mod);

// True if all calls to limit_address return the same value
static inline bool arena_limit_address_constant(arena_module mod);

// True if the module has a non-null contract handler
static inline bool arena_contract_active(arena_module mod);

//
// Arena level queries
//

// Bytes already allocated
static inline uint64_t arena_size(arena_module mod, arena_t a);

// Bytes available in total
static inline uint64_t arena_capacity(arena_module mod, arena_t a);

// Capacity - size
static inline uint64_t arena_available(arena_module mod, arena_t a);

// Allocate this many bytes to move base to the specified alignment
static inline uint64_t arena_bytes_needed_for_alignment(arena_module mod, uint64_t base, uint64_t align);

// Get pointer to the current base of the memory of the arena
static inline void * arena_base_address(arena_module mod, arena_t a);

// Get pointer to the next object that will be allocated
static inline void * arena_next_address(arena_module mod, arena_t a);

// Get pointer to the end of the arena
static inline void * arena_limit_address(arena_module mod, arena_t a);

//
// Mutators
// On reporting failure, arena argument is unchanged
//

// Increase or decrease capacity. On success returns true and mutates a.
static inline bool arena_change_capacity(arena_module mod, arena_t * a, uint64_t bytes);

// Allocate bytes space, returns offset from base address. Assert / UB if insufficient capacity.
static inline uint64_t arena_allocate_into_existing_capacity(arena_module mod, arena_t * a, uint64_t bytes);

// Returns ~0 on failure (the first allocation might be offset zero), attempting to increase capacity if out of space

static inline uint64_t arena_allocate(arena_module mod, arena_t *a, uint64_t bytes, uint64_t align);


struct arena_module_ty {
  arena_t (*const create)(uint64_t N);
  void (*const destroy)(arena_t);
  bool (*const valid)(arena_t);

  bool (*const base_address_constant)(void);
  bool (*const limit_address_constant)(void);

  uint64_t (*const size)(arena_t);
  uint64_t (*const capacity)(arena_t);

  void * (*const base_address)(arena_t);
  void * (*const next_address)(arena_t);
  void * (*const limit_address)(arena_t);
  
  bool (*const change_capacity)(arena_t *a, uint64_t bytes);

  uint64_t (*const allocate_into_existing_capacity)(arena_t *a, uint64_t bytes);

  // The maybe prefixed ones can be 0
  void (*const maybe_contract)(bool, void*, const char * message, size_t message_length);
  jmp_buf * maybe_jmp_buf;
};

#define ARENA_CONCAT2(X, Y) X##_##Y
#define ARENA_CONCAT(X, Y) ARENA_CONCAT2(X, Y)

#define ARENA_MODULE_INIT(PREFIX, CONTRACT, JMPBUF) {    \
    .create = ARENA_CONCAT(PREFIX, create), \
    .destroy = ARENA_CONCAT(PREFIX, destroy), \
    .valid = ARENA_CONCAT(PREFIX, valid), \
    .base_address_constant = ARENA_CONCAT(PREFIX, base_address_constant), \
    .limit_address_constant = ARENA_CONCAT(PREFIX, limit_address_constant), \
    .size = ARENA_CONCAT(PREFIX, size), \
    .capacity = ARENA_CONCAT(PREFIX, capacity), \
    .base_address = ARENA_CONCAT(PREFIX, base_address), \
    .next_address = ARENA_CONCAT(PREFIX, next_address), \
    .limit_address = ARENA_CONCAT(PREFIX, limit_address), \
    .change_capacity = ARENA_CONCAT(PREFIX, change_capacity), \
    .allocate_into_existing_capacity = ARENA_CONCAT(PREFIX, allocate_into_existing_capacity), \
      .maybe_contract = CONTRACT,                                       \
      .maybe_jmp_buf = JMPBUF,                                          \
    }


static inline void arena_require_func(arena_module mod, bool expr, const char *message, size_t message_length) {
  (void)mod;

  const bool contract = ARENA_CONTRACT();
  if (contract & !expr) {
#if ARENA_CONTRACT()

    write(STDERR_FILENO, message, message_length);
    if (mod->maybe_jmp_buf) {
      jmp_buf * buf = mod->maybe_jmp_buf;
      longjmp(*buf, 1);
    }
    //    fprintf(stderr, "arena contract failed L%u: %s\n", line, name);
    abort();
#else
    (void)name;
    (void)line;
#endif
  }
}


static inline arena_t arena_create(arena_module mod, uint64_t N)
{
  return mod->create(N);
}

static inline void arena_destroy(arena_module mod, arena_t a)
{
  arena_require(arena_valid(mod, a));
  mod->destroy(a);
}

static inline bool arena_valid(arena_module mod, arena_t a)
{
  if (!mod->valid(a)) { return false; }

  uint64_t base = (uint64_t) mod->base_address( a);
  uint64_t next = (uint64_t) mod->next_address( a);
  uint64_t limit = (uint64_t) mod->limit_address( a);
  return (base <= next) && (next <= limit);
}

static inline bool arena_base_address_constant(arena_module mod)
{
  return mod->base_address_constant();
}

static inline bool arena_limit_address_constant(arena_module mod)
{
  return mod->limit_address_constant();
}

static inline bool arena_contract_active(arena_module mod)
{
  return mod->maybe_contract != 0;
}

static inline uint64_t arena_size(arena_module mod, arena_t a)
{
  arena_require(arena_valid(mod, a));
  return mod->size(a);
}

static inline uint64_t arena_capacity(arena_module mod, arena_t a){
  arena_require(arena_valid(mod, a));
  return mod->capacity(a);
}

static inline uint64_t arena_available(arena_module mod, arena_t a){
  arena_require(arena_valid(mod, a));
  return arena_capacity(mod, a) - arena_size(mod, a);
}

static inline uint64_t arena_increment_needed_for_alignment(arena_module mod, uint64_t base, uint64_t align)
{
  arena_require(arena_power_of_two_p(align));

  uint64_t modulo_as_mask = (align - 1);
  uint64_t rem = base & modulo_as_mask;

  arena_require(rem == (base % align));

  return rem ? (align - rem) : 0;
}

static inline void * arena_base_address(arena_module mod, arena_t a){
  arena_require(arena_valid(mod, a));
  return mod->base_address(a);
}

static inline void * arena_next_address(arena_module mod, arena_t a){
  arena_require(arena_valid(mod, a));
  return mod->next_address(a);
}

static inline void * arena_limit_address(arena_module mod, arena_t a){
  arena_require(arena_valid(mod, a));
  return mod->limit_address(a);
}

static inline bool arena_change_capacity(arena_module mod, arena_t * a, uint64_t bytes)
{
  arena_t before = {0};
#if ARENA_CONTRACT()
  before = *a;
#endif
  
  arena_require(arena_valid(mod, *a));

  bool r = mod->change_capacity(a, bytes);
  
  if (!r)
    {
      arena_require(before.base == a->base);
      arena_require(before.next == a->next);
      arena_require(before.limit == a->limit);
    }
  else
    {
      arena_require(arena_valid(mod, *a));
      arena_require(arena_capacity(mod, *a) == bytes);
    }

  return r;
}

// Allocate bytes space, returns offset from base address. Assert / UB if insufficient capacity.
static inline uint64_t arena_allocate_into_existing_capacity(arena_module mod, arena_t * a, uint64_t bytes)
{
  arena_require(arena_valid(mod, *a));
  uint64_t available_before = arena_available(mod, *a);
  uint64_t size_before = arena_size(mod, *a);
  
  arena_require(bytes <= available_before);
  
  uint64_t r = mod->allocate_into_existing_capacity(a,bytes);
  
  arena_require((available_before - bytes) == arena_available(mod, *a));
  arena_require((size_before + bytes) == arena_size(mod, *a));

  return r;
}

// Returns ~0 on failure (the first allocation might be offset zero), attempting to increase capacity if out of space
static inline uint64_t arena_allocate(arena_module mod, arena_t *a, uint64_t bytes, uint64_t align)
{
  arena_require(arena_valid(mod, *a));
  arena_require(arena_power_of_two_p(align));
  uint64_t next = (uint64_t) arena_next_address(mod, *a);
  
  uint64_t available = arena_available(mod, *a); 

  uint64_t align_padding = arena_increment_needed_for_alignment(mod, next, align);
  
  uint64_t total_alloc = bytes + align_padding;

  if (available < total_alloc)
    {
      uint64_t shortfall = total_alloc - available;
      
      uint64_t cap = arena_capacity(mod, *a);
      bool r = arena_change_capacity(mod, a, cap + shortfall);
      if (!r) {
        return ~ UINT64_C(0);
      }

      arena_require(arena_capacity(mod, *a) >= (cap + shortfall));
    }
  arena_require(arena_available(mod, *a) >= total_alloc );

  
  uint64_t got = arena_allocate_into_existing_capacity(mod, a, total_alloc);
  arena_require(got == next);
  uint64_t res = next + align_padding;

  arena_require(arena_increment_needed_for_alignment(mod, res, align) == 0);
  
  return res;

}



#endif