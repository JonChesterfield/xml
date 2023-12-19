#include "lexer.h"
#include <memory>
#include <vector>
#include <string>

#include <re2/re2.h>
#include <re2/set.h>

#include <cassert>

static const constexpr bool verbose = false;

static std::unique_ptr<RE2::Set> set_from_regex_table(size_t regexes_size,
                                                      const char **regexes) {
  RE2::Options opts;
  opts.set_dot_nl(true);
  opts.set_log_errors(false);
  // maybe never-capture for the set object
  std::unique_ptr<RE2::Set> set =
      std::make_unique<RE2::Set>(opts, RE2::ANCHOR_START);

  {
    std::string err;
    for (size_t i = 0; i < regexes_size; i++) {
      int rc = set->Add(regexes[i], &err);
      if (rc == -1) {
        fprintf(stderr, "Cannot parse regex[%zu] \"%s\": %s\n", i, regexes[i],
                err.c_str());
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

bool lexer_success(lexer_state s) { return s.set_data != nullptr; }

using instance_type = std::vector<std::unique_ptr<re2::RE2>>;

lexer_state lexer_create(size_t N, const char **token_names,
                         const char **regexes) {
  assert(N > 0);
  assert(regexes[0] == std::string("."));
  std::unique_ptr<RE2::Set> data = set_from_regex_table(N, regexes);

  auto instances = std::make_unique<instance_type>();
  for (size_t i = 0; i < N; i++) {
    instances->push_back(std::make_unique<re2::RE2>(regexes[i]));
  }
  
  
  return (lexer_state){.set_data = static_cast<void *>(data.release()),
                       .inst_data = static_cast<void *>(instances.release()),
                       .N = N,
                       .token_names = token_names,
                       .regexes = regexes};
}

void lexer_destroy(lexer_state s) {
  if (lexer_success(s)) {
    std::unique_ptr<RE2::Set> data(static_cast<RE2::Set *>(s.set_data));
  }
}

token lexer_next(lexer_state s, const char *start, const char *end) {
  assert(start != end);
  assert(start < end);

  assert(lexer_success(s));
  constexpr const size_t TOKEN_ID_UNKNOWN = 0;

  RE2::Set &set = *static_cast<RE2::Set *>(s.set_data);
  const instance_type & instances = *static_cast<instance_type*>(s.inst_data);
  assert(instances.size() == s.N);
  std::vector<int> matches;
  re2::StringPiece cursor(start, end - start);
  RE2::Set::ErrorInfo err;
  if (!set.Match(cursor, &matches)) {
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
      printf("Regex %s |%s| matched\n", s.token_names[matches[i]],
             s.regexes[matches[i]]);
    }

  size_t size_before = cursor.size();

  if (verbose)
    printf("Consume on [%s] using regex %d, %s\n", cursor.as_string().c_str(),
           winning, s.token_names[winning]);

  // Original game plan was:
  // std::string var;
  // bool C = RE2::Consume(&cursor, regexes[winning], &var);
  // However that isn't working on escaped regexes, e.g. trying to match +
  const constexpr bool consume_works = false;

  size_t token_length = 0;

  bool C;
  if (consume_works) {
    std::string var;
    C = RE2::Consume(&cursor, s.regexes[winning], &var);
    if (verbose)
      printf("Consume ret %s\n", (C ? "true" : "false"));
    if (C) {
      token_length = var.size();
    }
  } else {
    // TODO: This here is dubious. There are N regex strings and building a
    // RE2 instance for one is probably expensive, should build the N RE2 instances
    // in the constructor.
    re2::StringPiece submatch[1];
    // Reproducibly quicker to reuse previously build regex instances instead of building
    // from scratch, probably worth the complexity. Would be much simpler if the C interface
    // is dropped, which is probably a reasonable improvement in itself.
#if 0
    re2::RE2 tmp(s.regexes[winning]);
#else
    re2::RE2 const &tmp = *instances[winning];
#endif
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
  return (token){
      .name = winning,
      .value_start = start,
      .value_end = start + token_length,
  };
}
