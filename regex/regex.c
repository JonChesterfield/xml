#include "regex.h"
#include "regex.declarations.h"
#include "regex.ptree.h"
#include "regex_string.h"

#include "../tools/arena.h"
#include "../tools/arena.libc.h"
#include "../tools/stack.libc.h"

#include <stdio.h>
#include <assert.h>

bool regex_nullable_p(ptree val) {
 tailcall:;
  
  uint64_t id = regex_ptree_identifier(val);
  switch ((enum regex_grouping)id) {
  #include "regex_grouping_byte_cases.data"
    return false;
    
  case regex_grouping_empty_string:
    return true;
    
  case regex_grouping_empty_set:
    return false;

  case regex_grouping_any_char:
    return false;

  case regex_grouping_kleene:
    return true;

  case regex_grouping_concat:
  case regex_grouping_and: {
    ptree r = regex_ptree_expression_element(val, 0);
    bool nr = regex_nullable_p(r);

    if (!nr) { return false; }
    
    val = regex_ptree_expression_element(val, 1);
    goto tailcall;
  }
  case regex_grouping_or:
    {
    ptree r = regex_ptree_expression_element(val, 0);
    bool nr = regex_nullable_p(r);

    if (nr) {return true; }
    
    val = regex_ptree_expression_element(val, 1);
    goto tailcall;
  }

  case regex_grouping_not: {
    // Could remove this recursion with an extra parameter
    ptree r = regex_ptree_expression_element(val, 0);
    bool nr = regex_nullable_p(r);
    return !nr;
  }

  }

  __builtin_unreachable();
}

// Could be a boolean function instead of returning the regex
ptree regex_nullable(ptree_context ctx, ptree val) {
  // partially tested
  if (regex_nullable_p(val))
    {
      return regex_make_empty_string(ctx);
    }
  else
    {
      return regex_make_empty_set(ctx);
    }
}

ptree regex_derivative(ptree_context ctx, ptree val, uint8_t byte) {
  uint64_t id = regex_ptree_identifier(val);

  switch ((enum regex_grouping)id) {
  #include "regex_grouping_byte_cases.data"
    return (byte == regex_grouping_extract_single_byte(id))
               ? regex_make_empty_string(ctx)
               : regex_make_empty_set(ctx);
    
  case regex_grouping_empty_string:
  case regex_grouping_empty_set:
    return regex_make_empty_set(ctx);

  case regex_grouping_any_char:
    return regex_make_empty_string(ctx);

  case regex_grouping_concat: {
    ptree r = regex_ptree_expression_element(val, 0);
    ptree s = regex_ptree_expression_element(val, 1);
    ptree dr = regex_derivative(ctx, r, byte);
    ptree ds = regex_derivative(ctx, s, byte);

    ptree lhs = regex_make_concat(ctx, dr, s);
    // maybe fold nullable on the fly here
    ptree rhs = regex_make_concat(ctx, regex_nullable(ctx, r), ds);
    return regex_make_or(ctx, lhs, rhs);
  }

  case regex_grouping_kleene: {
    ptree r = regex_ptree_expression_element(val, 0);
    ptree dr = regex_derivative(ctx, r, byte);
    return regex_make_concat(ctx, dr, r);
  }

  case regex_grouping_or:
  case regex_grouping_and: {
    ptree r = regex_ptree_expression_element(val, 0);
    ptree s = regex_ptree_expression_element(val, 1);
    ptree dr = regex_derivative(ctx, r, byte);
    ptree ds = regex_derivative(ctx, s, byte);
    return regex_ptree_expression2(ctx, id, dr, ds);
  }

  case regex_grouping_not: {
    ptree r = regex_ptree_expression_element(val, 0);
    ptree dr = regex_derivative(ctx, r, byte);
    return regex_make_not(ctx, dr);
  }
  }

  __builtin_unreachable();
  return ptree_failure();
}

static bool regex_is_not_of_empty_set(ptree val) {
  return regex_is_not(val) &&
         regex_is_empty_set(regex_ptree_expression_element(val, 0));
}

