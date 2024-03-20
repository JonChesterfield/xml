#include "ascii.ptree.h"

#include "ascii.lexer.h"

bool ascii_ptree_identifier_valid_token(uint64_t id)
{
  return (id > ascii_token_UNKNOWN) && (id < ascii_token_count);
}
