#include <cassert>
#include <re2/re2.h>
#include <re2/set.h>
#include <vector>

// lemon will let one add tokens that aren't used
// by the parser, and specify a prefix, but changing it
// to an enum trips up makeheader
#define TOKEN_ID_PLUS 1
#define TOKEN_ID_MINUS 2
#define TOKEN_ID_TIMES 3
#define TOKEN_ID_DIVIDE 4
#define TOKEN_ID_LPAR 5
#define TOKEN_ID_RPAR 6
#define TOKEN_ID_INTEGER 7
#define TOKEN_ID_WHITESPACE 8

// Lemon's assigned integers start at 1, can control the order
#define TOKEN_ID_MAX TOKEN_ID_WHITESPACE

enum { TOKEN_ID_UNKNOWN = 0 };

const char *token_names[] = {
    [TOKEN_ID_UNKNOWN] = "TOKEN_ID_UNKNOWN",
    [TOKEN_ID_PLUS] = "TOKEN_ID_PLUS",
    [TOKEN_ID_MINUS] = "TOKEN_ID_MINUS",
    [TOKEN_ID_TIMES] = "TOKEN_ID_TIMES",
    [TOKEN_ID_DIVIDE] = "TOKEN_ID_DIVIDE",
    [TOKEN_ID_LPAR] = "TOKEN_ID_LPAR",
    [TOKEN_ID_RPAR] = "TOKEN_ID_RPAR",
    [TOKEN_ID_INTEGER] = "TOKEN_ID_INTEGER",
    [TOKEN_ID_WHITESPACE] = "TOKEN_ID_WHITESPACE",
};
enum { token_names_size = sizeof(token_names) / sizeof(token_names[0]) };

const char *regexes[] = {
    [TOKEN_ID_UNKNOWN] = ".",
    [TOKEN_ID_PLUS] = R"([+])",
    [TOKEN_ID_MINUS] = R"(-)",
    [TOKEN_ID_TIMES] = R"([*])",
    [TOKEN_ID_DIVIDE] = R"([\/])",
    [TOKEN_ID_LPAR] = R"(\()",
    [TOKEN_ID_RPAR] = R"(\))",
    [TOKEN_ID_INTEGER] = R"((0|-*[1-9]+[0-9]*))",
    [TOKEN_ID_WHITESPACE] = R"(([ \n\t\r\v])+)",
};
enum { regexes_size = sizeof(regexes) / sizeof(regexes[0]) };

static_assert((size_t)regexes_size == (size_t)token_names_size, "");

static_assert(regexes_size == TOKEN_ID_MAX + 1, "");

struct token {
  const char *name;
  std::string value;
};

int main() {
  const bool verbose = false;
  const char *example = " 10 + 2 * (4 /\t2)    - 1 ";
  re2::StringPiece cursor(example);

  std::vector<token> vector_tokens;

  RE2::Options opts;
  opts.set_dot_nl(true);
  opts.set_log_errors(false);
  // maybe never-capture for the set object
  RE2::Set set(opts, RE2::ANCHOR_START);

  {
    std::string err;
    for (size_t i = 0; i < regexes_size; i++) {
      int rc = set.Add(regexes[i], &err);
      if (rc == -1) {
        fprintf(stderr, "Cannot parse regex[%zu] \"%s\": %s\n", i, regexes[i],
                err.c_str());
        return 1;
      }
      assert(rc == i);
    }
  }

  if (!set.Compile()) {
    fprintf(stderr, "Out of memory on Set::Compile\n");
    return 1;
  }

  std::vector<int> matches;

  while (!cursor.empty()) {
    if (verbose)
      printf("\nConsume [%s]\n", cursor.as_string().c_str());

    {
      RE2::Set::ErrorInfo err;
      if (!set.Match(cursor, &matches)) {
        fprintf(stderr, "Match failed (%u)\n", err.kind);
        return 1;
      }
    }

    // Unknown is always going to match and is token zero
    std::sort(matches.begin(), matches.end());
    assert(matches.size() > 0);
    if (matches.size() == 1) {
      assert(matches[0] == TOKEN_ID_UNKNOWN);
      fprintf(stderr, "Only match was unknown fallback\n");
      return 1;
    }

    assert(matches[0] == TOKEN_ID_UNKNOWN);
    assert(matches.size() > 1);
    int winning = matches[1];
    if (verbose)
      for (size_t i = 0; i < matches.size(); i++) {
        printf("Regex %s |%s| matched\n", token_names[matches[i]],
               regexes[matches[i]]);
      }

    size_t size_before = cursor.size();

    if (verbose)
      printf("Consume on [%s] using regex %d, %s\n", cursor.as_string().c_str(),
             winning, token_names[winning]);

    // Original game plan was:
    // std::string var;
    // bool C = RE2::Consume(&cursor, regexes[winning], &var);
    // However that isn't working on escaped regexes, e.g. trying to match +
    const constexpr bool consume_works = false;

    std::string var;

    bool C;
    if (consume_works) {
      C = RE2::Consume(&cursor, regexes[winning], &var);
      if (verbose)
        printf("Consume ret %s\n", (C ? "true" : "false"));
    } else {
      re2::StringPiece submatch[1];
      re2::RE2 tmp(regexes[winning]);
      C = tmp.Match(cursor, 0, cursor.size(), RE2::ANCHOR_START, submatch, 1);
      if (verbose)
        printf("Match ret %s\n", (C ? "true" : "false"));
      var = submatch[0].as_string();
      cursor.remove_prefix(submatch[0].size());
    }

    if (verbose && !C) {
      printf("Consume returned false\n");
    }

    vector_tokens.push_back({token_names[winning], var});

    if (verbose) {
      printf("Result? |%s|\n", var.c_str());
      printf("New cursor %s\n", cursor.as_string().c_str());
    }

    size_t size_after = cursor.size();

    if (size_after >= size_before) {
      fprintf(stderr, "Cursor size did not decrease, %zu -> %zu\n", size_before,
              size_after);
      return 1;
    } else {
      if (verbose)
        printf("Cursor size decreased, %zu -> %zu\n", size_before, size_after);
    }
  }

  printf("Input %s\n", example);
  printf("Lexed {");
  const char *sep = "";
  for (size_t i = 0; i < vector_tokens.size(); i++) {
    printf("%s{%s: |%s|}", sep, vector_tokens[i].name,
           vector_tokens[i].value.c_str());
    sep = ", ";
  }
  printf("}\n");

  return 0;
}