static ptree regex_canonicalise_impl(ptree_context ctx, ptree val) {
  if (!regex_ptree_is_expression(val))
    {
      return val;
    }

  uint64_t id = regex_ptree_identifier(val);

  // Should probably do something with (!11,) and similar here
  if (regex_grouping_id_is_single_byte(id)) {
    return val;
  }
  if (id == regex_grouping_any_char) {
      return val;
  }

  /*
   * Special cases involving the empty set first as they're varied and cheap
   * Considering removing the empty set at parsing time instead.
   */

  if (id == regex_grouping_and || id == regex_grouping_concat ||
      id == regex_grouping_or) {
    ptree r = regex_ptree_expression_element(val, 0);
    ptree s = regex_ptree_expression_element(val, 1);

    r = regex_canonicalise_impl(ctx, r);
    s = regex_canonicalise_impl(ctx, s);

    if (id == regex_grouping_and) {
      if (regex_is_empty_set(r) || regex_is_empty_set(s)) {
        // {} & s -> {}
        return regex_make_empty_set(ctx);
      }

      if (regex_is_not_of_empty_set(r)) {
        // ~{} & s -> s
        return s;
      }

      if (regex_is_not_of_empty_set(s)) {
        // r & ~{} -> r
        return r;
      }
    }

    if (id == regex_grouping_concat) {
      if (regex_is_empty_set(r) || regex_is_empty_set(s)) {
        // {} . s -> {}
        return regex_make_empty_set(ctx);
      }

      if (regex_is_empty_string(r)) {
        // e . s -> s
        return s;
      }

      if (regex_is_empty_string(s)) {
        // r . e -> r
        return r;
      }
    }

    if (id == regex_grouping_or) {
      if (regex_is_empty_set(r)) {
        return s;
      }

      if (regex_is_empty_set(s)) {
        return r;
      }

      if (regex_is_not_of_empty_set(r)) {
        // ~{} + s ->~{}
        return r;
      }

      if (regex_is_not_of_empty_set(s)) {
        // r + ~{} ->~{}
        return s;
      }
    }
  
    // Have canonicalised the arguments, keep that information
    val = regex_ptree_expression2(ctx, id, r, s);
  }

  if (id == regex_grouping_kleene) {
    ptree r = regex_ptree_expression_element(val, 0);
    r = regex_canonicalise_impl(ctx, r);

    // (r*)* -> r*
    if (regex_is_kleene(r)) {
      return r;
    }

    // e* -> e
    if (regex_is_empty_string(r)) {
      return r;
    }

    // {}* -> e
    if (regex_is_empty_set(r)) {
      return regex_make_empty_string(ctx);
    }

    val = regex_make_kleene(ctx, r);
  }

  if (id == regex_grouping_not) {
    // ~(~r) -> r
    ptree r = regex_ptree_expression_element(val, 0);
    r = regex_canonicalise_impl(ctx, r);
    if (regex_is_not(r)) {
      return r;
    }
    val = regex_make_not(ctx, r);
  }

  if (id == regex_grouping_and || id == regex_grouping_or ||
      id == regex_grouping_concat) {
    // (r * s) * t -> r * (s * t)
    ptree r = regex_ptree_expression_element(val, 0);
    ptree s = regex_ptree_expression_element(val, 1);

    // r and s have already been canonicalised
    if ((regex_ptree_identifier(r) == id) &&
        (regex_ptree_identifier(s) != id)) {

      ptree t = s;
      s = regex_ptree_expression_element(r, 1);
      r = regex_ptree_expression_element(r, 0);

      return regex_ptree_expression2(ctx, id, r,
                                     regex_ptree_expression2(ctx, id, s, t));
    }
  }

  if (id == regex_grouping_and || id == regex_grouping_or) {
    // r * s -> s * r based on order, put smaller on the left
    ptree r = regex_ptree_expression_element(val, 0);
    ptree s = regex_ptree_expression_element(val, 1);

    // r and s have already been canonicalised
    enum ptree_compare_res res = regex_ptree_compare(&stack_libc, r, s);
    if (res == ptree_compare_equal) {
      return r;
    }

    if (res == ptree_compare_greater) {
      return regex_ptree_expression2(ctx, id, s, r);
    }

    if (res == ptree_compare_out_of_memory) {
      return ptree_failure();
    }
  }

  return val;
}

