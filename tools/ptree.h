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

// The fast and convenient way to use this datatype is with a static const
// ptree_module instance. Given one of those, one can instantiate a set of
// functions that call into the function pointers defined within, which will
// usually be devirtualised.

#define PTREE_CONCAT2(X, Y) X##_ptree_##Y
#define PTREE_CONCAT(X, Y) PTREE_CONCAT2(X, Y)

#define PTREE_INSTANTIATE_DECLARE(PREFIX)                                      \
  ptree_context PTREE_CONCAT(PREFIX, create_context)(void);                    \
  void PTREE_CONCAT(PREFIX, destroy_context)(ptree_context ctx);               \
  const ptree_module* PTREE_CONCAT(PREFIX, retrieve_module)(void);                   \
  bool PTREE_CONCAT(PREFIX, identifier_is_failure)(uint64_t id);               \
  bool PTREE_CONCAT(PREFIX, identifier_valid_token)(uint64_t id);                 \
  bool PTREE_CONCAT(PREFIX, identifier_valid_expression)(uint64_t id);            \
  size_t PTREE_CONCAT(PREFIX, identifier_minimum_elements)(uint64_t id);       \
  size_t PTREE_CONCAT(PREFIX, identifier_maximum_elements)(uint64_t id);       \
  uint64_t PTREE_CONCAT(PREFIX, identifier)(ptree p);                          \
  bool PTREE_CONCAT(PREFIX, is_token)(ptree p);                                \
  bool PTREE_CONCAT(PREFIX, is_expression)(ptree p);                           \
  size_t PTREE_CONCAT(PREFIX, minimum_elements)(ptree p);                      \
  size_t PTREE_CONCAT(PREFIX, maximum_elements)(ptree p);                      \
  const char *PTREE_CONCAT(PREFIX, token_value)(ptree p);                      \
  size_t PTREE_CONCAT(PREFIX, token_width)(ptree p);                           \
  ptree PTREE_CONCAT(PREFIX, from_token)(ptree_context ctx, uint64_t id,       \
                                         const char *value, size_t width);     \
  size_t PTREE_CONCAT(PREFIX, expression_elements)(ptree p);                   \
  ptree PTREE_CONCAT(PREFIX, expression_element)(ptree p, size_t index);       \
  ptree PTREE_CONCAT(PREFIX, expression_append)(ptree_context ctx, ptree base, \
                                                ptree element);                \
  ptree PTREE_CONCAT(PREFIX, expression_construct)(                            \
      ptree_context ctx, uint64_t id, size_t N, ptree * elts);               \
  ptree PTREE_CONCAT(PREFIX, expression0)(ptree_context ctx, uint64_t id);     \
  ptree PTREE_CONCAT(PREFIX, expression1)(ptree_context ctx, uint64_t id,      \
                                          ptree x0);                           \
  ptree PTREE_CONCAT(PREFIX, expression2)(ptree_context ctx, uint64_t id,      \
                                          ptree x0, ptree x1);                 \
  ptree PTREE_CONCAT(PREFIX, expression3)(ptree_context ctx, uint64_t id,      \
                                          ptree x0, ptree x1, ptree x2);       \
  ptree PTREE_CONCAT(PREFIX, expression4)(                                     \
      ptree_context ctx, uint64_t id, ptree x0, ptree x1, ptree x2, ptree x3); \
  ptree PTREE_CONCAT(PREFIX, expression5)(ptree_context ctx, uint64_t id,      \
                                          ptree x0, ptree x1, ptree x2,        \
                                          ptree x3, ptree x4);                 \
  ptree PTREE_CONCAT(PREFIX, expression6)(ptree_context ctx, uint64_t id,      \
                                          ptree x0, ptree x1, ptree x2,        \
                                          ptree x3, ptree x4, ptree x5);       \
  ptree PTREE_CONCAT(PREFIX, expression7)(                                     \
      ptree_context ctx, uint64_t id, ptree x0, ptree x1, ptree x2, ptree x3,  \
      ptree x4, ptree x5, ptree x6);                                           \
  ptree PTREE_CONCAT(PREFIX, expression8)(                                     \
      ptree_context ctx, uint64_t id, ptree x0, ptree x1, ptree x2, ptree x3,  \
      ptree x4, ptree x5, ptree x6, ptree x7);

struct ptree_module_ty;
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

  uint64_t (*const identifier)(ptree);

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
typedef struct ptree_module_ty ptree_module;

