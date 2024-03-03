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
  ptree res = ptree_expression_create_uninitialised(mod, ctx, id, N);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));

  if (ptree_is_failure(res)) {return ptree_failure(); }
  
  for (size_t i = 0; i < N; i++) {
    ptree_expression_initialise_element(mod, res, i, elts[i]);
  }
  
  return res;
}

static inline ptree ptree_expression_create_uninitialised(const ptree_module *mod,
                                                          ptree_context ctx, uint64_t id,
                                                          size_t N)
{
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));

  ptree res = mod->expression_create_uninitialised(ctx, id, N);

  if (!ptree_is_failure(res)) {
    ptree_require(ptree_is_expression(mod, res));
    ptree_require(ptree_expression_elements(mod, res) == N);
    for (size_t i = 0; i < N; i++)
      {
        ptree_require(ptree_is_failure(mod->expression_element(res, i)));
      }
  }
  
  return res;
}

static inline void ptree_expression_initialise_element(const ptree_module *mod, ptree base,
                                                       size_t index, ptree elt)
{
  ptree_require(ptree_is_expression(mod, base));
  ptree_require(index < ptree_expression_elements(mod, base));
  ptree_require(!ptree_is_failure(elt));
  
  ptree_require(ptree_is_failure(mod->expression_element(base, index)));
  mod->expression_initialise_element(base, index, elt);
  ptree_require(!ptree_is_failure(mod->expression_element(base, index)));
  ptree_require(mod->expression_element(base, index).state == elt.state);
}


static inline ptree ptree_expression0(const ptree_module *mod,
                                      ptree_context ctx, uint64_t id) {
  enum { N = 0 };
  ptree_require(
      ptree_impl_identifier_number_elements_within_bounds(mod, id, N));
  ptree res = ptree_expression_construct(mod, ctx, id, 0, 0);
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
  ptree res = ptree_expression_construct(mod, ctx, id, N, arr);
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
  ptree res = ptree_expression_construct(mod, ctx, id, N, arr);
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
  ptree res = ptree_expression_construct(mod, ctx, id, N, arr);
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
  ptree res = ptree_expression_construct(mod, ctx, id, N, arr);
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
  ptree res = ptree_expression_construct(mod, ctx, id, N, arr);
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
  ptree res = ptree_expression_construct(mod, ctx, id, N, arr);
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
  ptree res = ptree_expression_construct(mod, ctx, id, N, arr);
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
  ptree res = ptree_expression_construct(mod, ctx, id, N, arr);
  ptree_require(ptree_impl_expression_failure_or_has_N_elements(mod, res, N));
  return res;
}

struct ptree_traverse_without_mod_callback_wrap_ty {
  int (*func)(ptree tree, uint64_t, void *);
  void *data;
};

static inline int ptree_traverse_without_mod_callback_wrap_func(
    const ptree_module *mod, ptree tree, uint64_t depth, void *p) {
  struct ptree_traverse_without_mod_callback_wrap_ty *arg =
      (struct ptree_traverse_without_mod_callback_wrap_ty *)p;
  return arg->func(tree, depth, arg->data);
}

static inline int ptree_traverse_without_mod_callback_parameter(
    const ptree_module *mod, stack_module stackmod, ptree tree, uint64_t depth,
    int (*pre)(ptree tree, uint64_t, void *), void *pre_data,
    int (*elt)(ptree tree, uint64_t, void *), void *elt_data,
    int (*post)(ptree tree, uint64_t, void *), void *post_data) {

  struct ptree_traverse_without_mod_callback_wrap_ty pre_w = {
      .func = pre,
      .data = pre_data,
  };
  struct ptree_traverse_without_mod_callback_wrap_ty elt_w = {
      .func = elt,
      .data = elt_data,
  };
  struct ptree_traverse_without_mod_callback_wrap_ty post_w = {
      .func = post,
      .data = post_data,
  };

  return ptree_traverse(mod, stackmod, tree, depth,
                        ptree_traverse_without_mod_callback_wrap_func, &pre_w,
                        ptree_traverse_without_mod_callback_wrap_func, &elt_w,
                        ptree_traverse_without_mod_callback_wrap_func, &post_w);
}

static inline int ptree_impl_as_xml_pre_impl(const ptree_module *mod,
                                             ptree tree, uint64_t depth,
                                             bool pretty, void *voidfile) {
  FILE *file = (FILE *)voidfile;
  if (ptree_is_expression(mod, tree)) {
    uint64_t id = ptree_identifier(mod, tree);
    const char *name =
        pretty ? ptree_identifier_expression_maybe_name(mod, id) : 0;
    size_t width = ptree_expression_elements(mod, tree);

    fprintf(file, "%*s", (int)depth, "");

    const char *close = width == 0 ? "/" : "";

    if (name) {
      fprintf(file, "<%s%s>\n", name, close);
    } else {
      fprintf(file, "<e%zu%s>\n", id, close);
    }
  }
  return 0;
}