ptree regex_canonicalise(ptree_context ctx, ptree val) {
  // This was intended to compute a fixpoint but evidently doesn't
  // Hack around that for now
  // Example that fails is (cat (cat (cat (cat whatever)))),
  // each pass through canonicalise reassociates it by a depth of one
  const unsigned N = 10;
  const bool verbose = false;
  ptree orig = val;
  
  for (unsigned i = 0; i < N; i++)
    {
      ptree canon = regex_canonicalise_impl(ctx, val);

      enum ptree_compare_res cmp = regex_ptree_compare(&stack_libc, val, canon);

      if (cmp == ptree_compare_out_of_memory) {
        return ptree_failure();
      }
      
      if (cmp == ptree_compare_equal)
        {
          if (verbose && (i > 2)) {
            printf("canon took %u\n", i);
            printf("original:\n");
            regex_ptree_as_xml(&stack_libc, stdout, orig);

            printf("canon once:\n");
            ptree canon = regex_canonicalise_impl(ctx, orig);
            regex_ptree_as_xml(&stack_libc, stdout, canon);

            printf("canon twice:\n");
            ptree canon2 = regex_canonicalise_impl(ctx, canon);
            regex_ptree_as_xml(&stack_libc, stdout, canon2);

            printf("canon thrice:\n");
            ptree canon3 = regex_canonicalise_impl(ctx, canon2);
            regex_ptree_as_xml(&stack_libc, stdout, canon3);
          }

          // maybe return val instead? ideally could free a little memory here
          return canon;
        }

  
      val = canon;
    }

  printf("Canonicalise failed to reach a fixpoint after %u iter\n", N);
  exit(1);
}

ptree regex_copy_into_context( ptree val, ptree_context ctx)
{
  if (ptree_is_failure(val)) {return ptree_failure(); }

  static arena_module arena_mod = &arena_libc;

  arena_t a = arena_create(arena_mod, 32);
  int rc = regex_to_char_sequence(arena_mod, &a, val);

  if (rc != 0)
    {
      arena_destroy(arena_mod, a);
      return ptree_failure();
    }

  char * b = arena_base_address(arena_mod, a);
  size_t N = arena_next_offset(arena_mod, a);

  ptree res = regex_from_char_sequence( ctx, b, N);
  arena_destroy(arena_mod, a);

  return res;
}

bool regex_is_canonical(ptree val)
{
  // TODO: Implement directly, not via calling canonicalise and equality
  if (ptree_is_failure(val)) {return false; }

  ptree_context ctx = regex_ptree_create_context();
  assert(regex_ptree_valid_context(ctx));
 
  ptree cp = regex_copy_into_context(val, ctx);
  ptree canon = regex_canonicalise(ctx, cp);  
  enum ptree_compare_res cmp = regex_ptree_compare(&stack_libc, val, canon);

  regex_ptree_destroy_context(ctx);
  return (cmp == ptree_compare_equal);
}

void regex_split(ptree_context ctx, ptree val, ptree edges[static regex_arity]) {
  for (size_t i = 0; i < regex_arity; i++) {
    edges[i] = regex_derivative(ctx, val, (uint8_t)i);
  }
}

int regex_main(void) {
  ptree_context ctx = regex_ptree_create_context();
  assert(regex_ptree_valid_context(ctx));
  ptree expr =
    regex_make_and(ctx,
                   regex_grouping_single_from_byte(ctx, 0),
                   regex_grouping_single_from_byte(ctx, 2));


  regex_ptree_as_xml(&stack_libc, stdout, expr);
  fprintf(stdout, "\n");

  regex_ptree_destroy_context(ctx);
  return 0;
}
