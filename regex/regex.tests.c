#include "../tools/EvilUnit/EvilUnit.h"

#include "../tools/arena.libc.h"
#include "../tools/stack.libc.h"
#include "regex.declarations.h"
#include "regex.h"
#include "regex.ptree.h"

#include "regex_string.h"

#include <string.h>

static MODULE(regex_nullable) {
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
  
  TEST("wip")
  {

    printf("WIP\n");
    
    const char seq[] = "01";

const    size_t N = strlen(seq);
    fprintf(stderr, "Try to parse %s, len %lu\n", seq, N);
    
    ptree tmp = regex_from_char_sequence(ctx, seq, N);
    CHECK(!ptree_is_failure(tmp));

    fprintf(stdout, "xml\n");
    regex_ptree_as_xml(&stack_libc, stdout, tmp);
    fprintf(stdout, "\n");

    uint64_t before = arena_size(&arena_libc, arena);
    CHECK(regex_to_char_sequence(&arena_libc, &arena, tmp) == 0);

    uint64_t after = arena_size(&arena_libc, arena);

    uint64_t size = after - before;
    const char * cursor = before + (char*)arena_base_address(&arena_libc, arena);

    fprintf(stderr, "N = %lu\n", N);
    fprintf(stderr, "size = %lu\n", size);
    fprintf(stderr, "thing %s\n", cursor);
    CHECK(size == N);
    
    
    
  }
  
  regex_ptree_destroy_context(ctx);
    arena_destroy(&arena_libc, arena);
}

MAIN_MODULE() {
  DEPENDS(ptree);
  DEPENDS(regex_nullable);
  DEPENDS(regex_split);
  DEPENDS(regex_string);
}
