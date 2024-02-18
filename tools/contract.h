#ifndef TOOLS_CONTRACT_H_INCLUDED
#define TOOLS_CONTRACT_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "syscall.h"

#include <x86_64-linux-gnu/asm/unistd_64.h>

#define contract_message1(E, F, L) "Fail [" #E "]: " #F " L" #L "\n"
#define contract_message0(E, F, L) contract_message1(E, F, L) 
#define contract_message(EXPR) contract_message0(EXPR, __FILE__, __LINE__)

#if __STDC_HOSTED__
#include <setjmp.h>
#endif


static inline void contract_active(bool, void*, const char*, size_t);
static inline void contract_discard(bool, void*, const char*, size_t);

static inline void contract_write(bool, void*, const char*, size_t);
static inline void contract_exit(bool, void*, const char*, size_t);
static inline void contract_longjmp(bool, void*, const char*, size_t);

static inline void contract_active(bool expr, void* maybe_jmp_buf, const char *message, size_t message_length)
{
  if (!expr)
    {
      contract_write(expr, maybe_jmp_buf, message, message_length);
      contract_longjmp(expr, maybe_jmp_buf, message, message_length);
      contract_exit(expr, maybe_jmp_buf, message, message_length);
    }
}
  
static inline void contract_discard(bool expr, void* maybe_jmp_buf, const char *message, size_t message_length)
{
  (void)expr;
  (void)maybe_jmp_buf;
  (void)message;
  (void)message_length;
}

static inline void contract_write(bool expr, void* maybe_jmp_buf, const char *message, size_t message_length)
{
  if (!expr)
    {
      int64_t r;
      // if write returns 0, give up instead of loop
      do
        {
          r = syscall3(__NR_write, 2, (uint64_t)message, message_length);
          message = &message[r];
          message_length -= r;
        } while ((r > 0) && ((uint64_t)r < message_length));
    }
  contract_discard(expr, maybe_jmp_buf, message, message_length);  
}

static inline void contract_exit(bool expr, void* maybe_jmp_buf, const char *message, size_t message_length)
{
  if (!expr)
    {
      syscall1(__NR_exit_group, 1);     
    }
  contract_discard(expr, maybe_jmp_buf, message, message_length);  
}

static inline void contract_longjmp(bool expr, void* maybe_jmp_buf, const char *message, size_t message_length)
{
#if __STDC_HOSTED__
  (void)message;
  (void)message_length;
  if (!expr)
    {
      if (maybe_jmp_buf)
      {
        jmp_buf * buf = maybe_jmp_buf;      
        longjmp(*buf, 1);
      }
    }
#else
  contract_discard(expr, maybe_jmp_buf, message, message_length);  
#endif
}

#endif
