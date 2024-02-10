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

static size_t ith_regex_matches_start(re2_lexer* lexer, const char* start,
                                      const char* end, size_t i)
{
  re2::StringPiece cursor(start, end - start);
    re2::StringPiece submatch[1];  
    re2::RE2 const& tmp = *(lexer->regex_engines[i]);
    bool C = tmp.Match(cursor, 0, cursor.size(), RE2::ANCHOR_START, submatch, 1);
    if (!C)
      {
        return 0;
      }

    return submatch[0].size();  
}


lexer_token_t lexer_re2_iterator_step(lexer_t lex, lexer_iterator_t* iter)
{
  // Does an initial call to check all the regex at the same time
  // then works out which of the matches is the longest/earliest
  
  re2_lexer* lexer = to_re2(lex);
  lexer_token_t failure = {.id = 0, .value = "", .width = 0};

  const char* start = iter->cursor;
  const char* end = iter->end;

  std::vector<int> matches;
  re2::StringPiece cursor(start, end - start);
  RE2::Set::ErrorInfo err;
  if (!lexer->set_of_all_regexes->Match(cursor, &matches, &err))
    {
      // Should be unreachable, regex[0] is '.'
      fprintf(stderr, "Match failed (%u)\n", err.kind);
      return failure;
    }

  // Matches aren't necessarily in order
  std::sort(matches.begin(), matches.end());

  // Unknown is always going to match and is token zero
  assert(matches.size() > 0);
  assert(matches[0] == TOKEN_ID_UNKNOWN);

  lexer_token_t result = {
    .id = TOKEN_ID_UNKNOWN,
    .value = start,
    .width = 1,
  };
    
  for (int i : matches) {
    assert(i >= 0);
    if (i == TOKEN_ID_UNKNOWN) continue;
    assert((size_t)i < lexer->regex_engines.size());
    
    bool is_first_match = result.id == TOKEN_ID_UNKNOWN;

    size_t w = ith_regex_matches_start(lexer, start, end, (size_t)i);
    
    if (w == 0)
      {
        fprintf(stderr, "regex %d matched the first time and not the second\n", i);
        continue;
      }

    bool is_longer_match = w > result.width;

    if (is_first_match | is_longer_match) {
      result.id = i;
      result.width = w;
    }
  }

  assert(result.value == start);
  assert(result.value == iter->cursor);
  iter->cursor += result.width;
  return result;  
}

#endif
