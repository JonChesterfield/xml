#ifndef LINUX_SYSCALL_H_INCLUDED
#define LINUX_SYSCALL_H_INCLUDED

#ifndef __STDC_VERSION__
#define LINUX_SYSCALL_INLINE
#else
#if __STDC_VERSION__ == 199409L
#define LINUX_SYSCALL_INLINE
#else
#define LINUX_SYSCALL_INLINE inline
#endif
#endif

#define LINUX_SYSCALL_ATTRIBUTE static __attribute__((unused)) LINUX_SYSCALL_INLINE

#if 0
/*
 * this sequence avoids uninitialised warnings on clang
 * haven't found the right invocation for gcc yet so taking =0 performance hit
 */
#ifdef __clang__
  u = *(&u);
#else
  u = 0;
#endif
#endif

LINUX_SYSCALL_ATTRIBUTE unsigned long linux_syscall6(unsigned long n, unsigned long a0,
                                     unsigned long a1, unsigned long a2,
                                     unsigned long a3, unsigned long a4,
                                     unsigned long a5);

LINUX_SYSCALL_ATTRIBUTE unsigned long linux_syscall0(unsigned long n) {
  unsigned long u;
#ifdef __clang__
  u = *(&u);
#else
  u = 0;
#endif
  return linux_syscall6(n, u, u, u, u, u, u);
}

LINUX_SYSCALL_ATTRIBUTE unsigned long linux_syscall1(unsigned long n, unsigned long a0) {
  unsigned long u;
#ifdef __clang__
  u = *(&u);
#else
  u = 0;
#endif
  return linux_syscall6(n, a0, u, u, u, u, u);
}

LINUX_SYSCALL_ATTRIBUTE unsigned long linux_syscall2(unsigned long n, unsigned long a0,
                                     unsigned long a1) {
  unsigned long u;
#ifdef __clang__
  u = *(&u);
#else
  u = 0;
#endif
  return linux_syscall6(n, a0, a1, u, u, u, u);
}

LINUX_SYSCALL_ATTRIBUTE unsigned long linux_syscall3(unsigned long n, unsigned long a0,
                                     unsigned long a1, unsigned long a2) {
  unsigned long u;
#ifdef __clang__
  u = *(&u);
#else
  u = 0;
#endif
  return linux_syscall6(n, a0, a1, a2, u, u, u);
}

LINUX_SYSCALL_ATTRIBUTE unsigned long linux_syscall4(unsigned long n, unsigned long a0,
                                     unsigned long a1, unsigned long a2,
                                     unsigned long a3) {
  unsigned long u;
#ifdef __clang__
  u = *(&u);
#else
  u = 0;
#endif
  return linux_syscall6(n, a0, a1, a2, a3, u, u);
}

LINUX_SYSCALL_ATTRIBUTE unsigned long linux_syscall5(unsigned long n, unsigned long a0,
                                     unsigned long a1, unsigned long a2,
                                     unsigned long a3, unsigned long a4) {
  unsigned long u;
#ifdef __clang__
  u = *(&u);
#else
  u = 0;
#endif
  return linux_syscall6(n, a0, a1, a2, a3, a4, u);
}

#if __x86_64__
 _Static_assert(sizeof(unsigned long) == 8, "");
LINUX_SYSCALL_ATTRIBUTE unsigned long linux_syscall6(unsigned long n, unsigned long a0,
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

#elif __aarch64__
 _Static_assert(sizeof(unsigned long) == 8, "");
LINUX_SYSCALL_ATTRIBUTE unsigned long linux_syscall6(unsigned long n, unsigned long a0,
                                     unsigned long a1, unsigned long a2,
                                     unsigned long a3, unsigned long a4,
                                     unsigned long a5) {
  unsigned long ret = 0;
  register unsigned long x0 __asm__("x0") = a0;
  register unsigned long x1 __asm__("x1") = a1;
  register unsigned long x2 __asm__("x2") = a2;
  register unsigned long x3 __asm__("x3") = a3;
  register unsigned long x4 __asm__("x4") = a4;
  register unsigned long x5 __asm__("x5") = a5;

  register unsigned long x8 __asm__("x8") = n;


  // Having a bad time finding documentation on what other registers linux might clobber
  // Going to take a guess at x6 and x7 for now, based on N being passed in 8
  // Also asm syntax rejected those on the clobbers list so putting them in output

  register unsigned long x6 __asm__("x6");
  register unsigned long x7 __asm__("x7");
  
  __asm__ volatile("svc #0"
                   : "=r"(ret), "=r"(x6), "=r"(x7)
                   : "0"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5), "r"(x8)
                   :  "memory");
  return ret;
  
}
#else

#error "Linux syscall unimplemented for this architecture"

#endif

#endif
