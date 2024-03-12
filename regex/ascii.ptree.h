#ifndef REGEX_ASCII_PTREE_H_INCLUDED
#define REGEX_ASCII_PTREE_H_INCLUDED
#include "regex.ptree.h"

static inline ptree ascii_ptree_expression_create_uninitialised(ptree_context ctx,
                                                  uint64_t id, size_t N)
{
  return regex_ptree_expression_create_uninitialised(ctx, id, N);

}


#endif
