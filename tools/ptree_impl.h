#ifndef PTREE_IMPL_H_INCLUDED
#define PTREE_IMPL_H_INCLUDED

#ifndef PTREE_H_INCLUDED
#error "Expected to be included from ptree.h"
#endif

#ifndef PTREE_CONTRACT
#error "Require PTREE_CONTRACT definition"
#endif

#define ptree_require(X) ptree_require_func(X, #X, __LINE__)
static inline void ptree_require_func(bool expr, const char *name, int line) {
  const bool contract = PTREE_CONTRACT();
  if (contract & !expr) {
#if PTREE_CONTRACT()
    fprintf(stderr, "ptree contract failed L%u: %s\n", line, name);
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

static inline const char *
ptree_identifier_token_maybe_name(const ptree_module *mod, uint64_t id) {
  return mod->identifier_token_maybe_name(id);
}

static inline const char *
ptree_identifier_expression_maybe_name(const ptree_module *mod, uint64_t id) {
  return mod->identifier_expression_maybe_name(id);
}

static inline size_t ptree_identifier_minimum_elements(const ptree_module *mod,
                                                       uint64_t id) {
  return mod->identifier_minimum_elements(id);
}

static inline size_t ptree_identifier_maximum_elements(const ptree_module *mod,
                                                       uint64_t id) {
  return mod->identifier_maximum_elements(id);
}

static inline ptree ptree_failure(void) {
  return (ptree){
      .state = 0,
  };
}

static inline bool ptree_is_failure(ptree p) { return p.state == 0; }

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
ptree_impl_expression_failure_or_has_N_elements(const ptree_module *mod,
                                                ptree res, size_t N) {
  return ptree_is_failure(res) || (ptree_is_expression(mod, res) &&
                                   (ptree_expression_elements(mod, res) == N));
}

static inline bool
ptree_impl_identifier_number_elements_within_bounds(const ptree_module *mod,
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
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(
      mod, res, elements_before + 1));
#endif
  return res;
}

static inline ptree ptree_expression_construct(const ptree_module *mod,
                                               ptree_context ctx, uint64_t id,
                                               size_t N, ptree *elts) {
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));
  for (size_t i = 0; i < N; i++) {
    ptree_require(!ptree_is_failure(elts[i]));
  }
  ptree res = mod->expression_construct(ctx, id, N, elts);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression0(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id) {
  enum { N = 0 };
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));
  ptree res = mod->expression_construct(ctx, id, 0, 0);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression1(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id,
                                      ptree x0) {
  enum { N = 1 };
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression2(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1) {
  enum { N = 2 };
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression3(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2) {
  enum { N = 3 };
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression4(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3) {
  enum { N = 4 };
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2, x3};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression5(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4) {
  enum { N = 5 };
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2, x3, x4};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression6(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4,
                                      ptree x5) {
  enum { N = 6 };
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2, x3, x4, x5};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression7(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4,
                                      ptree x5, ptree x6) {
  enum { N = 7 };
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2, x3, x4, x5, x6};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline ptree ptree_expression8(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id, ptree x0,
                                      ptree x1, ptree x2, ptree x3, ptree x4,
                                      ptree x5, ptree x6, ptree x7) {
  enum { N = 8 };
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));
  ptree arr[N] = {x0, x1, x2, x3, x4, x5, x6, x7};
  ptree res = mod->expression_construct(ctx, id, N, arr);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

static inline int ptree_impl_as_xml_pre(const ptree_module *mod,
                                        ptree tree,
                                        uint64_t depth,
                                        void *voidfile)
{
  FILE *file = (FILE*)voidfile;
  if (ptree_is_expression(mod, tree)) {
    uint64_t id = ptree_identifier(mod, tree);
    const char *name = ptree_identifier_expression_maybe_name(mod, id);

    fprintf(file, "%*s", (int)depth, "");
    if (name) {
      fprintf(file, "<%s>\n", name);
    } else {
      fprintf(file, "<e%zu>\n", id);
    }
  }
  return 0;
}

static inline int ptree_impl_as_xml_elt(const ptree_module *mod,
                                        ptree tree,
                                        uint64_t depth,
                                        void *voidfile) {
  FILE *file = (FILE*)voidfile;
  if (ptree_is_token(mod, tree)) {
    uint64_t id = ptree_identifier(mod, tree);
    const char *name = ptree_identifier_token_maybe_name(mod, id);

    const char *value = ptree_token_value(mod, tree);
    size_t width = ptree_token_width(mod, tree);
    
    fprintf(file, "%*s", (int)depth, "");
    if (name) {
      fprintf(file, "<%s", name);
    } else {
      fprintf(file, "<T%zu", id);
    }
    fprintf(file, " \"value\"=\"");
    // todo - buffer / %s, deal with non-ascii  etc
    for (size_t i = 0; i < width; i++) {
      fprintf(file, "%c", value[i]);
    }
    fprintf(file, "\" />\n");
  }

  return 0;
}

static inline int ptree_impl_as_xml_post(const ptree_module *mod,
                                        ptree tree,
                                         uint64_t depth,
                                        void *voidfile)
{
  FILE *file = (FILE*)voidfile;
  if (ptree_is_expression(mod, tree)) {
    uint64_t id = ptree_identifier(mod, tree);
    const char *name = ptree_identifier_expression_maybe_name(mod, id);

    fprintf(file, "%*s", (int)depth, "");
    if (name) {
      fprintf(file, "</%s>\n", name);
    } else {
      fprintf(file, "</e%zu>\n", id);
    }
  }
  return 0;
}

static inline void ptree_as_xml(const ptree_module *mod, FILE *file,
                                ptree tree) {
  if (ptree_is_failure(tree)) {
    fprintf(file, "<Failure/>");
    return;
  }

  ptree_traverse(mod,
                 tree,
                 0u,
                 ptree_impl_as_xml_pre,
                 file,
                 ptree_impl_as_xml_elt,
                 file,
                 ptree_impl_as_xml_post,
                 file);
}

#endif
