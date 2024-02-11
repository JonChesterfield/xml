#include "generic_ptree.h"
#include "ptree_malloc_allocator.h"

#include <assert.h>

static ptree_context generic_impl_ptree_create_context(void) {
  return ptree_malloc_ptree_create_context();
}

static void generic_impl_ptree_destroy_context(ptree_context ctx) {
  ptree_malloc_ptree_destroy_context(ctx);
}

static bool generic_impl_ptree_identifier_valid_token(uint64_t id) {
  (void)id;
  return true;
}

static bool generic_impl_ptree_identifier_valid_expression(uint64_t id) {
  (void)id;
  return true;
}

static const char * generic_impl_ptree_identifier_token_maybe_name(uint64_t id) {
  (void)id;
  return 0;
}

static const char * generic_impl_ptree_identifier_expression_maybe_name(uint64_t id) {
  (void)id;
  return 0;
}

static size_t generic_impl_ptree_identifier_minimum_elements(uint64_t id) {
  (void)id;
  return 0;
}
static size_t generic_impl_ptree_identifier_maximum_elements(uint64_t id) {
  (void)id;
  return SIZE_MAX;
}

static uint64_t generic_impl_ptree_identifier(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->id;
}

static bool generic_impl_ptree_is_token(ptree arg) {
  if (ptree_is_failure(arg)) {
    return false;
  }
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->is_token;
}

static bool generic_impl_ptree_is_expression(ptree arg) {
  if (ptree_is_failure(arg)) {
    return false;
  }
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return !p->is_token;
}

static const char *generic_impl_ptree_token_value(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->token_value;
}
static size_t generic_impl_ptree_token_width(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->token_width;
}
static ptree generic_impl_ptree_from_token(ptree_context ctx, uint64_t id,
                                           const char *value, size_t width) {
  ptree_malloc_ptree_context s =
      ptree_context_to_ptree_malloc_ptree_context(ctx);
  return ptree_malloc_ptree_to_ptree(
      make_ptree_malloc_ptree_from_token(s, id, value, width));
}
static size_t generic_impl_ptree_expression_elements(ptree arg) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->Nelements;
}

static ptree generic_impl_ptree_expression_element(ptree arg, size_t index) {
  ptree_malloc_ptree p = ptree_to_ptree_malloc_ptree(arg);
  return p->elements[index];
}

static inline ptree generic_impl_ptree_expression_construct(ptree_context ctx,
                                                            uint64_t id,
                                                            uint64_t N,
                                                            ptree *elts) {
  ptree_malloc_ptree_context s =
      ptree_context_to_ptree_malloc_ptree_context(ctx);
  ptree_malloc_ptree r = make_ptree_malloc_ptree_from_N_ptree(s, id, N, elts);
  return ptree_malloc_ptree_to_ptree(r);
}

static ptree generic_impl_ptree_expression_append(ptree_context ctx,
                                                  ptree basearg,
                                                  ptree elementarg) {
  ptree_malloc_ptree base = ptree_to_ptree_malloc_ptree(basearg);

  uint64_t before = base->Nelements;
  ptree arr[before + 1];
  for (uint64_t i = 0; i < before; i++) {
    arr[i] = base->elements[i];
  }
  arr[before] = elementarg;

  ptree res =
      generic_impl_ptree_expression_construct(ctx, base->id, before + 1, arr);
  if (ptree_to_ptree_malloc_ptree(res)) {
    for (uint64_t i = 0; i < before; i++) {
      base->elements[i] = ptree_malloc_ptree_to_ptree(0);
    }
    ptree_malloc_ptree_deallocate(base);
  }
  return res;
}

static const ptree_module generic_module =
    PTREE_INSTANTIATE_MODULE_INITIALIZER(generic_impl);

PTREE_INSTANTIATE_DEFINE(generic, generic_module);

ptree generic_ptree_from_other_ptree(ptree_context ctx, const ptree_module *mod,
                                     ptree other) {
  if (ptree_is_failure(other)) {
    return ptree_failure();
  }

  uint64_t id = ptree_identifier(mod, other);

  if (ptree_is_token(mod, other)) {
    return generic_ptree_from_token(ctx, id, ptree_token_value(mod, other),
                                    ptree_token_width(mod, other));
  }

  if (ptree_is_expression(mod, other)) {
    size_t N = ptree_expression_elements(mod, other);

    // need a different interface to avoid this allocation
    // (or to use append repeatedly)
    ptree arr[N];
    for (size_t i = 0; i < N; i++) {
      ptree tmp = ptree_expression_element(mod, other, i);
      arr[i] = generic_ptree_from_other_ptree(ctx, mod, tmp);
    }

    return generic_ptree_expression_construct(ctx, id, N, arr);
  }

  return ptree_failure();
}

void generic_ptree_to_file(FILE *file, ptree tree) {
  // todo: report errors, call fprintf less often
  if (ptree_is_failure(tree)) {
    return;
  }
  uint64_t id = generic_ptree_identifier(tree);

  if (generic_ptree_is_token(tree)) {
    const char *value = generic_ptree_token_value(tree);
    size_t width = generic_ptree_token_width(tree);

    fprintf(file, "(T%zu ", id);
    for (size_t i = 0; i < width; i++) {
      fprintf(file, "%c", value[i]);
    }
    fprintf(file, ")");
    return;
  }

  if (generic_ptree_is_expression(tree)) {
    size_t N = generic_ptree_expression_elements(tree);
    fprintf(file, "(e%zu", id);
    for (size_t i = 0; i < N; i++) {
      ptree p = generic_ptree_expression_element(tree, i);
      fprintf(file, " ");
      generic_ptree_to_file(file, p);
    }
    fprintf(file, ")");
  }
}
