#include "ascii.declarations.h"
#include "ascii.productions.h"
#include "ascii.ptree.h"

#include "regex.h"
#include "parse_hex.h"

#include "../tools/stack.libc.h"

ptree ascii_custom_production_union_ctor(ptree_context ctx,
                                         ptree /*result_RE*/ x1,
                                         token /*PIPE*/ x2,
                                         ptree /*simple_RE*/ x3) {
  return regex_make_or(ctx, x1, x3);
}

ptree ascii_custom_production_concat_ctor(ptree_context ctx,
                                          ptree /*simple_RE*/ x1,
                                          ptree /*basic_RE*/ x2) {
  return regex_make_concat(ctx, x1, x2);
}

ptree ascii_custom_production_star_ctor(ptree_context ctx,
                                        ptree /*elementary_RE*/ x1,
                                        token /*STAR*/ x2) {
  return regex_make_kleene(ctx, x1);
}

ptree ascii_custom_production_plus_ctor(ptree_context ctx,
                                        ptree /*elementary_RE*/ x1,
                                        token /*PLUS*/ x2) {
  return regex_make_concat(ctx, x1, regex_make_kleene(ctx, x1));
}

ptree ascii_custom_production_question_ctor(ptree_context ctx,
                                        ptree /*elementary_RE*/ x1,
                                        token /*QUESTION*/ x2) { 
  return regex_make_or(ctx, x1, regex_make_empty_string(ctx));
}

ptree ascii_custom_production_from_any(ptree_context ctx, token /*PERIOD*/ x1) {
  ptree any = regex_make_any_char(ctx);
  ptree nl = regex_grouping_single_from_byte(ctx, 0x0a);
  return regex_make_and(ctx, any, regex_make_not(ctx, nl));
}

ptree ascii_custom_production_set_from_negative_set(ptree_context ctx,
                                                    ptree /*negative_set*/ x1) {
  return regex_make_not(ctx, x1);
}

ptree ascii_custom_production_set_item_multiple(ptree_context ctx,
                                                ptree /*set_item*/ x1,
                                                ptree /*set_items*/ x2) {
  return regex_make_or(ctx, x1, x2);
}

ptree ascii_custom_production_range_ctor(ptree_context ctx,
                                         ptree /*char_RE*/ x1,
                                         token /*HYPHEN*/ x2,
                                         ptree /*char_RE*/ x3) {
  if (!regex_is_single_byte(x1) || !regex_is_single_byte(x3)) {
    printf("range ctor passed non-single bytes\n");
    return ptree_failure();
  }

  uint8_t left = regex_grouping_extract_single_byte(regex_ptree_identifier(x1));
  uint8_t right =
      regex_grouping_extract_single_byte(regex_ptree_identifier(x3));

  if (left > right) {
    // Could be match nothing, could be an error
    printf("failing range ctor as left > right, %u %u\n", left, right);
    return ptree_failure();
  }

  ptree cursor = regex_grouping_single_from_byte(ctx, right);

  while (left != right) {
    right--;
    ptree tmp = regex_grouping_single_from_byte(ctx, right);
    cursor = regex_make_or(ctx, tmp, cursor);
  }

  return cursor;
}

ptree ascii_custom_production_from_alphanumeric(ptree_context ctx,
                                           token /*Alphanumeric*/ x1) {
  if (x1.width == 1) {
    uint8_t u = x1.value[0];
    bool decimal = (u >= 0x30u) && (u <= 0x39u);
    bool lower = (u >= 0x61u) && (u <= 0x7au);
    bool upper = (u >= 0x41u) && (u <= 0x5au);
    if (decimal || lower || upper) {
      return regex_grouping_single_from_byte(ctx, u);
    }
  }
  return ptree_failure();
}

ptree ascii_custom_production_from_escaped_hex(ptree_context ctx,
                                             token /*Escaped_hex*/ x1) {
  if ((x1.width == 4) &&
      (x1.value[0] == '\\') &&
      (x1.value[1] == 'x'))
    {
      uint8_t l = parse_hex(x1.value[2]);
      uint8_t r = parse_hex(x1.value[3]);

      if ((l < 16) && (r < 16))
        {
          uint8_t byte = (uint8_t)(l * 16) + r;
          return regex_grouping_single_from_byte(ctx, byte);
        }    
    }
  return ptree_failure();
}

ptree ascii_custom_production_from_escaped_character(ptree_context ctx,
                                             token /*Escaped_character*/ x1) {
  if ((x1.width == 2) &&
      (x1.value[0] == '\\'))
    {
      switch(x1.value[1])
        {
        case 'f': return regex_grouping_single_from_byte(ctx, 0x0c);
        case 'n': return regex_grouping_single_from_byte(ctx, 0x0a);
        case 'r': return regex_grouping_single_from_byte(ctx, 0x0d);
        case 't': return regex_grouping_single_from_byte(ctx, 0x09);
        case 'v': return regex_grouping_single_from_byte(ctx, 0x0b);
        case 'a': return regex_grouping_single_from_byte(ctx, 0x07);
        case 'e': return regex_grouping_single_from_byte(ctx, 0x1b);
        default: break;
        }
    }
  return ptree_failure();
}

