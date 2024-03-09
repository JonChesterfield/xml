#include "../tools/EvilUnit/EvilUnit.h"

#include "../tools/arena.libc.h"
#include "../tools/stack.libc.h"
#include "regex.declarations.h"
#include "regex.h"
#include "regex.ptree.h"

#include "regex_string.h"
#include "regex_driver.h"

#include <string.h>

static MODULE(regex_nullable) {
  // Fun. So this leaks. The ptree context builds a tree from the last appended node,
  // but if that leaves part of the tree unreachable, that part leaks
  // Inclined to fix by declaring that the context always has arena semantics
  // and building a better linked structure on the fly for the versions backed
  // by libc (as opposed to areans on mmap or similar)
  ptree_context ctx = regex_ptree_create_context();
  ptree empty_str = regex_make_empty_string(ctx);
  ptree empty_set = regex_make_empty_set(ctx);

  TEST("empty string to empty string") {
    ptree s = regex_make_empty_string(ctx);
    CHECK(regex_is_empty_string(s));
    CHECK(regex_is_empty_string(regex_nullable(ctx, s)));
  }

  TEST("empty set to empty set") {
    ptree s = regex_make_empty_set(ctx);
    CHECK(regex_is_empty_set(s));
    CHECK(regex_is_empty_set(regex_nullable(ctx, s)));
  }

  TEST("single bytes to empty set") {
    bool ok = true;
    for (unsigned i = 0; i < 256; i++) {
      uint8_t byte = (uint8_t)i;
      ptree b = regex_grouping_single_from_byte(ctx, byte);
      ok &= (regex_grouping_id_is_single_byte(regex_ptree_identifier(b)));
      ok &= (byte ==
             regex_grouping_extract_single_byte(regex_ptree_identifier(b)));
      ok &= (regex_is_empty_set(regex_nullable(ctx, b)));
    }
    CHECK(ok);
  }

  TEST("keele on empty") {
    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_kleene(ctx, empty_str))));
    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_kleene(ctx, empty_set))));

    ptree indir = regex_make_kleene(ctx, regex_make_byte_00(ctx));
    CHECK(regex_is_empty_string(regex_nullable(ctx, indir)));
  }

  TEST("concat on empty") {
    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_concat(ctx, empty_str, empty_str))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_concat(ctx, empty_set, empty_set))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_concat(ctx, empty_str, empty_set))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_concat(ctx, empty_set, empty_str))));

    ptree indir_str = regex_make_kleene(ctx, regex_make_byte_00(ctx));
    CHECK(regex_is_empty_string(regex_nullable(ctx, indir_str)));

    ptree indir_set = regex_make_byte_01(ctx);
    CHECK(regex_is_empty_set(regex_nullable(ctx, indir_set)));

    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_concat(ctx, indir_str, indir_str))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_concat(ctx, indir_set, indir_set))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_concat(ctx, indir_str, indir_set))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_concat(ctx, indir_set, indir_str))));
  }

  TEST("and on empty") {
    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_and(ctx, empty_str, empty_str))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_and(ctx, empty_set, empty_set))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_and(ctx, empty_str, empty_set))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_and(ctx, empty_set, empty_str))));

    ptree indir_str = regex_make_kleene(ctx, regex_make_byte_00(ctx));
    CHECK(regex_is_empty_string(regex_nullable(ctx, indir_str)));

    ptree indir_set = regex_make_byte_01(ctx);
    CHECK(regex_is_empty_set(regex_nullable(ctx, indir_set)));

    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_and(ctx, indir_str, indir_str))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_and(ctx, indir_set, indir_set))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_and(ctx, indir_str, indir_set))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_and(ctx, indir_set, indir_str))));
  }

  TEST("or on empty") {
    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_or(ctx, empty_str, empty_str))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_or(ctx, empty_set, empty_set))));
    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_or(ctx, empty_str, empty_set))));
    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_or(ctx, empty_set, empty_str))));

    ptree indir_str = regex_make_kleene(ctx, regex_make_byte_00(ctx));
    CHECK(regex_is_empty_string(regex_nullable(ctx, indir_str)));

    ptree indir_set = regex_make_byte_01(ctx);
    CHECK(regex_is_empty_set(regex_nullable(ctx, indir_set)));

    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_or(ctx, indir_str, indir_str))));
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_or(ctx, indir_set, indir_set))));
    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_or(ctx, indir_str, indir_set))));
    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_or(ctx, indir_set, indir_str))));
  }

  TEST("not on empty") {
    ptree empty_str = regex_make_empty_string(ctx);
    ptree empty_set = regex_make_empty_set(ctx);
    CHECK(regex_is_empty_set(
        regex_nullable(ctx, regex_make_not(ctx, empty_str))));
    CHECK(regex_is_empty_string(
        regex_nullable(ctx, regex_make_not(ctx, empty_set))));
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
    CHECK(regex_ptree_compare(&stack_libc, left, right) ==
          ptree_compare_lesser);

    CHECK(regex_ptree_compare(&stack_libc, mid, mid) == ptree_compare_equal);

    CHECK(regex_ptree_compare(&stack_libc, right, mid) ==
          ptree_compare_greater);
  }

  regex_ptree_destroy_context(ctx);
}

