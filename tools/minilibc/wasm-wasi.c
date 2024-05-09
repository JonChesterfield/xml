#include "../minilibc.h"

#include <stddef.h>
#include <stdint.h>

// Partial implementation (exit and write)

#if __wasi__

typedef int __wasi_fd_t;
typedef long ssize_t;
typedef uint16_t __wasi_errno_t;
typedef long unsigned int __wasi_size_t;

typedef struct __wasi_ciovec_t {
    const uint8_t * buf;
    __wasi_size_t buf_len;
} __wasi_ciovec_t;


static ssize_t write(int, const void *, size_t);

static __wasi_errno_t __wasi_fd_write(
    __wasi_fd_t fd,
    const __wasi_ciovec_t *iovs,
    size_t iovs_len,
    __wasi_size_t *retptr0
) __attribute__((__warn_unused_result__));

int32_t __imported_wasi_snapshot_preview1_fd_write(int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("fd_write")
));


#define __WASI_ERRNO_NOTCAPABLE (UINT16_C(76))
#define ENOTCAPABLE __WASI_ERRNO_NOTCAPABLE
#define EBADF            9

// The libc functions expect to set errno, but errno isn't exposed to
// the users of this file so stubbing it out
//_Thread_local int errno = 0;

static ssize_t write(int fildes, const void *buf, size_t nbyte) {
  __wasi_ciovec_t iov = {.buf = (const uint8_t *)buf, .buf_len = nbyte};
  size_t bytes_written;
  __wasi_errno_t error =
      __wasi_fd_write(fildes, &iov, 1, &bytes_written);
  if (error != 0) {
    // errno = error == ENOTCAPABLE ? EBADF : error;
    return -1;
  }
  return bytes_written;
}

uint64_t minilibc_write(uint64_t file, uint64_t message, uint64_t length)
{
  int f = (int)file;
  ssize_t r = write(f, (const void*)message, length);
  return (uint64_t)r;
}

__wasi_errno_t __wasi_fd_write(
    __wasi_fd_t fd,
    const __wasi_ciovec_t *iovs,
    size_t iovs_len,
    __wasi_size_t *retptr0
){
    int32_t ret = __imported_wasi_snapshot_preview1_fd_write((int32_t) fd, (int32_t) iovs, (int32_t) iovs_len, (int32_t) retptr0);
    return (uint16_t) ret;
}



_Noreturn void __imported_wasi_snapshot_preview1_proc_exit(int32_t arg0) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("proc_exit")
));

void minilibc_exit(uint64_t status)
{
  uint32_t r = (uint32_t)status;
  __imported_wasi_snapshot_preview1_proc_exit((int32_t) r);
}

#endif
