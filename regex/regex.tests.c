#include "../tools/EvilUnit/EvilUnit.h"

#include "../tools/stack.libc.h"
#include "regex.declarations.h"
#include "regex.ptree.h"
#include "regex.h"

static MODULE(regex_nullable) {
  ptree_context ctx = regex_ptree_create_context();
  ptree empty_str = regex_make_empty_string(ctx);
  ptree empty_set = regex_make_empty_set(ctx);
  
  TEST("empty string to empty string")
    {
      ptree s = regex_make_empty_string(ctx);
      CHECK(regex_is_empty_string(s));
      CHECK(regex_is_empty_string(regex_nullable(ctx, s)));
    }

  TEST("empty set to empty set")
    {
      ptree s = regex_make_empty_set(ctx);
      CHECK(regex_is_empty_set(s));
      CHECK(regex_is_empty_set(regex_nullable(ctx, s)));
    }

  TEST("single bytes to empty set")
    {
      bool ok = true;
      for (unsigned i = 0; i < 256; i++)
        {
          uint8_t byte = (uint8_t)i;
          ptree b = regex_grouping_single_from_byte(ctx, byte);
          ok &= 
            (regex_grouping_id_is_single_byte(regex_ptree_identifier(b)));
          ok &= (byte == regex_grouping_extract_single_byte(regex_ptree_identifier(b)));
          ok &= (regex_is_empty_set(regex_nullable(ctx, b)));
        }
      CHECK(ok);
    }

  TEST("keele on empty")
    {
      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_kleene(ctx, empty_str))));
      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_kleene(ctx, empty_set))));

      ptree indir = regex_make_kleene(ctx, regex_make_byte_00(ctx));
      CHECK(regex_is_empty_string(regex_nullable(ctx, indir)));     
    }

  
  TEST("concat on empty")
    {
      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_concat(ctx, empty_str, empty_str))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_concat(ctx, empty_set, empty_set))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_concat(ctx, empty_str, empty_set))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_concat(ctx, empty_set, empty_str))));
      
      ptree indir_str = regex_make_kleene(ctx, regex_make_byte_00(ctx));
      CHECK(regex_is_empty_string(regex_nullable(ctx, indir_str)));

      ptree indir_set = regex_make_byte_01(ctx);
      CHECK(regex_is_empty_set(regex_nullable(ctx, indir_set)));
      
      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_concat(ctx, indir_str, indir_str))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_concat(ctx, indir_set, indir_set))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_concat(ctx, indir_str, indir_set))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_concat(ctx, indir_set, indir_str))));
    }

  
  TEST("and on empty")
    {
      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_and(ctx, empty_str, empty_str))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_and(ctx, empty_set, empty_set))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_and(ctx, empty_str, empty_set))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_and(ctx, empty_set, empty_str))));

      ptree indir_str = regex_make_kleene(ctx, regex_make_byte_00(ctx));
      CHECK(regex_is_empty_string(regex_nullable(ctx, indir_str)));

      ptree indir_set = regex_make_byte_01(ctx);
      CHECK(regex_is_empty_set(regex_nullable(ctx, indir_set)));

      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_and(ctx, indir_str, indir_str))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_and(ctx, indir_set, indir_set))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_and(ctx, indir_str, indir_set))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_and(ctx, indir_set, indir_str))));
      
    }

  TEST("or on empty")
    {
      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_or(ctx, empty_str, empty_str))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_or(ctx, empty_set, empty_set))));
      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_or(ctx, empty_str, empty_set))));
      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_or(ctx, empty_set, empty_str))));

      ptree indir_str = regex_make_kleene(ctx, regex_make_byte_00(ctx));
      CHECK(regex_is_empty_string(regex_nullable(ctx, indir_str)));

      ptree indir_set = regex_make_byte_01(ctx);
      CHECK(regex_is_empty_set(regex_nullable(ctx, indir_set)));

      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_or(ctx, indir_str, indir_str))));
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_or(ctx, indir_set, indir_set))));
      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_or(ctx, indir_str, indir_set))));
      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_or(ctx, indir_set, indir_str))));
    }


  TEST("not on empty")
    {
      ptree empty_str = regex_make_empty_string(ctx);
      ptree empty_set = regex_make_empty_set(ctx);
      CHECK(regex_is_empty_set(regex_nullable(ctx, regex_make_not(ctx, empty_str))));
      CHECK(regex_is_empty_string(regex_nullable(ctx, regex_make_not(ctx, empty_set))));
    }

  
  regex_ptree_destroy_context(ctx);
}

MODULE(ptree) {
  ptree_context ctx = regex_ptree_create_context();
  TEST("ctor/dtor") { (void)ctx; }

  TEST("make empty set") {
    ptree expr = regex_ptree_expression0(ctx, regex_grouping_empty_set);
    CHECK(regex_ptree_is_expression(expr));
    CHECK(regex_ptree_identifier(expr) == regex_grouping_empty_set);
    CHECK(regex_ptree_expression_elements(expr) == 0);
  }

  TEST("compare") {
    ptree left =
        regex_make_and(ctx, regex_make_byte_01(ctx), regex_make_byte_10(ctx));

    ptree mid =
        regex_make_and(ctx, regex_make_byte_bb(ctx), regex_make_byte_cd(ctx));

    ptree right =
        regex_make_and(ctx, regex_make_byte_ee(ctx), regex_make_byte_ef(ctx));

    CHECK(regex_ptree_compare(&stack_libc, left, mid) == ptree_compare_lesser);
    CHECK(regex_ptree_compare(&stack_libc, mid, right) == ptree_compare_lesser);
    CHECK(regex_ptree_compare(&stack_libc, left, right) == ptree_compare_lesser);


    CHECK(regex_ptree_compare(&stack_libc, mid, mid) == ptree_compare_equal);

    CHECK(regex_ptree_compare(&stack_libc, right, mid) == ptree_compare_greater);
  }

  regex_ptree_destroy_context(ctx);
}

MAIN_MODULE() { DEPENDS(ptree); DEPENDS(regex_nullable); }
