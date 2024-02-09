extern "C"
{
#include "lexer.re2.h"
}

#if LEXER_RE2_ENABLE

#include <re2/re2.h>
#include <re2/set.h>

#include <cassert>

namespace
{
enum
{
  verbose = false
};
enum
{
  TOKEN_ID_UNKNOWN = 0
};

struct re2_lexer
{
#ifndef NDEBUG
  enum lexer_engines engine = lexer_engines_re2;
#endif

  std::vector<std::unique_ptr<re2::RE2>> regex_engines;
  std::unique_ptr<RE2::Set> set_of_all_regexes;

  re2_lexer(size_t N, const char** regexes)
  {
    RE2::Options opts = config();

    for (size_t i = 0; i < N; i++)
      {
        assert(regexes[i] != nullptr);

        regex_engines.push_back(std::make_unique<re2::RE2>(regexes[i], opts));
      }

    set_of_all_regexes = set_from_regex_table(N, regexes);
  }

  bool valid()
  {
    for (size_t i = 0; i < regex_engines.size(); i++)
      {
        if (regex_engines[i] == nullptr)
          {
            return false;
          }
      }
    if (set_of_all_regexes == nullptr)
      {
        return false;
      }

    return true;
  }

  static RE2::Options config()
  {
    RE2::Options opts;
    opts.set_dot_nl(true);
    opts.set_log_errors(false);
    return opts;
  }

  static std::unique_ptr<RE2::Set> set_from_regex_table(size_t N,
                                                        const char** regexes)

  {
    RE2::Options opts = config();
    // maybe never-capture for the set object
    std::unique_ptr<RE2::Set> set =
        std::make_unique<RE2::Set>(opts, RE2::ANCHOR_START);

    {
      std::string err;
      for (size_t i = 0; i < N; i++)
        {
          int rc = set->Add(regexes[i], &err);
          if (rc == -1)
            {
              fprintf(stderr, "Cannot parse regex[%zu] \"%s\": %s\n", i,
                      regexes[i], err.c_str());
              return nullptr;
            }
          assert(rc >= 0 && ((size_t)rc == i));
        }
    }

    if (!set->Compile())
      {
        fprintf(stderr, "Out of memory on Set::Compile\n");
        return nullptr;
      }

    return set;
  }
};

#ifndef NDEBUG
static_assert(offsetof(re2_lexer, engine) == 0, "");
#endif
}  // namespace

static void check_engine(re2_lexer* l)
{
#ifndef NDEBUG
  if (l != NULL)
    {
      unsigned num = l->engine;
      if (num != lexer_engines_re2)
        {
          fprintf(stderr, "Invalid magic number (%u) for lexer.re2, aborting\n",
                  num);
          abort();
        }
    }
#else
  (void)l;
#endif
}

static lexer_t from_re2(re2_lexer* l)
{
  check_engine(l);
  return (lexer_t){.data = (void*)l};
}

static re2_lexer* to_re2(lexer_t l)
{
  re2_lexer* lexer = (re2_lexer*)l.data;
  check_engine(lexer);
  return lexer;
}

lexer_t lexer_re2_create(size_t N, const char** regexes)
{
  re2_lexer* lexer = new re2_lexer(N, regexes);
  assert(lexer);  // todo: look up how to get null out of new instead of an
                  // exception
  return from_re2(lexer);
}

void lexer_re2_destroy(lexer_t lex)
{
  if (lex.data)
    {
      re2_lexer* l = to_re2(lex);
      delete l;
    }
}

bool lexer_re2_valid(lexer_t lex)
{
  if (lex.data)
    {
      re2_lexer* l = to_re2(lex);
      return l->valid();
    }
  else
    {
      return false;
    }
}


lexer_token_t lexer_re2_iterator_step(lexer_t lex, lexer_iterator_t* iter)
{
  re2_lexer* lexer = to_re2(lex);
  lexer_token_t failure = {.id = 0, .value = "", .width = 0};

  const char* start = iter->cursor;
  const char* end = iter->end;

  std::vector<int> matches;
  re2::StringPiece cursor(start, end - start);
  RE2::Set::ErrorInfo err;
  if (!lexer->set_of_all_regexes->Match(cursor, &matches, &err))
    {
      fprintf(stderr, "Match failed (%u)\n", err.kind);
      return failure;
    }

  // Unknown is always going to match and is token zero
  std::sort(matches.begin(), matches.end());
  assert(matches.size() > 0);
  if (matches.size() == 1)
    {
      // Not generally a good sign
      assert(matches[0] == TOKEN_ID_UNKNOWN);
    }

  // Kind of messy rearranging. Want the lowest index other than
  // unknown, unless only unknown hits.
  assert(matches[0] == TOKEN_ID_UNKNOWN);
  int winning = matches.size() > 1 ? matches[1] : matches[0];
  if (winning < 0)
    {
      return failure;
    }
  size_t size_before = cursor.size();

  // Original game plan was:
  // std::string var;
  // bool C = RE2::Consume(&cursor, regexes[winning], &var);
  // However that isn't working on escaped regexes, e.g. trying to match +

  size_t token_length = 0;

  bool C;
  {
    // TODO: This here is dubious. There are N regex strings and building a
    // RE2 instance for one is probably expensive, should build the N RE2
    // instances in the constructor.
    re2::StringPiece submatch[1];
    // Reproducibly quicker to reuse previously build regex instances
    // instead of building from scratch, probably worth the complexity.
    // Would be much simpler if the C interface is dropped, which is
    // probably a reasonable improvement in itself.
    re2::RE2 const& tmp = *(lexer->regex_engines[winning]);
    C = tmp.Match(cursor, 0, cursor.size(), RE2::ANCHOR_START, submatch, 1);
    if (verbose) printf("Match %d ret %s\n", winning, (C ? "true" : "false"));
    if (C)
      {
        token_length = submatch[0].size();
      }
    cursor.remove_prefix(token_length);
  }

  if (verbose && !C)
    {
      printf("Consume returned false\n");
    }

  size_t size_after = cursor.size();

  if (size_after >= size_before)
    {
      fprintf(stderr, "Cursor size did not decrease, %zu -> %zu\n", size_before,
              size_after);
      exit(1);
    }
  else
    {
      if (verbose)
        printf("Cursor size decreased, %zu -> %zu\n", size_before, size_after);
    }

  assert((start + token_length) <= end);

  // token needs a pointer into the original buffer
  lexer_token_t res = {.id = static_cast<size_t>(winning),
                       .value = start,
                       .width = token_length};
  start += token_length;

  *iter = {.cursor = start, .end = end};
  return res;
}

#endif
