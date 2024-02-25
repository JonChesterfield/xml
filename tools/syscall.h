#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

#if __x86_64__

_Static_assert(sizeof(unsigned long) == 8, "");

static inline unsigned long syscall6(unsigned long n, unsigned long a0,
                                     unsigned long a1, unsigned long a2,
                                     unsigned long a3, unsigned long a4,
                                     unsigned long a5);

static inline unsigned long syscall0(unsigned long n) {
  unsigned long u;
  u = *(&u);
  return syscall6(n, u, u, u, u, u, u);
}

static inline unsigned long syscall1(unsigned long n, unsigned long a0) {
  unsigned long u;
  u = *(&u);
  return syscall6(n, a0, u, u, u, u, u);
}

static inline unsigned long syscall2(unsigned long n, unsigned long a0,
                                     unsigned long a1) {
  unsigned long u;
  u = *(&u);
  return syscall6(n, a0, a1, u, u, u, u);
}

static inline unsigned long syscall3(unsigned long n, unsigned long a0,
                                     unsigned long a1, unsigned long a2) {
  unsigned long u;
  u = *(&u);
  return syscall6(n, a0, a1, a2, u, u, u);
}

static inline unsigned long syscall4(unsigned long n, unsigned long a0,
                                     unsigned long a1, unsigned long a2,
                                     unsigned long a3) {
  unsigned long u;
  u = *(&u);
  return syscall6(n, a0, a1, a2, a3, u, u);
}

static inline unsigned long syscall5(unsigned long n, unsigned long a0,
                                     unsigned long a1, unsigned long a2,
                                     unsigned long a3, unsigned long a4) {
  unsigned long u;
  u = *(&u);
  return syscall6(n, a0, a1, a2, a3, a4, u);
}

static inline unsigned long syscall6(unsigned long n, unsigned long a0,
                                     unsigned long a1, unsigned long a2,
                                     unsigned long a3, unsigned long a4,
                                     unsigned long a5) {
  unsigned long ret = 0;
  register unsigned long r10 __asm__("r10") = a3;
  register unsigned long r8 __asm__("r8") = a4;
  register unsigned long r9 __asm__("r9") = a5;

  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"(n), "D"(a0), "S"(a1), "d"(a2), "r"(r10), "r"(r8),
                     "r"(r9)
                   : "rcx", "r11", "memory");

  return ret;
}

#else

#error "Unimplemented architecture"

#endif

#endif
