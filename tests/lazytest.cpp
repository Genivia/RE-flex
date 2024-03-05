
// To use lazy optional ?? in strings, trigraphs should be disabled or we
// simply use ?\?
// Or disable trigraphs by enabling the GNU standard:
// c++ -std=gnu++11 -Wall test.cpp pattern.cpp matcher.cpp

#include <reflex/matcher.h>

using namespace reflex;

struct Test {
  const char *pattern;
  const char *popts;
  const char *mopts;
  const char *cstring;
  size_t accepts[32];
};

Test tests[] = {
  // Lazy optional X?
  { "(a|b)?\?a", "", "", "aaba", { 1, 1, 1 } },
  { "a(a|b)?\?(?=a|ab)|ac", "", "", "aababac", { 1, 1, 1, 2 } },
  { "(a|b)?\?(a|b)?\?aa", "", "", "baaaabbaa", { 1, 1, 1 } },
  { "(a|b)?\?(a|b)?\?(a|b)?\?aaa", "", "", "baaaaaa", { 1, 1 } },
  { "a?\?b?a", "", "", "aba", { 1, 1 } }, // 'a' 'ba'
  { "a?\?b?b", "", "", "abb", { 1 } }, // 'abb'
  // Lazy closure X*
  { "a*?a", "", "", "aaaa", { 1, 1, 1, 1 } },
  { "a*?|b", "", "", "aab", { 1, 1, 2 } },
  { "(a|bb)*?abb", "", "", "abbbbabb", { 1, 1 } },
  { "ab*?|b", "", "", "ab", { 1, 2 } },
  { "(ab)*?|b", "", "", "b", { 2 } },
  { "a(ab)*?|b", "", "", "ab", { 1, 2 } },
  { "(a|b)*?a|c?", "", "", "bbaaac", { 1, 1, 1, 2 } },
  { "a(a|b)*?a", "", "", "aaaba", { 1, 1 } },
  { "a(a|b)*?a?\?|b", "", "", "aaaba", { 1, 1, 1, 2, 1 } },
  { "a(a|b)*?a?", "", "", "aa", { 1 } },
  { "a(a|b)*?a|a", "", "", "aaaba", { 1, 1 } },
  { "a(a|b)*?a|a?", "", "", "aaaba", { 1, 1 } },
  { "a(a|b)*?a|a?\?", "", "", "aaaba", { 1, 1 } },
  { "a(a|b)*?a|aa?", "", "", "aaaba", { 1, 1 } },
  { "a(a|b)*?a|aa?\?", "", "", "aaaba", { 1, 1 } },
  { "ab(ab|cd)*?ab|ab", "", "", "abababcdabab", { 1, 1, 2 } },
  { "(a|b)(a|b)*?a|a", "", "", "aaabaa", { 1, 1, 2 } },
  { "(ab|cd)(ab|cd)*?ab|ab", "", "", "abababcdabab", { 1, 1, 2 } },
  { "(ab)(ab)*?a|b", "", "", "abababa", { 1, 2, 1 } },
  { "a?(a|b)*?a", "", "", "aaababba", { 1, 1, 1, 1 } },
  { "(?m)^(a|b)*?a", "m", "", "bba", { 1 } },
  { "(?m)(a|b)*?a$", "m", "", "bba", { 1 } }, // OK: ending anchors & lazy quantifiers
  { "(a|b)*?a\\b", "", "", "bba", { 1 } }, // OK but limited: ending anchors & lazy quantifiers
  { "(?m)^(a|b)*?|b", "m", "", "ab", { 1, 2 } },
  // Lazy positive closure X+
  { "a+?a", "", "", "aaaa", { 1, 1 } },
  { "(a|b)+?", "", "", "ab", { 1, 1 } },
  { "(a|b)+?a", "", "", "bbaaa", { 1, 1 } },
  { "(a|b)+?a|c?", "", "", "bbaaa", { 1, 1 } },
  { "(ab|cd)+?ab|d?", "", "", "cdcdababab", { 1, 1 } },
  { "(ab)+?a|b", "", "", "abababa", { 1, 2, 1 } },
  { "(ab)+?ac", "", "", "ababac", { 1 } },
  { "ABB*?|ab+?|A|a", "", "", "ABab", { 1, 2 } },
  { "(a|b)+?a|a", "", "", "bbaaa", { 1, 1 } },
  { "(?m)^(a|b)+?a", "m", "", "abba", { 1 } }, // can starting anchors invalidate lazy quantifiers?
  { "(?m)(a|b)+?a$", "m", "", "abba", { 1 } }, // OK ending anchors at & lazy quantifiers
  // Lazy iterations {n,m}
  { "(a|b){0,3}?aaa", "", "", "baaaaaa", { 1, 1 } },
  { "(a|b){1,3}?aaa", "", "", "baaaaaaa", { 1, 1 } },
  { "(a|b){1,3}?aaa", "", "", "bbbaaaaaaa", { 1, 1 } },
  { "(ab|cd){0,3}?ababab", "", "", "cdabababababab", { 1, 1 } },
  { "(ab|cd){1,3}?ababab", "", "", "cdababababababab", { 1, 1 } },
  { "(a|b){1,}?a|a", "", "", "bbaaa", { 1, 1 } },
  { "(a|b){2,}?a|aa", "", "", "bbbaaaa", { 1, 1 } },
  // Lazy nestings
  { "(c[ab]*)*?cb|bb", "", "", "caaabcabcbbb", { 1, 2 } },
  { "(c[ab]*)[abc]*?cb|bb", "", "", "caaabcabcbbb", { 1, 2 } },
  { "(a+)??aaa", "", "", "aaaaaa", { 1, 1 } },
  { "((a|b)??b)*", "", "", "ababab", { 1 } },
  { "((a|b)*?b)*", "", "", "abaaab", { 1 } },
  { "((a|b)+?b)*", "", "", "bbaaab", { 1 } },
  { "((a|b)??b)?", "", "", "abbb", { 1, 1, 1 } },
  { "((a|b)*?b)?", "", "", "aabbb", { 1, 1, 1 } },
  { "((a|b)+?b)?", "", "", "baabbb", { 1, 1 } },
  { "((a|b)??b)+", "", "", "babbab", { 1 } },
  { "((a|b)*?b)+", "", "", "baabb", { 1 } },
  { "((a|b)+?b)+", "", "", "baabbb", { 1 } },
  { "((a|b)+?)?", "", "", "abb", { 1, 1, 1 } },
  { "((a|b)+?)+", "", "", "baabb", { 1 } },
  { NULL, NULL, NULL, NULL, { } }
};

