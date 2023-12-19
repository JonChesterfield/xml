#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdio.h>
#include <stdlib.h>

struct SToken
{
    int value;
    const char* token;
};


// Shows up in the prototype
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include "parse.h"
#ifdef __cplusplus
}
#endif
#endif