#define ptree_require(X) ptree_require_func(X, #X, __LINE__)
static inline void ptree_require_func(bool expr, const char *name, int line) {
  const bool contract = PTREE_CONTRACT();
  if (contract & !expr) {
#if PTREE_CONTRACT()
    fprintf(stderr, "Contract failed L%u: %s\n", line, name);
    abort();
#else
    (void)name;
    (void)line;
#endif
  }
}

static inline ptree_context ptree_create_context(const ptree_module *mod) {
  return mod->create_context();
}
static inline void ptree_destroy_context(const ptree_module *mod,
                                         ptree_context ctx) {
  mod->destroy_context(ctx);
}

static inline bool ptree_identifier_valid_token(const ptree_module *mod,
                                             uint64_t id) {
  return mod->identifier_valid_token(id);
}

static inline bool ptree_identifier_valid_expression(const ptree_module *mod,
                                                  uint64_t id) {
  return mod->identifier_valid_expression(id);
}


static inline size_t ptree_identifier_minimum_elements(const ptree_module *mod,
                                                       uint64_t id) {
  return mod->identifier_minimum_elements(id);
}

static inline size_t ptree_identifier_maximum_elements(const ptree_module *mod,
                                                       uint64_t id) {
  return mod->identifier_maximum_elements(id);
}

static inline ptree ptree_failure(void)
{
  return (ptree){.state = 0,};
}

static inline bool ptree_is_failure(ptree p) {
  return p.state == 0;
}

static inline uint64_t ptree_identifier(const ptree_module *mod, ptree p) {
  return mod->identifier(p);
}

static inline bool ptree_is_token(const ptree_module *mod, ptree p) {
  return mod->is_token(p);
}

static inline bool ptree_is_expression(const ptree_module *mod, ptree p) {
  return mod->is_expression(p);
}

static inline size_t ptree_minimum_elements(const ptree_module *mod, ptree p) {
  return mod->identifier_minimum_elements(ptree_identifier(mod, p));
}

static inline size_t ptree_maximum_elements(const ptree_module *mod, ptree p) {
  return mod->identifier_maximum_elements(ptree_identifier(mod, p));
}

static inline const char *ptree_token_value(const ptree_module *mod, ptree p) {
  ptree_require(ptree_is_token(mod, p));
  return mod->token_value(p);
}

static inline size_t ptree_token_width(const ptree_module *mod, ptree p) {
  ptree_require(ptree_is_token(mod, p));
  return mod->token_width(p);
}

static inline ptree ptree_from_token(const ptree_module *mod, ptree_context ctx,
                                     uint64_t id, const char *value,
                                     size_t width) {
  ptree_require(ptree_identifier_valid_token(mod, id));

  ptree res = mod->from_token(ctx, id, value, width);

  ptree_require(width == ptree_token_width(mod, res));

  // the _same_ pointer, not merely the same contents
  ptree_require(value == ptree_token_value(mod, res));

  return res;
}

static inline size_t ptree_expression_elements(const ptree_module *mod,
                                               ptree p) {
  ptree_require(ptree_is_expression(mod, p));
  size_t res = mod->expression_elements(p);
  ptree_require(res >= ptree_minimum_elements(mod, p));
  ptree_require(res <= ptree_maximum_elements(mod, p));
  return res;
}

static inline ptree ptree_expression_element(const ptree_module *mod, ptree p,
                                             size_t index) {
  ptree_require(ptree_is_expression(mod, p));
  ptree_require(index < ptree_expression_elements(mod, p));
  ptree res = mod->expression_element(p, index);
  ptree_require(!ptree_is_failure(res));
  return res;
}

static inline bool
ptree_expression_failure_or_has_N_elements(const ptree_module *mod, ptree res,
                                           size_t N) {
  return ptree_is_failure( res) ||
         (ptree_is_expression(mod, res) &&
          (ptree_expression_elements(mod, res) == N));
}

static inline bool
ptree_identifier_number_elements_within_bounds(const ptree_module *mod,
                                               uint64_t id, size_t N) {
  return (ptree_identifier_minimum_elements(mod, id) <= N) &&
         (ptree_identifier_maximum_elements(mod, id) >= N);
}

static inline ptree ptree_expression_append(const ptree_module *mod,
                                            ptree_context ctx, ptree base,
                                            ptree element) {
  ptree_require(!ptree_is_failure(base));
  ptree_require(!ptree_is_failure(element));
  ptree_require(ptree_is_expression(mod, base));
  ptree_require(ptree_is_token(mod, element) ||
                ptree_is_expression(mod, element));

#if PTREE_CONTRACT()
  size_t elements_before = ptree_expression_elements(mod, base);
  ptree_require(elements_before + 1 <= ptree_maximum_elements(mod, base));
#endif

  ptree res = mod->expression_append(ctx, base, element);

#if PTREE_CONTRACT()
  ptree_require(ptree_expression_failure_or_has_N_elements(
      mod, res, elements_before + 1));
#endif
  return res;
}

