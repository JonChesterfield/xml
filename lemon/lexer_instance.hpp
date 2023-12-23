#ifndef LEXER_INSTANCE_HPP_INCLUDED
#define LEXER_INSTANCE_HPP_INCLUDED

#include <cstring>

#include "../tools/io_buffer.h"
#include "token.h"

#include <re2/re2.h>
#include <re2/set.h>

template <size_t regexes_size, const char *(&token_names)[regexes_size],
          const char *(&regexes)[regexes_size],
          const char *(&literals)[regexes_size]>
struct lexer_instance;

template <size_t regexes_size, const char *(&token_names)[regexes_size],
          const char *(&regexes)[regexes_size],
          const char *(&literals)[regexes_size]>
int foreach_token_in_file(FILE *f, int (*func)(token tok)) {
  using io_buffer_deleter = struct {
    void operator()(io_buffer *b) { free(b); }
  };

  auto all_stdin =
      std::unique_ptr<io_buffer, io_buffer_deleter>(file_to_io_buffer(stdin));
  if (!all_stdin) {
    return 1;
  }

  using LexerType =
      lexer_instance<regexes_size, token_names, regexes, literals>;
  auto lexer = LexerType(all_stdin->data, all_stdin->N);
  if (!lexer) {
    return 2;
  }

  while (lexer) {
    token tok = lexer.next();
    if (token_empty(tok)) {
      return 3;
    }

    if (func(tok) != 0) {
      return 4;
    }
  }

  return 0;
}

template <size_t regexes_size, const char *(&token_names)[regexes_size],
          const char *(&regexes)[regexes_size],
          const char *(&literals)[regexes_size]>
struct lexer_instance {
  enum { TOKEN_ID_UNKNOWN = 0 };
  enum { verbose = false };

  static RE2::Options config()
  {
    RE2::Options opts;
    opts.set_dot_nl(true);
    opts.set_log_errors(false);
    return opts;
  }
  
  lexer_instance(const char *start, const char *end) : start(start), end(end) {
    RE2::Options opts = config();
    for (size_t i = 0; i < regexes_size; i++) {
      assert((regexes[i] == nullptr) != (literals[i] == nullptr));
      if (regexes[i] != nullptr) {
        assert(literals[i] == nullptr);
        derived_regexes.push_back(std::string(regexes[i]));
      }
      if (literals[i] != nullptr) {
        assert(regexes[i] == nullptr);
        derived_regexes.push_back(re2::RE2::QuoteMeta(literals[i]));
      }

      regex_engines.push_back(std::make_unique<re2::RE2>(derived_regexes[i], opts));
    }

    if (verbose) {
      for (size_t i = 0; i < regexes_size; i++) {
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
    if (!set_of_all_regexes->Match(cursor, &matches, &err)) {
      fprintf(stderr, "Match failed (%u)\n", err.kind);
      exit(1);
    }

    // Unknown is always going to match and is token zero
    std::sort(matches.begin(), matches.end());
    assert(matches.size() > 0);
    if (matches.size() == 1) {
      // Not generally a good sign
      assert(matches[0] == TOKEN_ID_UNKNOWN);
    }

    // Kind of messy rearranging. Want the lowest index other than
    // unknown, unless only unknown hits.
    assert(matches[0] == TOKEN_ID_UNKNOWN);
    int winning = matches.size() > 1 ? matches[1] : matches[0];
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
        printf("Match %d ret %s\n", winning, (C ? "true" : "false"));
      if (C) {
        token_length = submatch[0].size();
      }
      cursor.remove_prefix(token_length);
    }

    if (verbose && !C) {
      printf("Consume returned false\n");
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
    RE2::Options opts = config();
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
        assert(rc >= 0 && ((size_t)rc == i));
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
