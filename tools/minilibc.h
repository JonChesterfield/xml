#ifndef TOOLS_MINILIBC_H_INCLUDED
#define TOOLS_MINILIBC_H_INCLUDED

#include <stdint.h>

uint64_t minilibc_write(uint64_t file, uint64_t message, uint64_t length);
uint64_t minilibc_read(uint64_t file, uint64_t message, uint64_t length);
uint64_t minilibc_brk(uint64_t);
void minilibc_exit(void);

#endif
