#ifndef TOOLS_CONTRACT_H_INCLUDED
#define TOOLS_CONTRACT_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "syscall.h"

// Possibly dubious to assume __NR_write etc are consistent across architectures
#if defined(__x86_64__)
#if __has_include(<x86_64-linux-gnu/asm/unistd_64.h>)
#include <x86_64-linux-gnu/asm/unistd_64.h>
#elif __has_include(<asm/unistd_64.h>)
#include <asm/unistd_64.h>
#else
#error "On x64, missing unistd_64.h"
#endif
#endif

#if defined(__aarch64__)
// Might have the include path wrong for this
#include <asm-generic/unistd.h>
#endif

#include "EvilUnit/EvilUnit_contract.h"

#define contract_message1(E, F, L) "Fail [" #E "]: " #F " L" #L "\n"
#define contract_message0(E, F, L) contract_message1(E, F, L)
#define contract_message(EXPR) contract_message0(EXPR, __FILE__, __LINE__)

// Here mostly because it gets used by various contracts
static inline bool contract_is_zero_or_power_of_two(uint64_t x) {
  return !(x & (x - 1));
}

static inline bool contract_is_power_of_two(uint64_t x) {
  return x && contract_is_zero_or_power_of_two(x);
}

static inline void contract_active(bool, const char *, size_t);
static inline void contract_discard(bool, const char *, size_t);
static inline void contract_unit_test(bool, const char *, size_t);

static inline void contract_write(bool, const char *, size_t);
static inline void contract_exit(bool, const char *, size_t);
static inline void contract_longjmp(bool, const char *, size_t);

static inline void contract_active(bool expr, const char *message,
                                   size_t message_length) {
  // Write warnings, longjmp if a buffer is available, exit regardless
  if (!expr) {
    contract_write(expr, message, message_length);
    contract_longjmp(expr, message, message_length);
    contract_exit(expr, message, message_length);
  }
}

static inline void contract_unit_test(bool expr, const char *message,
                                      size_t message_length) {
  // Variant more appropriate for unit tests - no printing unless longjmp is
  // called out of context
  if (!expr) {
    contract_longjmp(expr, message, message_length);
    contract_exit(expr, message, message_length);
  }
}

static inline void contract_discard(bool expr, const char *message,
                                    size_t message_length) {
  (void)expr;
  (void)message;
  (void)message_length;
}

static inline void contract_write(bool expr, const char *message,
                                  size_t message_length) {
  if (!expr) {
    int64_t r;
    // if write returns 0, give up instead of loop
    do {
      r = syscall3(__NR_write, 2, (uint64_t)message, message_length);
      message = &message[r];
      message_length -= r;
    } while ((r > 0) && ((uint64_t)r < message_length));
  }
  contract_discard(expr, message, message_length);
}

static inline void contract_exit(bool expr, const char *message,
                                 size_t message_length) {
  if (!expr) {
    syscall1(__NR_exit_group, 1);
  }
  contract_discard(expr, message, message_length);
}

static inline void contract_longjmp(bool expr, const char *message,
                                    size_t message_length) {
#ifdef CONTRACT_SETJMP_ERR_MSG
#error "Unexpected macro CONTRACT_SETJMP_ERR_MSG"
#endif
#define CONTRACT_SETJMP_ERR_MSG() "Error: Longjmp failed, missing setjmp?\n"

  if (!expr) {
    evilunit_contract_longjump();

    contract_write(false, CONTRACT_SETJMP_ERR_MSG(),
                   sizeof(CONTRACT_SETJMP_ERR_MSG()) - 1);
    contract_write(expr, message, message_length);

    contract_exit(expr, message, message_length);
  }

#undef CONTRACT_SETJMP_ERR_MSG
}

#endif