int main()
{
  size_t fail = 0;
  for (const Test *test = tests; test->pattern != NULL; ++test)
  {
    Pattern pattern(test->pattern, test->popts);
    Matcher matcher(pattern, test->cstring, test->mopts);
#ifdef INTERACTIVE
    matcher.interactive();
#endif
    printf("Test \"%s\" against \"%s\"\n", test->pattern, test->cstring);
    if (*test->popts)
      printf("With pattern options \"%s\"\n", test->popts);
    if (*test->mopts)
      printf("With matcher options \"%s\"\n", test->mopts);
    for (Pattern::Index i = 1; i <= pattern.size(); ++i)
      if (!pattern.reachable(i))
        printf("WARNING: pattern[%u]=\"%s\" not reachable\n", i, pattern[i].c_str());
    size_t i = 0;
    while (matcher.scan())
    {
      printf("  At %zu,%zu;[%zu,%zu]: \"%s\" matches pattern[%zu]=\"%s\" from %u choice(s)\n", matcher.lineno(), matcher.columno(), matcher.first(), matcher.last(), matcher.text(), matcher.accept(), pattern[matcher.accept()].c_str(), pattern.size());
      if (matcher.accept() != test->accepts[i])
        break;
      ++i;
    }
    if (matcher.accept() != 0 || test->accepts[i] != 0 || !matcher.at_end())
    {
      ++fail;
      if (!matcher.at_end())
        printf("!!!! %zu ERROR: remaining input rest = '%s'; dumping dump.gv and dump.cpp\n", fail, matcher.rest());
      else
        printf("!!!! %zu ERROR: accept = %zu text = '%s'; dumping dump.gv and dump.cpp\n", fail, matcher.accept(), matcher.text());
    }
    else
    {
      printf("OK\n\n");
    }
  }
  if (fail)
    printf("\n%zu FAILED\n", fail);
  else
    printf("ALL OK\n");
  return 0;
}
