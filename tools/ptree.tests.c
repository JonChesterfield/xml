#include "ptree.h"
#include "EvilUnit/EvilUnit.h"

static MODULE(construct_tokens)
{

}

static MODULE(ptree)
{
  DEPENDS(construct_tokens);
  CHECK(1);

}

MAIN_MODULE()
{
  DEPENDS(ptree);
}

