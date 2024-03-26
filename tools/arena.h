#ifndef TOOLS_ARENA_H_INCLUDED
#define TOOLS_ARENA_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

struct arena_module_ty;
typedef const struct arena_module_ty *arena_module;

#define arena_require(X)                                                       \
  do {                                                                         \
    if (arena_contract_active(mod)) {                                          \
      mod->maybe_contract(X, contract_message(X),                              \
                          sizeof(contract_message(X)) - 1);                    \
    }                                                                          \
  } while (0)

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
typedef struct {
  uint64_t base;
  uint64_t next;
  uint64_t limit;
} arena_t;

// Returns arena that fails the valid? test on failure, arena with capacity >= N
// bytes on success
static inline arena_t arena_create(arena_module mod, uint64_t N);
static inline void arena_destroy(arena_module mod, arena_t a);
static inline bool arena_valid(arena_module mod, arena_t a);

//
// Module level queries
//

// True if all calls to base_address return the same value
static inline bool arena_base_address_constant(arena_module mod);

// True if all calls to limit_address return the same value
static inline bool arena_limit_address_constant(arena_module mod);

// The alignment of base_address and the maximum alignment of allocations
static inline uint64_t arena_alignment(arena_module mod);

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
static inline uint64_t arena_increment_needed_for_alignment(arena_module mod,
                                                            uint64_t base,
                                                            uint64_t align);

// Get pointer to the current base of the memory of the arena
static inline void *arena_base_address(arena_module mod, arena_t a);

// Get pointer to the next object that will be allocated
static inline void *arena_next_address(arena_module mod, arena_t a);

// Get pointer to the end of the arena
static inline void *arena_limit_address(arena_module mod, arena_t a);

// Get offset from base of the next object that will be allocated
static inline uint64_t arena_next_offset(arena_module mod, arena_t a)
{
  uint64_t next_offset =
      (char *)arena_next_address(mod, a) - (char *)arena_base_address(mod, a);
  return next_offset;
}

//
// Mutators
// On reporting failure, arena argument is unchanged
//

// Increase or decrease capacity. On success returns true and mutates a.
static inline bool arena_change_capacity(arena_module mod, arena_t *a,
                                         uint64_t bytes);

static inline void arena_change_allocation(arena_module mod, arena_t *a,
                                           uint64_t bytes);

static inline void arena_discard_last_allocated(arena_module mod, arena_t *a,
                                                uint64_t bytes);

// Try to increase capacity to ensure at least bytes are available
static inline bool arena_request_available(arena_module mod, arena_t *a,
                                           uint64_t bytes);

// Try to append zeros until next() has specified alignment
static inline bool arena_pad_to_alignment(arena_module mod, arena_t *a,
                                          uint64_t align);

// Try to append N bytes to arena
static inline bool arena_append_bytes(arena_module mod, arena_t *a,
                                      const unsigned char *bytes, size_t N);

// Try to append a single byte
static inline bool arena_append_byte(arena_module mod, arena_t *a,
                                      unsigned char byte);


// Allocate bytes space, returns offset from base address. Assert / UB if
// insufficient capacity.
static inline uint64_t arena_allocate_into_existing_capacity(arena_module mod,
                                                             arena_t *a,
                                                             uint64_t bytes);

// Returns ~0 on failure (the first allocation might be offset zero), attempting
// to increase capacity if out of space

static inline uint64_t arena_allocate(arena_module mod, arena_t *a,
                                      uint64_t bytes, uint64_t align);

struct arena_module_ty {
  arena_t (*const create)(uint64_t N);
  void (*const destroy)(arena_t);
  bool (*const valid)(arena_t);

  bool (*const base_address_constant)(void);
  bool (*const limit_address_constant)(void);
  uint64_t (*const alignment)(void);

  uint64_t (*const size)(arena_t);
  uint64_t (*const capacity)(arena_t);

  void *(*const base_address)(arena_t);
  void *(*const next_address)(arena_t);
  void *(*const limit_address)(arena_t);

  bool (*const change_capacity)(arena_t *a, uint64_t bytes);
  void (*const change_allocation)(arena_t *a, uint64_t bytes);

  uint64_t (*const allocate_into_existing_capacity)(arena_t *a, uint64_t bytes);

  // The maybe prefixed ones can be 0
  void (*const maybe_contract)(bool, const char *message,
                               size_t message_length);
};

#define ARENA_CONCAT2(X, Y) X##_##Y
#define ARENA_CONCAT(X, Y) ARENA_CONCAT2(X, Y)

