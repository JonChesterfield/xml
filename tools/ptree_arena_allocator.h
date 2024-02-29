#ifndef PTREE_ARENA_ALLOCATOR_H
#define PTREE_ARENA_ALLOCATOR_H

// Let 0 mean unallocated
// Tokens in range (0, T)
// Expressions in range [T, E)
// Offset from start of buffer is sufficient information for a token -
// can retrieve value/width by running the regex again. Which is faster with the tag.
// Expressions have arity which is partly determined by T
//

// Tokens in contiguous memory, pointer known somewhere. Dealing in offsets.
// Expressions in different contiguous memory.

// using the first tag to indicate extension
enum { token_count = 10 };

// Using the same offset for token and expr states that the size of
// each contiguous memory are expected to be broadly similar, assumes
// one can respin if the pointer proves too small
//
// tag bits are deterministic function of language, initial offset guess
// derived from size of input (rounded up to a multiple of bytes, maybe words)

typedef struct {
  unsigned int tag : 8;
  unsigned int offset : 24;
} ptree_arena_node;

uint64_t offset(ptree_arena_node p)
{
  return p.offset;
}

bool is_unalloc(ptree_arena_node p)
{
  return p.tag == 0;
}

bool is_token(ptree_arena_node p)
{
  return p.tag > 0 && p.tag < token_count;
}

bool is_extension(ptree_arena_node p)
{
  return p.tag == token_count;
}

bool is_expr(ptree_arena_node p)
{
  return p.tag > token_count;
}

// might use an explicit arena instead of a stack
struct ptree_arena_context_ty {
  void * stack;
};

typedef struct ptree_arena_context_ty *ptree_arena_context;

_Static_assert(sizeof(ptree_context) == 8, "");
_Static_assert(sizeof(ptree_arena_context) == 8, "");

static ptree_arena_context
ptree_context_to_ptree_arena_context(ptree_context ctx) {
  ptree_arena_context res;
  __builtin_memcpy(&res, &ctx, 8);
  return res;
}

static ptree_context
ptree_arena_context_to_ptree_context(ptree_arena_context ctx) {
  ptree_context res;
  __builtin_memcpy(&res, &ctx, 8);
  return res;
}

