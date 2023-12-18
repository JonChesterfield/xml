#include <re2/re2.h>
#include <cassert>
#include <cstdio>

int main()
{
  printf("Hi\n");

  assert(RE2::FullMatch("hello", "h.*o"));
  assert(!RE2::FullMatch("hello", "w.*d"));
}
