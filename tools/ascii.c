#define ASCII_TABLE_ENABLE 1
#include "ascii.h"

#include <assert.h>
#include <stdio.h>

int main()
{
  for (unsigned i = 0; i < 256; i++)
    {
      char buf[8];
      int c = snprintf(buf,8,"%02x",i);
      assert(c == 2);

      const char * hex = ascii_char_to_hex((char)i);
      assert(buf[0] == hex[0]);
      assert(buf[1] == hex[1]);
      assert(hex[2] == '\0');
      assert(buf[2] == '\0');

      if (i < 128) {
        assert(buf[0] == ascii_table[i].hex[0]);
        assert(buf[1] == ascii_table[i].hex[1]);
        assert(buf[2] == ascii_table[i].hex[2]);
      }
    }

  for (unsigned i = 0; i < 128; i++)
    {
      assert(ascii_char_to_type((char)i) == ascii_table[i].type);
    }
  for (unsigned i = 128; i < 256; i++)
    {
      assert(ascii_char_to_type((char)i) == out_of_range);      
    }

  for (unsigned i = 0; i < 256; i++)
    {
      const char * n = ascii_char_to_name((char)i);
      const char * hex = ascii_char_to_hex((char)i);
      
      for (unsigned j = 0; j < 3; j++)
        {
          if (i < 128) {
            assert(n[j] == ascii_table[i].name[j]);
          } else {
            assert(n[j] == hex[j]);
          }
        }
    }

  const char * ptr_table = (const char*)&ascii_table;
  const char *str_table = (const char*) &ascii_stringtable;
  for (unsigned i = 0; i < 1024; i++) {

    bool ok = (ptr_table[i] == str_table[i]);
    if (!ok)
    printf("[%u] %s %s %d ?= %d %c %c\n",
           i,
           ascii_char_to_name(i / 8),
           (ok ? "pass" : "fail"),
           (int)ptr_table[i],
           (int)str_table[i],
           ptr_table[i],
           str_table[i]


           );    
  }


  for (unsigned i = 0; i < 256; i++)
    {
      char c = (char)i;
      assert(ascii_table_char_to_type(c) ==
             ascii_stringtable_char_to_type(c));

      {
      const char * t = ascii_table_char_to_name(c);
      const char * s = ascii_stringtable_char_to_name(c);
      for (int j = 0; j < 4; j++)
        {
          assert(t[j] == s[j]);
        }
      }

            {
      const char * t = ascii_table_char_to_hex(c);
      const char * s = ascii_stringtable_char_to_hex(c);
      for (int j = 0; j < 3; j++)
        {
          assert(t[j] == s[j]);
        }
      }

      
    }
  
  return 0;
}
