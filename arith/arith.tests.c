#include "arith.ptree.h"
#include "arith.declarations.h"

#include "../tools/EvilUnit/EvilUnit.h"

#include "../tools/generic_ptree.h"

MODULE(ptree)
{
    ptree_context ctx = arith_ptree_create_context();
  TEST("ctor/dtor") {
    (void)ctx;
  }

  TEST("make token")
    {
      const char * value = "+";
      ptree plus = arith_ptree_from_token(ctx, arith_token_PLUS, value, 1);
      CHECK(arith_ptree_is_token(plus));
      CHECK(arith_ptree_identifier(plus) == arith_token_PLUS);
      CHECK(arith_ptree_token_value(plus) == value);
      CHECK(arith_ptree_token_width(plus) == 1);
    }

  TEST("make empty expression")
    {
      uint64_t id = 42;
      ptree expr = arith_ptree_expression0(ctx, id);
      CHECK(arith_ptree_is_expression(expr));
      CHECK(arith_ptree_identifier(expr) == id);
      CHECK(arith_ptree_expression_elements(expr) == 0);
    }
  
  TEST("make unary expression")
    {
      uint64_t id = 42;
      ptree expr = arith_ptree_expression2(ctx,
                                           id,
                                           arith_ptree_expression0(ctx, 101),
                                           arith_ptree_from_token(ctx, arith_token_PLUS, "+", 1)
                                           );
      CHECK(arith_ptree_is_expression(expr));
      CHECK(arith_ptree_identifier(expr) == id);
      CHECK(arith_ptree_expression_elements(expr) == 2);

      {
        ptree_context gctx= generic_ptree_create_context();
        ptree generic = generic_ptree_from_other_ptree(gctx, arith_ptree_retrieve_module(), expr);
        generic_ptree_to_file(stdout, generic);
        fprintf(stdout, "\n");
        generic_ptree_destroy_context(gctx);                                                     
      }

      arith_ptree_as_xml(stdout, expr);
      fprintf(stdout, "\n");
    }
  

  arith_ptree_destroy_context(ctx);
}

MAIN_MODULE()
{
  DEPENDS(ptree);
}
