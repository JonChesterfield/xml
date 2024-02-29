#include "../tools/EvilUnit/EvilUnit.h"

#include "../tools/stack.libc.h"
#include "regex.declarations.h"
#include "regex.ptree.h"

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

MAIN_MODULE() { DEPENDS(ptree); }
