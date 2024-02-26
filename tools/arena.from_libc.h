#ifndef ARENA_FROM_LIBC_H_INCLUDED
#define ARENA_FROM_LIBC_H_INCLUDED

// Possible this project should spawn code generators instead of doing this

#ifndef ARENA_CONCAT
#error "Missing ARENA_CONCAT macro"
#endif

#define ARENA_FROM_LIBC(PREFIX, MALLOC, REALLOC, FREE)                         \
                                                                               \
  static arena_t ARENA_CONCAT(PREFIX, create)(uint64_t N) {                    \
    arena_t r = {0};                                                           \
    void *d = MALLOC(N);                                                       \
    if (d) {                                                                   \
      r.base = (uint64_t)d;                                                    \
      r.next = (uint64_t)d;                                                    \
      r.limit = (uint64_t)d + N;                                               \
    }                                                                          \
    return r;                                                                  \
  }                                                                            \
                                                                               \
  static void ARENA_CONCAT(PREFIX, destroy)(arena_t a) {                       \
    void *d = (void *)a.base;                                                  \
    FREE(d);                                                                   \
  }                                                                            \
                                                                               \
  static bool ARENA_CONCAT(PREFIX, valid)(arena_t a) { return a.base != 0; }   \
                                                                               \
  static bool ARENA_CONCAT(PREFIX, base_address_constant)(void) {              \
    return false;                                                              \
  }                                                                            \
  static bool ARENA_CONCAT(PREFIX, limit_address_constant)(void) {             \
    return false;                                                              \
  }                                                                            \
  static uint64_t ARENA_CONCAT(PREFIX, alignment)(void) {                      \
    return _Alignof(max_align_t);                                              \
  }                                                                            \
                                                                               \
  static uint64_t ARENA_CONCAT(PREFIX, size)(arena_t a) {                      \
    return a.next - a.base;                                                    \
  }                                                                            \
                                                                               \
  static uint64_t ARENA_CONCAT(PREFIX, capacity)(arena_t a) {                  \
    return a.limit - a.base;                                                   \
  }                                                                            \
                                                                               \
  static void *ARENA_CONCAT(PREFIX, base_address)(arena_t a) {                 \
    return (void *)a.base;                                                     \
  }                                                                            \
                                                                               \
  static void *ARENA_CONCAT(PREFIX, next_address)(arena_t a) {                 \
    return (void *)a.next;                                                     \
  }                                                                            \
                                                                               \
  static void *ARENA_CONCAT(PREFIX, limit_address)(arena_t a) {                \
    return (void *)a.limit;                                                    \
  }                                                                            \
                                                                               \
  static bool ARENA_CONCAT(PREFIX, change_capacity)(arena_t * a,               \
                                                    uint64_t bytes) {          \
    char *base = (char *)a->base;                                              \
    char *next = (char *)a->next;                                              \
    char *limit = (char *)a->limit;                                            \
                                                                               \
    uint64_t next_offset = next - base;                                        \
    uint64_t capacity = limit - base;                                          \
    if (bytes == capacity) {                                                   \
      return true;                                                             \
    }                                                                          \
                                                                               \
    char *r = REALLOC(base, bytes);                                            \
    if (!r) {                                                                  \
      return false;                                                            \
    }                                                                          \
                                                                               \
    a->base = (uint64_t)r;                                                     \
    a->next = (uint64_t)r + next_offset;                                       \
    a->limit = (uint64_t)r + bytes;                                            \
                                                                               \
    return true;                                                               \
  }                                                                            \
                                                                               \
  static uint64_t ARENA_CONCAT(PREFIX, allocate_into_existing_capacity)(       \
      arena_t * a, uint64_t bytes) {                                           \
    char *base = (char *)a->base;                                              \
    char *next = (char *)a->next;                                              \
    char *incr = next + bytes;                                                 \
    a->next = (uint64_t)incr;                                                  \
    return (uint64_t)(next - base);                                            \
  }

#endif