static inline ptree ptree_expression_construct(const ptree_module *mod,
                                               ptree_context ctx, uint64_t id,
                                               size_t N, ptree *elts) {
  ptree_require(ptree_identifier_number_elements_within_bounds(mod, id, N));
  for (size_t i = 0; i < N; i++) {
    ptree_require(!ptree_is_failure(elts[i]));
  }
  ptree res = mod->expression_construct(ctx, id, N, elts);
  ptree_require(ptree_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression0(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id) {
  enum { N = 0 };
  ptree_require(ptree_identifier_number_elements_within_bounds(mod, id, N));
  ptree res = mod->expression_construct(ctx, id, 0, 0);
  ptree_require(ptree_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression1(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id,
                                      ptree x0) {
  enum { N = 1 };
  ptree_require(ptree_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression2(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1) {
  enum { N = 2 };
  ptree_require(ptree_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression3(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2) {
  enum { N = 3 };
  ptree_require(ptree_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression4(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3) {
  enum { N = 4 };
  ptree_require(ptree_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2, x3};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression5(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4) {
  enum { N = 5 };
  ptree_require(ptree_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2, x3, x4};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression6(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4,
                                      ptree x5) {
  enum { N = 6 };
  ptree_require(ptree_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2, x3, x4, x5};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression7(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4,
                                      ptree x5, ptree x6) {
  enum { N = 7 };
  ptree_require(ptree_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2, x3, x4, x5, x6};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression8(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4,
                                      ptree x5, ptree x6, ptree x7) {
  enum { N = 8 };
  ptree_require(ptree_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2, x3, x4, x5, x6, x7};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

// If there are local functions that happen to have the form
// PREFIX_ptree_create_context then this macro will initialise a module instance
// from them
#define PTREE_INSTANTIATE_MODULE_INITIALIZER(PREFIX)                           \
  {                                                                            \
    .create_context = PTREE_CONCAT(PREFIX, create_context),                    \
    .destroy_context = PTREE_CONCAT(PREFIX, destroy_context),                  \
      .identifier_valid_token = PTREE_CONCAT(PREFIX, identifier_valid_token), \
    .identifier_valid_expression =                                                \
        PTREE_CONCAT(PREFIX, identifier_valid_expression),                        \
    .identifier_minimum_elements =                                             \
        PTREE_CONCAT(PREFIX, identifier_minimum_elements),                     \
    .identifier_maximum_elements =                                             \
        PTREE_CONCAT(PREFIX, identifier_maximum_elements),                     \
    .identifier = PTREE_CONCAT(PREFIX, identifier),                            \
    .is_token = PTREE_CONCAT(PREFIX, is_token),                          \
    .is_expression = PTREE_CONCAT(PREFIX, is_expression),                          \
    .token_value = PTREE_CONCAT(PREFIX, token_value),                          \
    .token_width = PTREE_CONCAT(PREFIX, token_width),                          \
    .from_token = PTREE_CONCAT(PREFIX, from_token),                            \
    .expression_elements = PTREE_CONCAT(PREFIX, expression_elements),          \
    .expression_element = PTREE_CONCAT(PREFIX, expression_element),            \
    .expression_append = PTREE_CONCAT(PREFIX, expression_append),              \
    .expression_construct = PTREE_CONCAT(PREFIX, expression_construct),        \
  }

#define PTREE_INSTANTIATE_DEFINE(PREFIX, MODULE)                               \
  ptree_context PTREE_CONCAT(PREFIX, create_context)(void) {                   \
    return ptree_create_context(&MODULE);                                      \
  }                                                                            \
  void PTREE_CONCAT(PREFIX, destroy_context)(ptree_context ctx) {              \
    return ptree_destroy_context(&MODULE, ctx);                                \
  }                                                                            \
  const ptree_module * PTREE_CONCAT(PREFIX, retrieve_module)(void) {           \
    return &MODULE;                                                            \
  }                                                                            \
  bool PTREE_CONCAT(PREFIX, identifier_valid_token)(uint64_t id) {                \
    return ptree_identifier_valid_token(&MODULE, id);                             \
  }                                                                            \
  bool PTREE_CONCAT(PREFIX, identifier_valid_expression)(uint64_t id) {           \
    return ptree_identifier_valid_expression(&MODULE, id);                        \
  }                                                                            \
  size_t PTREE_CONCAT(PREFIX, identifier_minimum_elements)(uint64_t id) {      \
    return ptree_identifier_minimum_elements(&MODULE, id);                     \
  }                                                                            \
  size_t PTREE_CONCAT(PREFIX, identifier_maximum_elements)(uint64_t id) {      \
    return ptree_identifier_maximum_elements(&MODULE, id);                     \
  }                                                                            \
  uint64_t PTREE_CONCAT(PREFIX, identifier)(ptree p) {                         \
    return ptree_identifier(&MODULE, p);                                       \
  }                                                                            \
  bool PTREE_CONCAT(PREFIX, is_token)(ptree p) {                               \
    return ptree_is_token(&MODULE, p);                                         \
  }                                                                            \
  bool PTREE_CONCAT(PREFIX, is_expression)(ptree p) {                          \
    return ptree_is_expression(&MODULE, p);                                    \
  }                                                                            \
  size_t PTREE_CONCAT(PREFIX, minimum_elements)(ptree p) {                     \
    return ptree_minimum_elements(&MODULE, p);                                 \
  }                                                                            \
  size_t PTREE_CONCAT(PREFIX, maximum_elements)(ptree p) {                     \
    return ptree_maximum_elements(&MODULE, p);                                 \
  }                                                                            \
  const char *PTREE_CONCAT(PREFIX, token_value)(ptree p) {                     \
    return ptree_token_value(&MODULE, p);                                      \
  }                                                                            \
  size_t PTREE_CONCAT(PREFIX, token_width)(ptree p) {                          \
    return ptree_token_width(&MODULE, p);                                      \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, from_token)(ptree_context ctx, uint64_t id,       \
                                         const char *value, size_t width) {    \
    return ptree_from_token(&MODULE, ctx, id, value, width);                   \
  }                                                                            \
  size_t PTREE_CONCAT(PREFIX, expression_elements)(ptree p) {                  \
    return ptree_expression_elements(&MODULE, p);                              \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression_element)(ptree p, size_t index) {      \
    return ptree_expression_element(&MODULE, p, index);                        \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression_append)(ptree_context ctx, ptree base, \
                                                ptree element) {               \
    return ptree_expression_append(&MODULE, ctx, base, element);               \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression_construct)(                            \
      ptree_context ctx, uint64_t id, size_t N, ptree * elts) {              \
    return ptree_expression_construct(&MODULE, ctx, id, N, elts);              \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression0)(ptree_context ctx, uint64_t id) {    \
    return ptree_expression0(&MODULE, ctx, id);                                \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression1)(ptree_context ctx, uint64_t id,      \
                                          ptree x0) {                          \
    return ptree_expression1(&MODULE, ctx, id, x0);                            \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression2)(ptree_context ctx, uint64_t id,      \
                                          ptree x0, ptree x1) {                \
    return ptree_expression2(&MODULE, ctx, id, x0, x1);                        \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression3)(ptree_context ctx, uint64_t id,      \
                                          ptree x0, ptree x1, ptree x2) {      \
    return ptree_expression3(&MODULE, ctx, id, x0, x1, x2);                    \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression4)(ptree_context ctx, uint64_t id,      \
                                          ptree x0, ptree x1, ptree x2,        \
                                          ptree x3) {                          \
    return ptree_expression4(&MODULE, ctx, id, x0, x1, x2, x3);                \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression5)(ptree_context ctx, uint64_t id,      \
                                          ptree x0, ptree x1, ptree x2,        \
                                          ptree x3, ptree x4) {                \
    return ptree_expression5(&MODULE, ctx, id, x0, x1, x2, x3, x4);            \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression6)(ptree_context ctx, uint64_t id,      \
                                          ptree x0, ptree x1, ptree x2,        \
                                          ptree x3, ptree x4, ptree x5) {      \
    return ptree_expression6(&MODULE, ctx, id, x0, x1, x2, x3, x4, x5);        \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression7)(                                     \
      ptree_context ctx, uint64_t id, ptree x0, ptree x1, ptree x2, ptree x3,  \
      ptree x4, ptree x5, ptree x6) {                                          \
    return ptree_expression7(&MODULE, ctx, id, x0, x1, x2, x3, x4, x5, x6);    \
  }                                                                            \
  ptree PTREE_CONCAT(PREFIX, expression8)(                                     \
      ptree_context ctx, uint64_t id, ptree x0, ptree x1, ptree x2, ptree x3,  \
      ptree x4, ptree x5, ptree x6, ptree x7) {                                \
    return ptree_expression8(&MODULE, ctx, id, x0, x1, x2, x3, x4, x5, x6,     \
                             x7);                                              \
  }

#endif