static inline int ptree_impl_as_xml_elt_impl(const ptree_module *mod,
                                             ptree tree, uint64_t depth,
                                             bool pretty, void *voidfile) {
  FILE *file = (FILE *)voidfile;
  if (ptree_is_token(mod, tree)) {
    uint64_t id = ptree_identifier(mod, tree);
    const char *name = pretty ? ptree_identifier_token_maybe_name(mod, id) : 0;

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

static inline int ptree_impl_as_xml_post_impl(const ptree_module *mod,
                                              ptree tree, uint64_t depth,
                                              bool pretty, void *voidfile) {
  FILE *file = (FILE *)voidfile;
  if (ptree_is_expression(mod, tree)) {
    uint64_t id = ptree_identifier(mod, tree);
    const char *name =
        pretty ? ptree_identifier_expression_maybe_name(mod, id) : 0;
    size_t width = ptree_expression_elements(mod, tree);

    if (width != 0) {
      fprintf(file, "%*s", (int)depth, "");
      if (name) {
        fprintf(file, "</%s>\n", name);
      } else {
        fprintf(file, "</e%zu>\n", id);
      }
    }
  }
  return 0;
}

static inline int ptree_impl_as_xml_pre(const ptree_module *mod, ptree tree,
                                        uint64_t depth, void *voidfile) {
  return ptree_impl_as_xml_pre_impl(mod, tree, depth, true, voidfile);
}
static inline int ptree_impl_as_xml_elt(const ptree_module *mod, ptree tree,
                                        uint64_t depth, void *voidfile) {
  return ptree_impl_as_xml_elt_impl(mod, tree, depth, true, voidfile);
}
static inline int ptree_impl_as_xml_post(const ptree_module *mod, ptree tree,
                                         uint64_t depth, void *voidfile) {
  return ptree_impl_as_xml_post_impl(mod, tree, depth, true, voidfile);
}

static inline int ptree_impl_as_raw_xml_pre(const ptree_module *mod, ptree tree,
                                            uint64_t depth, void *voidfile) {
  return ptree_impl_as_xml_pre_impl(mod, tree, depth, false, voidfile);
}
static inline int ptree_impl_as_raw_xml_elt(const ptree_module *mod, ptree tree,
                                            uint64_t depth, void *voidfile) {
  return ptree_impl_as_xml_elt_impl(mod, tree, depth, false, voidfile);
}
static inline int ptree_impl_as_raw_xml_post(const ptree_module *mod,
                                             ptree tree, uint64_t depth,
                                             void *voidfile) {
  return ptree_impl_as_xml_post_impl(mod, tree, depth, false, voidfile);
}

static inline void ptree_as_xml(const ptree_module *mod, stack_module stackmod,
                                FILE *file, ptree tree) {
  if (ptree_is_failure(tree)) {
    fprintf(file, "<Failure/>");
    return;
  }

  ptree_traverse(mod, stackmod, tree, 0u, ptree_impl_as_xml_pre, file,
                 ptree_impl_as_xml_elt, file, ptree_impl_as_xml_post, file);
}

static inline void ptree_as_raw_xml(const ptree_module *mod,
                                    stack_module stackmod, FILE *file,
                                    ptree tree) {
  if (ptree_is_failure(tree)) {
    fprintf(file, "<Failure/>");
    return;
  }

  ptree_traverse(mod, stackmod, tree, 0u, ptree_impl_as_raw_xml_pre, file,
                 ptree_impl_as_raw_xml_elt, file, ptree_impl_as_raw_xml_post,
                 file);
}

static inline int ptree_traverse(
    const ptree_module *mod, stack_module stackmod, ptree tree, uint64_t depth,
    int (*pre)(const ptree_module *mod, ptree tree, uint64_t, void *),
    void *pre_data,
    int (*elt)(const ptree_module *mod, ptree tree, uint64_t, void *),
    void *elt_data,
    int (*post)(const ptree_module *mod, ptree tree, uint64_t, void *),
    void *post_data) {
  ptree_require(!ptree_is_failure(tree));

  if (ptree_is_token(mod, tree)) {
    return elt(mod, tree, depth, elt_data);
  }

  void *stack = stack_create(stackmod, 4);
  if (!stack) {
    return 1;
  }
  stack_push_assuming_capacity(stackmod, stack, tree.state);

  for (uint64_t size = 1; size = stack_size(stackmod, stack), size != 0;) {

    uint64_t top = stack_pop(stackmod, stack);
    size = size - 1; // keep the local variable accurate

    // Clear the low bit when recreating the tree instance
    ptree t = {.state = top & ~UINT64_C(1)};

    bool low_set = top & 1;

    if (low_set) {
      ptree_require(ptree_is_expression(mod, t));
      depth--;
      int a = post(mod, t, depth, post_data);
      if (a != 0) {
        stack_destroy(stackmod, stack);
        return a;
      }
      continue;
    }

    if (ptree_is_token(mod, t)) {
      int e = elt(mod, t, depth, elt_data);
      if (e != 0) {
        stack_destroy(stackmod, stack);
        return e;
      }
      continue;
    }

    if (ptree_is_expression(mod, t)) {
      size_t N = ptree_expression_elements(mod, t);
      {
        void *s2 = stack_reserve(stackmod, stack, size + N + 1);
        if (!s2) {
          stack_destroy(stackmod, stack);
          return 1;
        }
        stack = s2;
      }

      // Current with the bit set
      stack_push_assuming_capacity(stackmod, stack, t.state | (UINT64_C(1)));

      // Then all the elements
      for (size_t i = N; i-- > 0;) {
        ptree p = ptree_expression_element(mod, t, i);
        stack_push_assuming_capacity(stackmod, stack, p.state);
      }

      int b = pre(mod, t, depth, pre_data);
      if (b != 0) {
        stack_destroy(stackmod, stack);
        return b;
      }

      depth++;
    }
  }

  stack_destroy(stackmod, stack);
  return 0;
}

static inline enum ptree_compare_res ptree_compare(const ptree_module *mod,
                                                   stack_module stackmod,
                                                   ptree left, ptree right) {
  ptree_require(!ptree_is_failure(left));
  ptree_require(!ptree_is_failure(right));

  void *stack = stack_create(stackmod, 4);
  if (!stack) {
    return ptree_compare_out_of_memory;
  }

  stack_push_assuming_capacity(stackmod, stack, right.state);
  stack_push_assuming_capacity(stackmod, stack, left.state);

  for (uint64_t size = 2; size = stack_size(stackmod, stack), size != 0;) {
    ptree left = {.state = stack_pop(stackmod, stack)};
    ptree right = {.state = stack_pop(stackmod, stack)};
    size = size - 2;

    if (left.state == right.state) {
      continue;
    }

    ptree_require(!ptree_is_failure(left));
    ptree_require(!ptree_is_failure(right));

    uint64_t left_id = ptree_identifier(mod, left);
    uint64_t right_id = ptree_identifier(mod, right);

    if (left_id < right_id) {
      stack_destroy(stackmod, stack);
      return ptree_compare_lesser;
    }

    if (left_id > right_id) {
      stack_destroy(stackmod, stack);
      return ptree_compare_greater;
    }

    ptree_require(left_id == right_id);

    if (ptree_is_token(mod, left)) {
      ptree_require(ptree_is_token(mod, right));
      uint64_t left_width = ptree_token_width(mod, left);
      uint64_t right_width = ptree_token_width(mod, right);

      if (left_width < right_width) {
        stack_destroy(stackmod, stack);
        return ptree_compare_lesser;
      }
      if (left_width > right_width) {
        stack_destroy(stackmod, stack);
        return ptree_compare_greater;
      }
      ptree_require(left_width == right_width);

      const char *left_value = ptree_token_value(mod, left);
      const char *right_value = ptree_token_value(mod, right);
      for (uint64_t i = 0; i < left_width; i++) {
        // Dealing with signed compares the brute force way
        int l = (int)left_value[i] + 256;
        int r = (int)right_value[i] + 256;

        if (l < r) {
          stack_destroy(stackmod, stack);
          return ptree_compare_lesser;
        }
        if (r < l) {
          stack_destroy(stackmod, stack);
          return ptree_compare_greater;
        }
      }
      // Tokens were equal id, width, value
    } else {
      ptree_require(ptree_is_expression(mod, left));
      ptree_require(ptree_is_expression(mod, right));

      uint64_t left_width = ptree_expression_elements(mod, left);
      uint64_t right_width = ptree_expression_elements(mod, right);

      if (left_width < right_width) {
        stack_destroy(stackmod, stack);
        return ptree_compare_lesser;
      }
      if (left_width > right_width) {
        stack_destroy(stackmod, stack);
        return ptree_compare_greater;
      }
      ptree_require(left_width == right_width);

      {
        void *reserved_stack = stack_reserve(
            stackmod, stack, stack_size(stackmod, stack) + 2 * left_width);
        if (!reserved_stack) {
          stack_destroy(stackmod, stack);
          return ptree_compare_out_of_memory;
        }
        stack = reserved_stack;
      }

      for (uint64_t i = left_width; i-- > 0;) {
        ptree relt = ptree_expression_element(mod, right, i);
        ptree lelt = ptree_expression_element(mod, left, i);
        stack_push_assuming_capacity(stackmod, stack, relt.state);
        stack_push_assuming_capacity(stackmod, stack, lelt.state);
      }
    }
  }
  stack_destroy(stackmod, stack);
  return ptree_compare_equal;
}

#endif
