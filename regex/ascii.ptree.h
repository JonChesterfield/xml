#ifndef REGEX_ASCII_PTREE_H_INCLUDED
#define REGEX_ASCII_PTREE_H_INCLUDED
#include "regex.ptree.h"

static inline ptree_context ascii_ptree_create_context(void) {return regex_ptree_create_context();}
static inline bool ascii_ptree_valid_context(ptree_context ctx) {return regex_ptree_valid_context(ctx);}
static inline void ascii_ptree_destroy_context(ptree_context ctx){regex_ptree_destroy_context(ctx);}


static inline ptree ascii_ptree_expression_create_uninitialised(ptree_context ctx,
                                                  uint64_t id, size_t N)
{
  return regex_ptree_expression_create_uninitialised(ctx, id, N);

}

static inline void ascii_ptree_as_xml(stack_module stackmod, FILE *file, ptree tree)
{
  regex_ptree_as_xml(stackmod, file, tree);
}

#endif
