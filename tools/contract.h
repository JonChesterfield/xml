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

static inline bool contract_jmpbuf_is_zero(jmp_buf * buf)
{
  _Static_assert(sizeof(jmp_buf) % 8 == 0, "");
  bool ret = true;

#if 1
  // Requires may_alias attribute, gives better codegen
  typedef uint64_t __attribute__((__may_alias__)) aliasing_uint64;
  
  aliasing_uint64 * cursor = (aliasing_uint64*)buf;

  for (size_t i = 0; i < sizeof(jmp_buf)/8; i++)
    {
      ret &= (cursor[i] == 0);
    }
#else


  unsigned char * cursor = (unsigned char *)buf;

#ifdef __has_builtin
#if __has_builtin( __builtin_assume_aligned)
  // damage mitigation
  cursor = __builtin_assume_aligned(cursor, _Alignof(jmp_buf));
#endif
#endif
  for (size_t i = 0; i < sizeof(jmp_buf); i++)
    {
      ret &= (cursor[i] == 0);
    }
#endif
  
  return ret;
}

static inline void contract_jmpbuf_set_zero(jmp_buf * buf)
{
  jmp_buf tmp = {0};
  
  unsigned char * src = (unsigned char *)&tmp;
  unsigned char * dst = (unsigned char *)buf;
  for (size_t i = 0; i < sizeof(jmp_buf); i++)
    {
      dst[i] = src[i];
    }
}

#endif


static inline void contract_active(bool, void*, const char*, size_t);
static inline void contract_discard(bool, void*, const char*, size_t);
static inline void contract_unit_test(bool, void*, const char*, size_t);

static inline void contract_write(bool, void*, const char*, size_t);
static inline void contract_exit(bool, void*, const char*, size_t);
static inline void contract_longjmp(bool, void*, const char*, size_t);

static inline void contract_active(bool expr, void* maybe_jmp_buf, const char *message, size_t message_length)
{
  // Write warnings, longjmp if a buffer is available, exit regardless
  if (!expr)
    {
      contract_write(expr, maybe_jmp_buf, message, message_length);
      contract_longjmp(expr, maybe_jmp_buf, message, message_length);
      contract_exit(expr, maybe_jmp_buf, message, message_length);
    }
}

static inline void contract_unit_test(bool expr, void* maybe_jmp_buf, const char *message, size_t message_length)
{
  // Variant more appropriate for unit tests - no printing unless longjmp is called out of context
  if (!expr)
    {
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
  if (maybe_jmp_buf)
    {
      if (contract_jmpbuf_is_zero(maybe_jmp_buf))
        {
          contract_write(false, 0, "Warning: Zero jmp_buf\n", sizeof("Warning: Zero jmp_buf\n")-1);
          contract_write(expr, 0, message, message_length);
          return;
        }
      
      if (!expr)
        {
          jmp_buf * buf = maybe_jmp_buf;
          longjmp(*buf, 1);
        }
      else
        {
          // reasonable point to reset it to zero
          contract_jmpbuf_set_zero(maybe_jmp_buf);
        }
    }
  
#else
  contract_discard(expr, maybe_jmp_buf, message, message_length);  
#endif
}

#endif
