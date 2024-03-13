#include "ascii_interpreter.h"

#include "interpreter.data"
#include "regex_interpreter.h"

#include "ascii.parser_lemon.h"
#include "regex_string.h"

bool ascii_interpreter_match_failure(uint64_t x) { return regex_interpreter_match_failure(x);}
bool ascii_interpreter_machine_failure(uint64_t x){ return regex_interpreter_machine_failure(x);}
bool ascii_interpreter_regex_unrecognised(uint64_t x){ return regex_interpreter_regex_unrecognised(x);}
bool ascii_interpreter_success(uint64_t x) { return regex_interpreter_success(x);}

char * ascii_regex_as_prefix_regex_c_string(const char *regex)
{
  ptree_context ascii_context = regex_ptree_create_context();
  ptree ascii = ascii_parser_lemon_parse_cstr(ascii_context, regex);
  if (ptree_is_failure(ascii)) {
    regex_ptree_destroy_context(ascii_context);
    return 0;
  }

  char * regstr = regex_to_malloced_c_string(ascii);
  regex_ptree_destroy_context(ascii_context);
  return regstr;
}

uint64_t ascii_interpreter_string_matches(const char* regex,
                                          unsigned char *target,
                                          size_t target_len)
{

  char * regstr = ascii_regex_as_prefix_regex_c_string(regex);
  // more likely than malloc failing. todo, work through oom implications on api
  if (!regstr) { return regex_unrecognised; }
  size_t regstrN = strlen(regstr);

  printf("got a prefix %s\n", regstr);
  for (size_t i = 0; i < target_len; i++)
    {
      printf("%02x", target[i]);
    }
  printf("\n");
  uint64_t res = regex_interpreter_string_matches((unsigned char*) regstr, regstrN, target, target_len);
  free(regstr);

  return res;
}
