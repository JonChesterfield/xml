#include "../minilibc.h"

#ifdef __linux__

#include "linux_syscall.h"

// Possibly dubious to assume __NR_write etc are consistent across architectures

#if defined(__x86_64__)
  #if __has_include(<x86_64-linux-gnu/asm/unistd_64.h>)
    #include <x86_64-linux-gnu/asm/unistd_64.h>
  #elif __has_include(<asm/unistd_64.h>)
    #include <asm/unistd_64.h>
  #else
    #error "On x64, can't find unistd_64.h"
  #endif
#endif

#if defined(__aarch64__)
  #if __has_include(<asm-generic/unistd.h>)
    #include <asm-generic/unistd.h>
  #else
    #error "On aarch64, can't find syscall definitions"
  #endif
#endif

uint64_t minilibc_write(uint64_t file, uint64_t message, uint64_t length)
{
  return linux_syscall3(__NR_write, file, message, length);
}

uint64_t minilibc_read(uint64_t file, uint64_t message, uint64_t length)
{
  return linux_syscall3(__NR_read, file, message, length);
}

uint64_t minilibc_brk(uint64_t x)
{
  return linux_syscall1(__NR_brk, x);
}

void minilibc_exit(void)
{
  linux_syscall1(__NR_exit_group, 1);
}

#endif