#define ARENA_MODULE_INIT(PREFIX, CONTRACT)                                    \
  {                                                                            \
    .create = ARENA_CONCAT(PREFIX, create),                                    \
    .destroy = ARENA_CONCAT(PREFIX, destroy),                                  \
    .valid = ARENA_CONCAT(PREFIX, valid),                                      \
    .base_address_constant = ARENA_CONCAT(PREFIX, base_address_constant),      \
    .limit_address_constant = ARENA_CONCAT(PREFIX, limit_address_constant),    \
    .alignment = ARENA_CONCAT(PREFIX, alignment),                              \
    .size = ARENA_CONCAT(PREFIX, size),                                        \
    .capacity = ARENA_CONCAT(PREFIX, capacity),                                \
    .base_address = ARENA_CONCAT(PREFIX, base_address),                        \
    .next_address = ARENA_CONCAT(PREFIX, next_address),                        \
    .limit_address = ARENA_CONCAT(PREFIX, limit_address),                      \
    .change_capacity = ARENA_CONCAT(PREFIX, change_capacity),                  \
    .change_allocation = ARENA_CONCAT(PREFIX, change_allocation),              \
    .allocate_into_existing_capacity =                                         \
        ARENA_CONCAT(PREFIX, allocate_into_existing_capacity),                 \
    .maybe_contract = CONTRACT,                                                \
  }

static inline void arena_require_func(arena_module mod, bool expr,
                                      const char *message,
                                      size_t message_length) {
  (void)mod;
  const bool contract = ARENA_CONTRACT();
  if (contract & !expr) {
#if ARENA_CONTRACT()
    write(STDERR_FILENO, message, message_length);
    evilunit_contract_longjump();
    abort();
#else
    (void)name;
    (void)line;
#endif
  }
}

static inline arena_t arena_create(arena_module mod, uint64_t N) {
  arena_require(contract_is_power_of_two(arena_alignment(mod)));
  return mod->create(N);
}

static inline void arena_destroy(arena_module mod, arena_t a) {
  arena_require(arena_valid(mod, a));
  mod->destroy(a);
}

static inline bool arena_valid(arena_module mod, arena_t a) {
  if (!mod->valid(a)) {
    return false;
  }

  uint64_t base = (uint64_t)mod->base_address(a);
  uint64_t next = (uint64_t)mod->next_address(a);
  uint64_t limit = (uint64_t)mod->limit_address(a);
  return (base <= next) && (next <= limit);
}

static inline bool arena_base_address_constant(arena_module mod) {
  return mod->base_address_constant();
}

static inline bool arena_limit_address_constant(arena_module mod) {
  return mod->limit_address_constant();
}

static inline uint64_t arena_alignment(arena_module mod) {
  return mod->alignment();
}

static inline bool arena_contract_active(arena_module mod) {
  return mod->maybe_contract != 0;
}

static inline uint64_t arena_size(arena_module mod, arena_t a) {
  arena_require(arena_valid(mod, a));
  return mod->size(a);
}

static inline uint64_t arena_capacity(arena_module mod, arena_t a) {
  arena_require(arena_valid(mod, a));
  return mod->capacity(a);
}

static inline uint64_t arena_available(arena_module mod, arena_t a) {
  arena_require(arena_valid(mod, a));
  return arena_capacity(mod, a) - arena_size(mod, a);
}

static inline uint64_t arena_increment_needed_for_alignment(arena_module mod,
                                                            uint64_t base,
                                                            uint64_t align) {
  arena_require(contract_is_power_of_two(align));

  uint64_t modulo_as_mask = (align - 1);
  uint64_t rem = base & modulo_as_mask;

  arena_require(rem == (base % align));

  // Obvious calculation lowers to a select. and/sub/test/cmov
  uint64_t select = rem ? (align - rem) : 0;

  // But can also just mask it again. and/sub/and
  uint64_t snd_mask = (align - rem) & modulo_as_mask;
  arena_require(select == snd_mask);

  uint64_t result = select;

  arena_require(result < align);
  arena_require(((base + result) % align) == 0);
  return result;
}

static inline void *arena_base_address(arena_module mod, arena_t a) {
  arena_require(arena_valid(mod, a));
  void *res = mod->base_address(a);
  arena_require(arena_increment_needed_for_alignment(
                    mod, (uint64_t)res, arena_alignment(mod)) == 0);
  return res;
}

static inline void *arena_next_address(arena_module mod, arena_t a) {
  arena_require(arena_valid(mod, a));
  return mod->next_address(a);
}

static inline void *arena_limit_address(arena_module mod, arena_t a) {
  arena_require(arena_valid(mod, a));
  return mod->limit_address(a);
}

static inline bool arena_change_capacity(arena_module mod, arena_t *a,
                                         uint64_t bytes) {
  arena_t before = {0};
#if ARENA_CONTRACT()
  before = *a;
#endif

  arena_require(arena_valid(mod, *a));

  bool r = mod->change_capacity(a, bytes);

  if (!r) {
    arena_require(before.base == a->base);
    arena_require(before.next == a->next);
    arena_require(before.limit == a->limit);
  } else {
    arena_require(arena_valid(mod, *a));
    arena_require(arena_capacity(mod, *a) == bytes);
  }

  return r;
}

static inline void arena_change_allocation(arena_module mod, arena_t *a,
                                           uint64_t bytes) {
  arena_require(arena_valid(mod, *a));
  arena_require(bytes <= arena_capacity(mod, *a));
  mod->change_allocation(a, bytes);
  arena_require(((char *)arena_base_address(mod, *a) + bytes) ==
                (char *)arena_next_address(mod, *a));
}

