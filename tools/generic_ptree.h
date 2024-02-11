#ifndef GENERIC_PTREE_H_INCLUDED
#define GENERIC_PTREE_H_INCLUDED

#include "ptree.h"

// A parse tree is a collection of tokens and expressions, each with a uint64_t
// id This generic form prints tokens as (Tnum value) and may introduce (Hnum
// value) if the value needs to be hex encoded to round trip nicely. Expressions
// as (enum ...). Numbers in base 10 for now. The token length is derived from
// value. A generic parse tree may be representable in some more specific parse
// tree if the numbers, arities etc are meaningful in that context.

PTREE_INSTANTIATE_DECLARE(generic);

ptree generic_ptree_from_other_ptree(ptree_context context,
                                     const ptree_module *mod, ptree other);

void generic_ptree_to_file(FILE *file, ptree tree);

#endif
