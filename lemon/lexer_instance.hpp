#ifndef LEXER_INSTANCE_HPP_INCLUDED
#define LEXER_INSTANCE_HPP_INCLUDED

#include <cstring>

#include "token.h"

#include <re2/re2.h>
#include <re2/set.h>

template <size_t N, const char *(&token_names)[N], const char *(&regexes)[N],
          const char *(&literals)[N]>
struct lexer_instance {
  enum { TOKEN_ID_UNKNOWN = 0 };
  enum { verbose = 0 };
  lexer_instance(const char *start, const char *end) : start(start), end(end) {

    for (size_t i = 0; i < N; i++) {
      assert((regexes[i] == nullptr) != (literals[i] == nullptr));
      if (regexes[i] != nullptr) {
        assert(literals[i] == nullptr);
        derived_regexes.push_back(std::string(regexes[i]));
      }
      if (literals[i] != nullptr) {
        assert(regexes[i] == nullptr);
        derived_regexes.push_back(re2::RE2::QuoteMeta(literals[i]));
      }

      regex_engines.push_back(std::make_unique<re2::RE2>(derived_regexes[i]));
    }

    if (verbose) {
      for (size_t i = 0; i < N; i++) {
        printf("Regex[%zu] = %s\n", i, derived_regexes[i].c_str());
      }
    }

    set_of_all_regexes = set_from_regex_table(derived_regexes);
  }

  lexer_instance(const char *start) : lexer_instance(start, strlen(start)) {}
  lexer_instance(const char *start, size_t len)
      : lexer_instance(start, start + len) {}

  explicit operator bool() const { return start != end; }

  token next() {

    std::vector<int> matches;
    re2::StringPiece cursor(start, end - start);
    RE2::Set::ErrorInfo err;
    if (!set_of_all_regexes->Match(cursor, &matches)) {
      fprintf(stderr, "Match failed (%u)\n", err.kind);
      exit(1);
    }

    // Unknown is always going to match and is token zero
    std::sort(matches.begin(), matches.end());
    assert(matches.size() > 0);
    if (matches.size() == 1) {
      assert(matches[0] == TOKEN_ID_UNKNOWN);
      fprintf(stderr, "Only match was unknown fallback\n");
      exit(1);
    }

    assert(matches[0] == TOKEN_ID_UNKNOWN);
    assert(matches.size() > 1);
    int winning = matches[1];
    if (verbose)
      for (size_t i = 0; i < matches.size(); i++) {
        printf("Regex %s |%s| matched\n", token_names[matches[i]],
               derived_regexes[matches[i]].c_str());
      }

    size_t size_before = cursor.size();

    // Original game plan was:
    // std::string var;
    // bool C = RE2::Consume(&cursor, regexes[winning], &var);
    // However that isn't working on escaped regexes, e.g. trying to match +
    const constexpr bool consume_works = false;

    size_t token_length = 0;

    bool C;
    if (consume_works) {
      std::string var;
      C = RE2::Consume(&cursor, derived_regexes[winning], &var);
      if (verbose)
        printf("Consume ret %s\n", (C ? "true" : "false"));
      if (C) {
        token_length = var.size();
      }
    } else {
      // TODO: This here is dubious. There are N regex strings and building a
      // RE2 instance for one is probably expensive, should build the N RE2
      // instances in the constructor.
      re2::StringPiece submatch[1];
      // Reproducibly quicker to reuse previously build regex instances instead
      // of building from scratch, probably worth the complexity. Would be much
      // simpler if the C interface is dropped, which is probably a reasonable
      // improvement in itself.
      re2::RE2 const &tmp = *regex_engines[winning];
      C = tmp.Match(cursor, 0, cursor.size(), RE2::ANCHOR_START, submatch, 1);
      if (verbose)
        printf("Match ret %s\n", (C ? "true" : "false"));
      if (C) {
        token_length = submatch[0].size();
      }
      cursor.remove_prefix(token_length);
    }

    if (verbose && !C) {
      printf("Consume returned false\n");
    }

    if (verbose) {
      printf("New cursor %s\n", cursor.as_string().c_str());
    }

    size_t size_after = cursor.size();

    if (size_after >= size_before) {
      fprintf(stderr, "Cursor size did not decrease, %zu -> %zu\n", size_before,
              size_after);
      exit(1);
    } else {
      if (verbose)
        printf("Cursor size decreased, %zu -> %zu\n", size_before, size_after);
    }

    assert((start + token_length) <= end);

    token res = {
        .name = winning,
        .value_start = start,
        .value_end = start + token_length,
    };

    start += token_length;

    return res;
  }

private:
  const char *start;
  const char *end;

  std::vector<std::string>
      derived_regexes; // the engine should have captured this
  std::vector<std::unique_ptr<re2::RE2>> regex_engines;
  std::unique_ptr<RE2::Set> set_of_all_regexes;

  static std::unique_ptr<RE2::Set>
  set_from_regex_table(std::vector<std::string> const &derived_regexes) {
    RE2::Options opts;
    opts.set_dot_nl(true);
    opts.set_log_errors(false);
    // maybe never-capture for the set object
    std::unique_ptr<RE2::Set> set =
        std::make_unique<RE2::Set>(opts, RE2::ANCHOR_START);

    {
      std::string err;
      for (size_t i = 0; i < derived_regexes.size(); i++) {
        int rc = set->Add(derived_regexes[i], &err);
        if (rc == -1) {
          fprintf(stderr, "Cannot parse regex[%zu] \"%s\": %s\n", i,
                  derived_regexes[i].c_str(), err.c_str());
          exit(1);
        }
        assert(rc == i);
      }
    }

    if (!set->Compile()) {
      fprintf(stderr, "Out of memory on Set::Compile\n");
      exit(1);
    }

    return set;
  }
};

#endif
