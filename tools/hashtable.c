#include "hashtable.h"

#include "EvilUnit/EvilUnit.h"

MAIN_MODULE() {
  DEPENDS(intset);
  DEPENDS(intmap);
  DEPENDS(stringtable);
  DEPENDS(intstack); // not a hashtable, maybe rename this runner
}