static MODULE(regex_split) {
  ptree_context ctx = regex_ptree_create_context();

  TEST("serialise") {
    arena_t arena = arena_create(&arena_libc, 64);

    ptree regex = regex_make_or(
        ctx, regex_make_kleene(ctx, regex_make_byte_00(ctx)),

        regex_make_or(ctx, regex_make_byte_02(ctx),
                      regex_make_not(ctx, regex_make_byte_cc(ctx))));

    regex_to_char_sequence(&arena_libc, &arena, regex);

    printf("%s\n", (char *)arena_base_address(&arena_libc, arena));

    arena_destroy(&arena_libc, arena);
  }

  TEST("wip") {
    return;
    ptree regex =
        regex_make_or(ctx, regex_make_kleene(ctx, regex_make_byte_00(ctx)),
                      regex_make_byte_02(ctx));

    regex_ptree_as_xml(&stack_libc, stdout, regex);
    fprintf(stdout, "\n");

    ptree *tmp = malloc(256 * sizeof(ptree));
    if (tmp) {
      regex_split(ctx, regex, tmp);

      for (size_t i = 0; i < 256; i++) {
        fprintf(stdout, "Regex %zu\n", i);
        regex_ptree_as_xml(&stack_libc, stdout, tmp[i]);
        fprintf(stdout, "\n");
      }
    }
    free(tmp);
  }

  regex_ptree_destroy_context(ctx);
}

static MODULE(regex_string)
{
  arena_t arena = arena_create(&arena_libc, 64);
  ptree_context ctx = regex_ptree_create_context();
  
  TEST("parse and print a sequence")
  {
    const char seq[] = "(|(*00)(|02(~cc)))";

    const size_t N = strlen(seq);
    
    ptree tmp = regex_from_char_sequence(ctx, seq, N);
    CHECK(!ptree_is_failure(tmp));

    char * out = regex_to_malloced_c_string(tmp);
    uint64_t size = out ? strlen(out) : 0;

    CHECK(out != NULL);
    CHECK(size == N);
    if (size == N)
      {
        CHECK(memcmp(seq, out, N) == 0);
      }

    free(out);
  }

  TEST("syntax to/from c identifier reversible")
    {
      for (unsigned i = 0; i < 256; i++)
        {
          unsigned char src = (unsigned char)i;

          {
            unsigned char cid = regex_syntax_byte_to_c_identifier_byte(src);
            unsigned char syn = regex_c_identifer_byte_to_syntax_byte(cid);
            CHECK(src == syn);
          }

          {
            unsigned char syn = regex_c_identifer_byte_to_syntax_byte(src);
            unsigned char cid = regex_syntax_byte_to_c_identifier_byte(syn);
            if (src != cid) {
              printf("I %u: src %c, syn %c, cid %c\n", i, src, syn, cid);
            }
            CHECK(src == cid);
          }
        }
    }

  TEST("syn/cid lowercase hex digits unchanged")
    {
      unsigned char src[16] = {'0','1','2','3','4',
                               '5','6','7','8','9',
                               'a','b','c','d','e','f',};
      size_t N = sizeof(src);

      for (size_t i = 0; i < N; i++)
        {
          CHECK(src[i] == regex_c_identifer_byte_to_syntax_byte(src[i]));
          CHECK(src[i] == regex_syntax_byte_to_c_identifier_byte(src[i]));
        }
    }

  TEST("specific byte conversions")
    {
      // sufficient to know that 'accept' is not in the possible name set
      CHECK(regex_syntax_byte_to_c_identifier_byte('t') == 't');
      
      // parens
      CHECK(regex_syntax_byte_to_c_identifier_byte('(') == 'L');
      CHECK(regex_syntax_byte_to_c_identifier_byte(')') == 'R');

      // concat
      CHECK(regex_syntax_byte_to_c_identifier_byte(':') == 'C');
            
      // unary op
      CHECK(regex_syntax_byte_to_c_identifier_byte('*') == 'K');
      CHECK(regex_syntax_byte_to_c_identifier_byte('~') == 'N');

      // binary op
      CHECK(regex_syntax_byte_to_c_identifier_byte('&') == 'A');
      CHECK(regex_syntax_byte_to_c_identifier_byte('|') == 'O');

      // empty set
      CHECK(regex_syntax_byte_to_c_identifier_byte('%') == 'F');

      // empty string
      CHECK(regex_syntax_byte_to_c_identifier_byte('_') == 'E');
      
    }
  
  regex_ptree_destroy_context(ctx);
  arena_destroy(&arena_libc, arena);
}

static MODULE(driver)
{
  TEST("demo")
    {
      printf("DEMO\n");
      regex_cache_t D = regex_cache_create();
      CHECK(regex_cache_valid(D));
      const char * regstr = "";
        
      regstr = "(|" "(&(*(|0001))(|(*00)(|02(~cc))))" "(|(|0001)(|0203))" ")";

      regstr =  "(|(|0001)(|0203))" ;
      
      size_t N = __builtin_strlen(regstr);

      CHECK(regex_in_byte_representation(regstr, N));
      CHECK(regex_driver_insert(&D, regstr, N));

      stringtable_index_t index  = stringtable_insert(&D.strtab, regstr, N);
      CHECK(regex_driver_regex_to_c(&D, index));
      
      regex_cache_destroy(D);
    }
}

MAIN_MODULE() {
  DEPENDS(ptree);
  DEPENDS(intset);
  DEPENDS(intmap);
  DEPENDS(stringtable);
  DEPENDS(regex_nullable);
  DEPENDS(regex_split);
  DEPENDS(regex_string);
  DEPENDS(driver);
}
