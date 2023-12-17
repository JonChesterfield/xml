#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdlib.h>

struct SToken
{
    int value;
    const char* token;
};

extern void *ParseAlloc(void *(*)(size_t));
extern void Parse(void *, int, struct SToken*);
extern void ParseFree(void *, void (*)(void*));

#endif
