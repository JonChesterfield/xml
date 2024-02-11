#ifndef PTREE_H_INCLUDED
#define PTREE_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Covers preconditions and postconditions.
// E.g. no asking for the token value of a ptree that isn't dynamically a token.
#define PTREE_CONTRACT() 1

#if PTREE_CONTRACT()
#include <stdio.h>
#include <stdlib.h>
#endif

// Static information is in ptree_module. Dynamic information is in
// ptree_context, notably memory allocation. Per-node information in ptree.

typedef struct {
  void *state; // 0 if construction failed
} ptree_context;

// Not in use at present but probably should be
typedef struct {
  uint64_t state;
} ptree_id;

typedef struct {
  uint64_t state; // 0 if failure / sentinel / otherwise invalid
} ptree;

// An instance of this represents a specific ptree specialisation
struct ptree_module_ty;
typedef struct ptree_module_ty ptree_module;

// This interface is available given a module. A given specialisation may
// have an equivalent set with a prefix added and the *mod argument dropped.
// Implemented in ptree_impl.h header (included by this file) partly so the
// precondition/postcondition contract checks are readily visible.

// Optional, avoids passing &module to calls. Works well with inlining.
#include "ptree_macro_wrapper.h"

static inline ptree_context ptree_create_context(const ptree_module *mod);
static inline void ptree_destroy_context(const ptree_module *mod,
                                         ptree_context ctx);

static inline bool ptree_identifier_valid_token(const ptree_module *mod,
                                                uint64_t id);
static inline bool ptree_identifier_valid_expression(const ptree_module *mod,
                                                     uint64_t id);

// TODO: Probably don't want max/min elements in the external API
static inline size_t ptree_identifier_minimum_elements(const ptree_module *mod,
                                                       uint64_t id);

static inline size_t ptree_identifier_maximum_elements(const ptree_module *mod,
                                                       uint64_t id);
static inline ptree ptree_failure(void);
static inline bool ptree_is_failure(ptree p);
static inline uint64_t ptree_identifier(const ptree_module *mod, ptree p);

static inline bool ptree_is_token(const ptree_module *mod, ptree p);
static inline bool ptree_is_expression(const ptree_module *mod, ptree p);

static inline size_t ptree_minimum_elements(const ptree_module *mod, ptree p);
static inline size_t ptree_maximum_elements(const ptree_module *mod, ptree p);

static inline const char *ptree_token_value(const ptree_module *mod, ptree p);
static inline size_t ptree_token_width(const ptree_module *mod, ptree p);
static inline ptree ptree_from_token(const ptree_module *mod, ptree_context ctx,
                                     uint64_t id, const char *value,
                                     size_t width);

static inline size_t ptree_expression_elements(const ptree_module *mod,
                                               ptree p);
static inline ptree ptree_expression_element(const ptree_module *mod, ptree p,
                                             size_t index);

static inline ptree ptree_expression_append(const ptree_module *mod,
                                            ptree_context ctx, ptree base,
                                            ptree element);
static inline ptree ptree_expression_construct(const ptree_module *mod,
                                               ptree_context ctx, uint64_t id,
                                               size_t N, ptree *elts);

// Call into construct.
static inline ptree ptree_expression0(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id);
static inline ptree ptree_expression1(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0);
static inline ptree ptree_expression2(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1);
static inline ptree ptree_expression3(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2);
static inline ptree ptree_expression4(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3);
static inline ptree ptree_expression5(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4);
static inline ptree ptree_expression6(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4,
                                      ptree x5);
static inline ptree ptree_expression7(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4,
                                      ptree x5, ptree x6);
static inline ptree ptree_expression8(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4,
                                      ptree x5, ptree x6, ptree x7);

struct ptree_module_ty {
  ptree_context (*const create_context)(void);
  void (*const destroy_context)(ptree_context);

  bool (*const identifier_valid_token)(uint64_t);
  bool (*const identifier_valid_expression)(uint64_t);

  // A given expression identifier might have hard constraints on the number
  // of elements it can have, e.g. exactly 2 or at most 4
  // Maximum is UINT64_MAX for no limit.
  // This applies to all instances of that expression.
  // Probably not one for the API
  size_t (*const identifier_minimum_elements)(uint64_t);
  size_t (*const identifier_maximum_elements)(uint64_t);

  uint64_t (*const identifier)(ptree); // zero if passed failure

  bool (*const is_token)(ptree);
  bool (*const is_expression)(ptree);

  const char *(*const token_value)(ptree);
  size_t (*const token_width)(ptree);
  ptree (*const from_token)(ptree_context, uint64_t id, const char *value,
                            size_t width);

  size_t (*const expression_elements)(ptree);

  ptree (*const expression_element)(ptree, size_t);

  ptree (*const expression_append)(ptree_context, ptree base, ptree element);

  ptree (*const expression_construct)(ptree_context, uint64_t id, size_t N,
                                      ptree *);
};

#include "ptree_impl.h"

#endif