static inline void arena_discard_last_allocated(arena_module mod, arena_t *a,
                                                uint64_t bytes)

{
  arena_require(arena_valid(mod, *a));
  arena_require(bytes <= arena_size(mod, *a));
  uint64_t next = arena_next_offset(mod, *a);
  uint64_t revised = next - bytes;
  arena_change_allocation(mod, a, revised);
}

static inline bool arena_request_available(arena_module mod, arena_t *a,
                                           uint64_t bytes) {
  arena_require(arena_valid(mod, *a));
  uint64_t req_size = arena_size(mod, *a) + bytes;
  if (req_size < arena_capacity(mod, *a)) {
    return true;
  }

  bool r = arena_change_capacity(mod, a, req_size);
  if (r) {
    arena_require(arena_available(mod, *a) >= bytes);
  }
  return r;
}

static inline bool arena_pad_to_alignment(arena_module mod, arena_t *a,
                                          uint64_t align) {
  arena_require(arena_valid(mod, *a));
  arena_require(contract_is_power_of_two(align));
  arena_require(align <= arena_alignment(mod));

  uint64_t next_offset = arena_next_offset(mod, *a);

  uint64_t n = arena_increment_needed_for_alignment(mod, next_offset, align);
  if (n > 0) {
    uint64_t r = arena_allocate(mod, a, n, 1);
    if (r == UINT64_MAX) {
      return false;
    }
    char *cursor = (char *)arena_base_address(mod, *a) + r;
    for (uint64_t i = 0; i < n; i++) {
      cursor[i] = 0;
    }
  }
  return true;
}

static inline bool arena_append_bytes(arena_module mod, arena_t *a,
                                      const unsigned char *bytes, size_t N) {
  arena_require(arena_valid(mod, *a));
  if (N == 0) { return true; }
  uint64_t r = arena_allocate(mod, a, N, 1);
  if (r == UINT64_MAX) {
    return false;
  }

  char *cursor = (char *)arena_base_address(mod, *a) + r;
  __builtin_memcpy(cursor, bytes, N);
  return true;
}

static inline bool arena_append_byte(arena_module mod, arena_t *a,
                                      unsigned char byte)
{
  unsigned char array[1] = {byte};
  return arena_append_bytes(mod, a, &array[0], 1);
}



// Allocate bytes space, returns offset from base address. Assert / UB if
// insufficient capacity.
static inline uint64_t arena_allocate_into_existing_capacity(arena_module mod,
                                                             arena_t *a,
                                                             uint64_t bytes) {
  arena_require(arena_valid(mod, *a));
  uint64_t available_before = arena_available(mod, *a);
  uint64_t size_before = arena_size(mod, *a);

  arena_require(bytes <= available_before);

  uint64_t r = mod->allocate_into_existing_capacity(a, bytes);

  arena_require((available_before - bytes) == arena_available(mod, *a));
  arena_require((size_before + bytes) == arena_size(mod, *a));

  return r;
}

// Returns ~0 on failure (the first allocation might be offset zero), attempting
// to increase capacity if out of space
static inline uint64_t arena_allocate(arena_module mod, arena_t *a,
                                      uint64_t bytes, uint64_t align) {
  arena_require(arena_valid(mod, *a));
  arena_require(contract_is_power_of_two(align));
  arena_require(align <= arena_alignment(mod));
  uint64_t base = (uint64_t)arena_base_address(mod, *a);
  uint64_t offset = arena_next_offset(mod, *a);

  uint64_t available = arena_available(mod, *a);

  // Somewhat ambiguous alignment
  // Either the offset from base could be aligned, or
  // the offset could be chosen to align base+offset
  // The latter wouldn't handle moving base very well
  // Thus it seems to follow that the arena has a maximum alignment
  // or that the maximum requested alignment needs to be tracked
  // and handled on reallocation

  uint64_t align_padding =
      arena_increment_needed_for_alignment(mod, offset, align);

  uint64_t total_alloc = bytes + align_padding;

  if (available < total_alloc) {
    uint64_t shortfall = total_alloc - available;
    uint64_t cap = arena_capacity(mod, *a);
    bool r = arena_change_capacity(mod, a, cap + shortfall);
    if (!r) {
      return ~UINT64_C(0);
    }
    arena_require(arena_capacity(mod, *a) >= (cap + shortfall));
  }
  arena_require(arena_available(mod, *a) >= total_alloc);

  uint64_t got = arena_allocate_into_existing_capacity(mod, a, total_alloc);
  uint64_t res = offset + align_padding;
  arena_require(got == offset);
  arena_require(res <= (offset + total_alloc));
  arena_require(arena_increment_needed_for_alignment(mod, res, align) == 0);
  arena_require(arena_increment_needed_for_alignment(mod, base + res, align) ==
                0);
  return res;
}

#endif
