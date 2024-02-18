#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

#include <stdint.h>

static inline uint64_t syscall6(uint64_t n, uint64_t a0, uint64_t a1, uint64_t a2,
                                uint64_t a3, uint64_t a4, uint64_t a5);

static inline uint64_t syscall0(uint64_t n)
{
  uint64_t u;
  u = *(&u);
  return syscall6(n, u, u, u, u, u, u);
}

static inline uint64_t syscall1(uint64_t n, uint64_t a0)
{
  uint64_t u;
  u = *(&u);
  return syscall6(n, a0, u, u, u, u, u);
}

static inline uint64_t syscall2(uint64_t n, uint64_t a0, uint64_t a1)
{
  uint64_t u;
  u = *(&u);
  return syscall6(n, a0, a1, u, u, u, u);
}

static inline uint64_t syscall3(uint64_t n, uint64_t a0, uint64_t a1, uint64_t a2)
{
  uint64_t u;
  u = *(&u);
  return syscall6(n, a0, a1, a2, u, u, u);
}

static inline uint64_t syscall4(uint64_t n, uint64_t a0, uint64_t a1, uint64_t a2,
                         uint64_t a3) {
  uint64_t u;
  u = *(&u);
  return syscall6(n, a0, a1, a2, a3, u, u);
}

static inline uint64_t syscall5(uint64_t n, uint64_t a0, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4) {
  uint64_t u;
  u = *(&u);
  return syscall6(n, a0, a1, a2, a3, a4, u);
}

static inline uint64_t syscall6(uint64_t n, uint64_t a0, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4, uint64_t a5) {
  uint64_t ret = 0;
  register uint64_t r10 __asm__("r10") = a3;
  register uint64_t r8 __asm__("r8") = a4;
  register uint64_t r9 __asm__("r9") = a5;

  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"(n), "D"(a0), "S"(a1), "d"(a2), "r"(r10), "r"(r8),
                     "r"(r9)
                   : "rcx", "r11", "memory");

  return ret;
}

#endif
