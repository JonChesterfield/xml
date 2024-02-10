#include "arith.ptree.h"
#include "arith.declarations.h"

#include "../tools/EvilUnit/EvilUnit.h"

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
      ptree expr = arith_ptree_expression1(ctx, id, arith_ptree_expression0(ctx, id));
      CHECK(arith_ptree_is_expression(expr));
      CHECK(arith_ptree_identifier(expr) == id);
      CHECK(arith_ptree_expression_elements(expr) == 1);
    }
  

  arith_ptree_destroy_context(ctx);
}

MAIN_MODULE()
{
  DEPENDS(ptree);
}
