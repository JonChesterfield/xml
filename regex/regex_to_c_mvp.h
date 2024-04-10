#ifndef REGEX_TO_C_MVP_H_INCLUDE
#define REGEX_TO_C_MVP_H_INCLUDE

#include "regex.ptree.h"

// 0 on success
int regex_to_c_mvp(const char *lang, const char *prefix, ptree_context, ptree regex);

int regexes_to_c_lexer_mvp(ptree_context context, const char *lang,
                           ptree (*parser)(ptree_context, const char *),
                           size_t token_count, const char **tokens,
                           const char **regexes);

#endif
