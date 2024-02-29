#ifndef TOOLS_REGEX_H_INCLUDED
#define TOOLS_REGEX_H_INCLUDED

#include "regex.declarations.h"
#include "regex.ptree.h"

#include "../tools/arena.h"

static inline bool regex_is_empty_set(ptree val) {
  uint64_t id = regex_ptree_identifier(val);
  return id == regex_grouping_empty_set;
}

static inline bool regex_is_empty_string(ptree val) {
  uint64_t id = regex_ptree_identifier(val);
  return id == regex_grouping_empty_string;
}

static inline bool regex_is_kleene(ptree val) {
  uint64_t id = regex_ptree_identifier(val);
  return id == regex_grouping_kleene;
}

static inline bool regex_is_concat(ptree val) {
  uint64_t id = regex_ptree_identifier(val);
  return id == regex_grouping_concat;
}

static inline bool regex_is_and(ptree val) {
  uint64_t id = regex_ptree_identifier(val);
  return id == regex_grouping_and;
}

static inline bool regex_is_or(ptree val) {
  uint64_t id = regex_ptree_identifier(val);
  return id == regex_grouping_or;
}

static inline bool regex_is_not(ptree val) {
  uint64_t id = regex_ptree_identifier(val);
  return id == regex_grouping_not;
}

static inline ptree regex_make_empty_set(ptree_context ctx) {
  return regex_ptree_expression0(ctx, regex_grouping_empty_set);
}
static inline ptree regex_make_empty_string(ptree_context ctx) {
  return regex_ptree_expression0(ctx, regex_grouping_empty_string);
}
static inline ptree regex_make_kleene(ptree_context ctx, ptree val) {
  return regex_ptree_expression1(ctx, regex_grouping_kleene, val);
}
static inline ptree regex_make_not(ptree_context ctx, ptree val) {
  return regex_ptree_expression1(ctx, regex_grouping_not, val);
}
static inline ptree regex_make_range(ptree_context ctx, ptree lhs, ptree rhs) {
  // todo, force these to be bytes somewhere
  // uint64_t lhs_id = regex_ptree_identifier(lhs);
  // uint64_t rhs_id = regex_ptree_identifier(rhs);
  return regex_ptree_expression2(ctx, regex_grouping_range, lhs, rhs);
}

static inline ptree regex_make_concat(ptree_context ctx, ptree lhs, ptree rhs) {
  return regex_ptree_expression2(ctx, regex_grouping_concat, lhs, rhs);
}

static inline ptree regex_make_or(ptree_context ctx, ptree lhs, ptree rhs) {
  return regex_ptree_expression2(ctx, regex_grouping_or, lhs, rhs);
}

static inline ptree regex_make_and(ptree_context ctx, ptree lhs, ptree rhs) {
  return regex_ptree_expression2(ctx, regex_grouping_and, lhs, rhs);
}

ptree regex_nullable(ptree_context ctx, ptree val);
ptree regex_derivative(ptree_context ctx, ptree val, uint8_t byte);
ptree regex_canonicalise(ptree_context ctx, ptree val);

void regex_split(ptree_context ctx, ptree val, ptree *edges);

int regex_to_char_sequence(arena_module mod, arena_t *arena, ptree val);

#endif