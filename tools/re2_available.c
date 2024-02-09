int main(void)
{
#if __has_include(<re2/re2.h>) && __has_include(<re2/set.h>)
  return 1;
#else
  return 0;
#endif
}
