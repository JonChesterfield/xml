#ifndef PTREE_MACRO_WRAPPER_H_INCLUDED
#define PTREE_MACRO_WRAPPER_H_INCLUDED

#if 0
// This file should probably be generated.
// Crude code generator to provide lambda dropped / specialised interface
// to ptree.
//
// add a prefix, drop the mod argument, provide an accessor for the module
// which is likely to be a static global in an implementation file.
#endif

#define PTREE_CONCAT2(X, Y) X##_ptree_##Y
#define PTREE_CONCAT(X, Y) PTREE_CONCAT2(X, Y)

#define PTREE_INSTANTIATE_DECLARE(PREFIX)                                      \
  ptree_context PTREE_CONCAT(PREFIX, create_context)(void);                    \
  void PTREE_CONCAT(PREFIX, destroy_context)(ptree_context ctx);               \
  const ptree_module *PTREE_CONCAT(PREFIX, retrieve_module)(void);             \
  bool PTREE_CONCAT(PREFIX, identifier_is_failure)(uint64_t id);               \
  bool PTREE_CONCAT(PREFIX, identifier_valid_token)(uint64_t id);              \
  bool PTREE_CONCAT(PREFIX, identifier_valid_expression)(uint64_t id);         \
  const char *PTREE_CONCAT(PREFIX, identifier_token_maybe_name)(uint64_t id);  \
  const char *PTREE_CONCAT(PREFIX,                                             \
                           identifier_expression_maybe_name)(uint64_t id);     \
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
      ptree_context ctx, uint64_t id, size_t N, ptree * elts);                 \
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
      ptree x4, ptree x5, ptree x6, ptree x7);                                 \
  void PTREE_CONCAT(PREFIX, as_raw_xml)(stack_module stackmod, FILE * file,    \
                                        ptree tree);                           \
  void PTREE_CONCAT(PREFIX, as_xml)(stack_module stackmod, FILE * file,        \
                                    ptree tree);                               \
  enum ptree_compare_res PTREE_CONCAT(PREFIX, compare)(                        \
      stack_module stackmod, ptree left, ptree right);                         \
  int PTREE_CONCAT(PREFIX, traverse)(                                          \
      stack_module stackmod, ptree tree, uint64_t depth,                       \
      int (*pre)(ptree tree, uint64_t, void *), void *pre_data,                \
      int (*elt)(ptree tree, uint64_t, void *), void *elt_data,                \
      int (*post)(ptree tree, uint64_t, void *), void *post_data)

#define PTREE_INSTANTIATE_DEFINE(PREFIX, MODULE)                               \
  PTREE_INSTANTIATE_DECLARE(PREFIX);                                           \
  ptree_context PTREE_CONCAT(PREFIX, create_context)(void) {                   \
    return ptree_create_context(&MODULE);                                      \
  }                                                                            \
  void PTREE_CONCAT(PREFIX, destroy_context)(ptree_context ctx) {              \
    return ptree_destroy_context(&MODULE, ctx);                                \
  }                                                                            \
  const ptree_module *PTREE_CONCAT(PREFIX, retrieve_module)(void) {            \
    return &MODULE;                                                            \
  }                                                                            \
  bool PTREE_CONCAT(PREFIX, identifier_valid_token)(uint64_t id) {             \
    return ptree_identifier_valid_token(&MODULE, id);                          \
  }                                                                            \
  bool PTREE_CONCAT(PREFIX, identifier_valid_expression)(uint64_t id) {        \
    return ptree_identifier_valid_expression(&MODULE, id);                     \
  }                                                                            \
  const char *PTREE_CONCAT(PREFIX, identifier_token_maybe_name)(uint64_t id) { \
    return ptree_identifier_token_maybe_name(&MODULE, id);                     \
  }                                                                            \
  const char *PTREE_CONCAT(PREFIX,                                             \
                           identifier_expression_maybe_name)(uint64_t id) {    \
    return ptree_identifier_expression_maybe_name(&MODULE, id);                \
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
      ptree_context ctx, uint64_t id, size_t N, ptree * elts) {                \
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
  }                                                                            \
  void PTREE_CONCAT(PREFIX, as_raw_xml)(stack_module stackmod, FILE * file,    \
                                        ptree tree) {                          \
    return ptree_as_raw_xml(&MODULE, stackmod, file, tree);                    \
  }                                                                            \
  void PTREE_CONCAT(PREFIX, as_xml)(stack_module stackmod, FILE * file,        \
                                    ptree tree) {                              \
    return ptree_as_xml(&MODULE, stackmod, file, tree);                        \
  }                                                                            \
  enum ptree_compare_res PTREE_CONCAT(PREFIX, compare)(                        \
      stack_module stackmod, ptree left, ptree right) {                        \
    return ptree_compare(&MODULE, stackmod, left, right);                      \
  }                                                                            \
  int PTREE_CONCAT(PREFIX, traverse)(                                          \
      stack_module stackmod, ptree tree, uint64_t depth,                       \
      int (*pre)(ptree tree, uint64_t, void *), void *pre_data,                \
      int (*elt)(ptree tree, uint64_t, void *), void *elt_data,                \
      int (*post)(ptree tree, uint64_t, void *), void *post_data) {            \
    return ptree_traverse_without_mod_callback_parameter(                      \
        &MODULE, stackmod, tree, depth, pre, pre_data, elt, elt_data, post,    \
        post_data);                                                            \
  }

#if 0
// If there are local functions that happen to have the form
// PREFIX_ptree_create_context then this macro will initialise a module instance
// from them
#endif

#define PTREE_INSTANTIATE_MODULE_INITIALIZER(PREFIX)                           \
  {                                                                            \
    .create_context = PTREE_CONCAT(PREFIX, create_context),                    \
    .destroy_context = PTREE_CONCAT(PREFIX, destroy_context),                  \
    .identifier_valid_token = PTREE_CONCAT(PREFIX, identifier_valid_token),    \
    .identifier_valid_expression =                                             \
        PTREE_CONCAT(PREFIX, identifier_valid_expression),                     \
    .identifier_token_maybe_name =                                             \
        PTREE_CONCAT(PREFIX, identifier_token_maybe_name),                     \
    .identifier_expression_maybe_name =                                        \
        PTREE_CONCAT(PREFIX, identifier_expression_maybe_name),                \
    .identifier_minimum_elements =                                             \
        PTREE_CONCAT(PREFIX, identifier_minimum_elements),                     \
    .identifier_maximum_elements =                                             \
        PTREE_CONCAT(PREFIX, identifier_maximum_elements),                     \
    .identifier = PTREE_CONCAT(PREFIX, identifier),                            \
    .is_token = PTREE_CONCAT(PREFIX, is_token),                                \
    .is_expression = PTREE_CONCAT(PREFIX, is_expression),                      \
    .token_value = PTREE_CONCAT(PREFIX, token_value),                          \
    .token_width = PTREE_CONCAT(PREFIX, token_width),                          \
    .from_token = PTREE_CONCAT(PREFIX, from_token),                            \
    .expression_elements = PTREE_CONCAT(PREFIX, expression_elements),          \
    .expression_element = PTREE_CONCAT(PREFIX, expression_element),            \
    .expression_append = PTREE_CONCAT(PREFIX, expression_append),              \
    .expression_construct = PTREE_CONCAT(PREFIX, expression_construct),        \
  }

#endif
