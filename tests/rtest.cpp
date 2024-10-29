
// To use lazy optional ?? in strings, trigraphs should be disabled or we
// simply use ?\?
// Or disable trigraphs by enabling the GNU standard:
// c++ -std=gnu++11 -Wall test.cpp pattern.cpp matcher.cpp

#include <reflex/matcher.h>

// #define INTERACTIVE // for interactive mode testing

void banner(const char *title)
{
  int i;
  printf("\n\n/");
  for (i = 0; i < 78; i++)
    putchar('*');
  printf("\\\n *%76s*\n * %-75s*\n *%76s*\n\\", "", title, "");
  for (i = 0; i < 78; i++)
    putchar('*');
  printf("/\n\n");
}

static void error(const char *text)
{
  std::cout << "FAILED: " << text << std::endl;
  exit(EXIT_FAILURE);
}

using namespace reflex;

class WrappedMatcher : public Matcher {
 public:
  WrappedMatcher() : Matcher(), source(0)
  { }
 private:
  virtual bool wrap()
  {
    switch (source++)
    {
      case 0: in = "Hello World!";
              return true;
      case 1: in = "How now brown cow.";
              return true;
      case 2: in = "An apple a day.";
              return true;
    }
    return false;
  }
  int source;
};

struct Test {
  const char *pattern;
  const char *popts;
  const char *mopts;
  const char *cstring;
  size_t accepts[32];
};

Test tests[] = {
  { "ab", "", "", "ab", { 1 } },
  { "ab", "", "", "abab", { 1, 1 } },
  { "ab|xy", "", "", "abxy", { 1, 2 } },
  { "a(p|q)z", "", "", "apzaqz", { 1, 1 } },
  // DFA edge compaction test
  { "[a-cg-ik]z|d|[e-g]|j|y|[x-z]|.|\\n", "", "", "azz", { 1, 6 } },
  // POSIX character classes
  {
    "[[:ASCII:]]-"
    "[[:space:]]-"
    "[[:xdigit:]]-"
    "[[:cntrl:]]-"
    "[[:print:]]-"
    "[[:alnum:]]-"
    "[[:alpha:]]-"
    "[[:blank:]]-"
    "[[:digit:]]-"
    "[[:graph:]]-"
    "[[:lower:]]-"
    "[[:punct:]]-"
    "[[:upper:]]-"
    "[[:word:]]", "", "", "\x7E-\r-F-\x01-&-0-A-\t-0-#-l-.-U-_", { 1 } },
  {
    "\\p{ASCII}-"
    "\\p{Space}-"
    "\\p{XDigit}-"
    "\\p{Cntrl}-"
    "\\p{Print}-"
    "\\p{Alnum}-"
    "\\p{Alpha}-"
    "\\p{Blank}-"
    "\\p{Digit}-"
    "\\p{Graph}-"
    "\\p{Lower}-"
    "\\p{Punct}-"
    "\\p{Upper}-"
    "\\p{Word}", "", "", "\x7E-\r-F-\x01-&-0-A-\t-0-#-l-.-U-_", { 1 } },
  { "[\\s]-"
    "[\\cA-\\cZ\\x1b-\\x1f\\x7f]-"
    "[\\d]-"
    "[\\l]-"
    "[\\u]-"
    "[\\w]", "", "", "\r-\x01-0-l-U-_", { 1 } },
  // Pattern option e
  { "%(%x41%xFF%)", "e=%", "", "(A\xFF)", { 1 } },
  // Pattern option q
  { "\"(^|$)\\\"\\.+\"", "q", "", "(^|$)\"\\.+", { 1 } },
  { "(?q:\"(^|$)\\\"\\.+\")", "", "", "(^|$)\"\\.+", { 1 } },
  { "\\Q(^|$)\"\\.+\\E", "", "", "(^|$)\"\\.+", { 1 } },
  // Pattern option i
  { "(?i:abc)", "", "", "abcABC", { 1, 1 } },
  { "(?i)abc|xyz", "", "", "abcABCxyzXYZ", { 1, 1, 2, 2 } },
  { "(?i:abc)|xyz", "", "", "abcABCxyz", { 1, 1, 2 } },
  { "(?i:abc)|(?i:xyz)", "", "", "abcABCxyzXYZ", { 1, 1, 2, 2 } },
  { "(?i)abc|(?-i:xyz)|(?-i:XYZ)", "", "", "abcABCxyzXYZ", { 1, 1, 2, 3 } },
  { "(?i:abc(?-i:xyz))|ABCXYZ", "", "", "abcxyzABCxyzABCXYZ", { 1, 1, 2 } },
  { "(?i)aaa|abc|aac|def", "", "", "aaaabcaacdefAAAABCAACDEF", { 1, 2, 3, 4, 1, 2, 3, 4 } },
  { "(?i)aaa|abc|aac?|aaad?", "", "", "aaaabcaacaaadAAAABCAACAAAD", { 1, 2, 3, 4, 1, 2, 3, 4 } },
  // Pattern option x
  { "(?x) a\tb\n c | ( xy ) z ?", "", "", "abcxy", { 1, 2 } },
  { "(?x: a b\n c)", "", "", "abc", { 1 } },
  { "(?x) a b c\n|\n# COMMENT\n x y z", "", "", "abcxyz", { 1, 2 } },
  { "(?# test option (?x:... )(?x: a b c)|x y z", "", "", "abcx y z", { 1, 2 } },
  // Pattern option s
  { "(?s).", "", "", "a\n", { 1, 1 } },
  { "(?s:.)", "", "", "a\n", { 1, 1 } },
  { "(?s).", "", "", "a\n", { 1, 1 } },
  // Anchors \A, \z, ^, and $
  { "\\Aa\\z", "", "", "a", { 1 } },
  { "^a$", "", "", "a", { 1 } },
  { "(?m)^a$|\\n", "m", "", "a\na", { 1, 2, 1 } },
  { "(?m)^a|a$|a|\\n", "m", "", "aa\naaa", { 1, 2, 4, 1, 3, 2 } },
  { "(?m)\\Aa\\z|\\Aa|a\\z|^a$|^a|a$|a|^ab$|^ab|ab$|ab|\\n", "m", "", "a\na\naa\naaa\nab\nabab\nababab\na", { 2, 12, 4, 12, 5, 6, 12, 5, 7, 6, 12, 8, 12, 9, 10, 12, 9, 11, 10, 12, 3 } },
  // Optional X?
  { "a?z", "", "", "azz", { 1, 1 } },
  // Closure X*
  { "a*z", "", "", "azaazz", { 1, 1, 1 } },
  // Positive closure X+
  { "a+z", "", "", "azaaz", { 1, 1 } },
  // Combi ? * +
  { "a?b+|a", "", "", "baba", { 1, 1, 2 } },
  { "a*b+|a", "", "", "baabaa", { 1, 1, 2, 2 } },
  // Iterations {n,m}
  { "ab{2}", "", "", "abbabb", { 1, 1 } },
  { "ab{2,3}", "", "", "abbabbb", { 1, 1 } },
  { "ab{2,}", "", "", "abbabbbabbbb", { 1, 1, 1 } },
  { "ab{0,}", "", "", "a", { 1 } },
  { "(ab{0,2}c){2}", "", "", "abbcacabcabc", { 1, 1 } },
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
  { "(?m)^(a|b)+?a", "m", "", "abba", { 1 } }, // TODO can starting anchors invalidate lazy quantifiers?
  { "(?m)(a|b)+?a$", "m", "", "abba", { 1 } }, // OK ending anchors at & lazy quantifiers
  // Lazy iterations {n,m}
  { "(a|b){0,3}?aaa", "", "", "baaaaaa", { 1, 1 } },
  { "(a|b){1,3}?aaa", "", "", "baaaaaaa", { 1, 1 } },
  { "(a|b){1,3}?aaa", "", "", "bbbaaaaaaa", { 1, 1 } },
  { "(ab|cd){0,3}?ababab", "", "", "cdabababababab", { 1, 1 } },
  { "(ab|cd){1,3}?ababab", "", "", "cdababababababab", { 1, 1 } },
  { "(a|b){1,}?a|a", "", "", "bbaaa", { 1, 1 } },
  { "(a|b){2,}?a|aa", "", "", "bbbaaaa", { 1, 1 } },
  // Lazy misc tests
  { "(c[ab]*)*?cb|bb", "", "", "caaabcabcbbbcbcb", { 1, 2, 1, 1 } },
  { "(c[ab]*)[abc]*?cb|bb", "", "", "caaabcabcbbb", { 1, 2 } },
  { "((a|b)??b)*", "", "", "ababab", { 1 } },
  { "((a|b)*?b)*", "", "", "abaaab", { 1 } },
  { "((a|b)+?b)*", "", "", "bbaaab", { 1 } },
  { "((a|b)??b)+", "", "", "babbab", { 1 } },
  { "((a|b)*?b)+", "", "", "baabb", { 1 } },
  { "((a|b)+?)?", "", "", "abb", { 1, 1, 1 } },
  // Bracket lists
  { "[a-z]", "", "", "abcxyz", { 1, 1, 1, 1, 1, 1 } },
  { "[a-d-z]", "", "", "abcd-z", { 1, 1, 1, 1, 1, 1 } },
  { "[-z]", "", "", "-z", { 1, 1 } },
  { "[z-]", "", "", "-z", { 1, 1 } },
  { "[--z]", "", "", "-az", { 1, 1, 1 } },
  { "[ --]", "", "", " +-", { 1, 1, 1 } },
  { "[^a-z]", "", "", "A", { 1 } },
  { "[[:alpha:]]", "", "", "abcxyz", { 1, 1, 1, 1, 1, 1 } },
  { "[\\p{Alpha}]", "", "", "abcxyz", { 1, 1, 1, 1, 1, 1 } },
  { "[][]", "", "", "[]", { 1, 1 } },
  // Lookahead
  { "a(?=bc)|ab(?=d)|bc|d", "", "", "abcdabd", { 1, 3, 4, 2, 4 } },
  { "ab|a(?=[ab])", "", "", "abaab", { 1, 2, 1 } },
  { "a(a|b)?(?=a)|a", "", "", "aba", { 1, 2 } }, // Ambiguous, undefined in POSIX
  { "zx*(?=xy*)|x?y*", "", "", "zxxy", { 1, 2 } }, // Ambiguous, undefined in POSIX
  // { "[ab]+(?=ab)|-|ab", "", "", "aaab-bbab", { 1, 3, 2, 1, 3 } }, // Ambiguous, undefined in POSIX
  { "(?m)a(?=b?)|bc", "m", "", "aabc", { 1, 1, 2 } },
  { "(?m)a(?=\\nb)|a|^b|\\n", "m", "", "aa\nb\n", { 2, 1, 4, 3, 4 } },
  { "(?m)^a(?=b$)|b|\\n", "m", "", "ab\n", { 1, 2, 3 } },
  { "(?m)a(?=\n)|a|\\n", "m", "", "aa\n", { 2, 1, 3 } },
  { "(?m)^( +(?=a)|b)|a|\\n", "m", "", " a\n  a\nb\n", { 1, 2, 3, 1, 2, 3, 1, 3 } },
  { "abc(?=\\w+|(?^def))|xyzabcdef", "", "", "abcxyzabcdef", { 1, 2 } },
  // Negative patterns and option A (all)
  { "(?^ab)|\\w+| ", "", "", "aa ab abab ababba", { 2, 3, 3, 2, 3, 2 } },
  { "(?^ab)|\\w+| ", "", "A", "aa ab abab ababba", { 2, 3, reflex::Matcher::Const::REDO, 3, 2, 3, 2 } },
  { "\\w+|(?^ab)| ", "", "", "aa ab abab ababba", { 1, 3, 3, 1, 3, 1 } }, // non-reachable warning is given, but works
  { "\\w+|(?^\\s)", "", "", "99 Luftballons", { 1, 1 } },
  { "(\\w+|(?^ab(?=\\w*)))| ", "", "", "aa ab abab ababba", { 1, 2, 2, 2, 1 } },
  { "(?^ab(?=\\w*))|\\w+| ", "", "", "aa ab abab ababba", { 2, 3, 3, 3, 2 } },
  // Word boundaries \<, \>, \b, and \B
  { "\\<a\\>|\\<a|a\\>|a|-", "", "", "a-aaa", { 1, 5, 2, 4, 3 } },
  { "\\<.*\\>", "", "", "abc def", { 1 } },
  { "\\<.*\\>|-", "", "", "abc-", { 1, 2 } },
  { "\\b.*\\b|-", "", "", "abc-", { 1, 2 } },
  { "-|\\<.*\\>", "", "", "-abc-", { 1, 2, 1 } },
  { "-|\\b.*\\b", "", "", "-abc-", { 1, 2, 1 } },
  { "\\<(-|a)(-|a)\\>| ", "", "", "aa aa", { 1, 2, 1 } },
  { "\\b(-|a)(-|a)\\b| ", "", "", "aa aa", { 1, 2, 1 } },
  { "\\B(-|a)(-|a)\\B|b|#", "", "", "baab#--#", { 2, 1, 2, 3, 1, 3 } },
  { "\\<.*ab\\>|[ab]*|-|\\n", "", "", "-aaa-aaba-aab-\n-aaa", { 3, 1, 3, 4, 3, 2 } },
  { "a.*\\bbb.*\\b", "", "", "a--bb--cc--bb", { 1 } },
  { "[ab]-\\<([bc]|\\<c)|c", "", "", "a-bc", { 1, 2 } },
  { "a|(\\Bb?)*c", "", "", "abc", { 1, 2 } },
  { "-\\b(-|a)(-|a)\\b", "", "", "-aa", { 1 } },
  { "a\\b(-|a)(-|a)\\b", "", "", "a-a", { 1 } },
  { "a?\\>(-|a)(-|a)\\b-a", "", "", "a-a-a", { 1 } },
  { "\\b(-|a)(-|a)\\bz?| ", "", "", "aa a-z", { 1, 2, 1 } },
  { "(-|a)(-|a)\\bz?| ", "", "", "aa a-z", { 1, 2, 1 } },
  { "a?\\b(-|a)(-|a)\\b|b", "", "", "a-ba-a", { 1, 2, 1 } },
  // Indent and matcher option T (Tab)
  { "(?m)^[ \\t]+|[ \\t]+\\i|[ \\t]*\\j|a|[ \\n]", "m", "", "a\n  a\n  a\n    a\n", { 4, 5, 2, 4, 5, 1, 4, 5, 2, 4, 5, 3, 3 } },
  { "(?m)^[ \\t]+|^[ \\t]*\\i|^[ \\t]*\\j|\\j|a|[ \\n]", "m", "", "a\n  a\n  a\n    a\n", { 5, 6, 2, 5, 6, 1, 5, 6, 2, 5, 6, 4, 4 } },
  { "(?m)^[ \\t]+|[ \\t]*\\i|[ \\t]*\\j|a|[ \\n]", "m", "", "a\n  a\n  a\n    a\na\n", { 4, 5, 2, 4, 5, 1, 4, 5, 2, 4, 5, 3, 3, 4, 5 } },
  { "(?m)^[ \\t]+|[ \\t]*\\i|[ \\t]*\\j|a|[ \\n]", "m", "", "a\n  a\n  a\n    a\n  a\na\n", { 4, 5, 2, 4, 5, 1, 4, 5, 2, 4, 5, 3, 4, 5, 3, 4, 5 } },
  { "(?m)^[ \\t]+|[ \\t]*\\i|[ \\t]*\\j|a|[ \\n]", "m", "T=2", "a\n  a\n\ta\n    a\n\ta\na\n", { 4, 5, 2, 4, 5, 1, 4, 5, 2, 4, 5, 3, 4, 5, 3, 4, 5 } },
  { "(?m)^[ \\t]+|[ \\t]*\\i|[ \\t]*\\j|a|(?^[ \\n])", "m", "", "a\n\n  a\n\n  a\n\n    a\n\n  a\na\n", { 4, 2, 4, 1, 4, 2, 4, 3, 4, 3, 4 } },
  { "(?m)[ \\t]*\\i|^[ \\t]+|[ \\t]*\\j|a|(?^[ \\n])", "m", "", "a\n  a\n  a\n    a\n  a\na\n", { 4, 1, 4, 2, 4, 1, 4, 3, 4, 3, 4 } },
  // { "(?m)[ \\t]*\\ia|^[ \\t]+|[ \\t]*\\ja|[ \\t]*\\j|a|[ \\n]", "m", "", "a\n  a\na\n", { 5, 6, 1, 6, 3, 6 } }, \\ \i and \j must be at pattern ends (like $)
  { "(?m)_*\\i|^_+|_*\\j|\\w|(?^[ \\n])", "m", "", "a\n__a\n__a\n____a\n__a\na\n", { 4, 1, 4, 2, 4, 1, 4, 3, 4, 3, 4 } },
  { "(?m)[ \\t]*\\i|^[ \\t]+|[ \\t]*\\j|a|[ \\n]|(?^^[ \\t]*#\n)", "m", "", "a\n  a\n    #\n  a\n    a\n#\n  a\na\n", { 4, 5, 1, 4, 5, 2, 4, 5, 1, 4, 5, 3, 4, 5, 3, 4, 5 } },
  { "[ \\t]*\\i|^[ \\t]+|[ \\t]*\\j|a|[ \\n]|(?^\\\\\n[ \\t]+)", "m", "", "a\n  a\n  a\\\n      a a\n    a\n  a\na\n", { 4, 5, 1, 4, 5, 2, 4, 4, 5, 4, 5, 1, 4, 5, 3, 4, 5, 3, 4, 5 } },
  // { "(?m)[ \\t]*\\i|^[ \\t]+|[ \\t]*\\j|a|[ \\n]|(?^\\\\\n[ \\t]*)", "m", "", "a\n  a\n  a\\\na\n    a\n  a\na\n", { 4, 5, 1, 4, 5, 2, 4, 4, 5, 1, 4, 5, 2, 3, 4, 5, 3, 4, 5 } }, // TODO line continuation stopping at left margin triggers dedent
  // Unicode or UTF-8 (TODO: requires a flag and changes to the parser so that UTF-8 multibyte chars are parsed as ONE char)
  { "(©)+", "", "", "©", { 1 } },
  { NULL, NULL, NULL, NULL, { } }
};

int main()
{
  banner("PATTERN TESTS");
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
      if (!matcher.at_end())
        printf("ERROR: remaining input rest = '%s'; dumping dump.gv and dump.cpp\n", matcher.rest());
      else
        printf("ERROR: accept = %zu text = '%s'; dumping dump.gv and dump.cpp\n", matcher.accept(), matcher.text());
      std::string options(test->popts);
      options.append(";f=dump.gv,dump.cpp");
      Pattern(test->pattern, options);
      exit(1);
    }
    printf("OK\n\n");
  }
  Pattern pattern1("\\w+|\\W", "f=dump.cpp");
  Pattern pattern2("\\<.*\\>", "f=dump.gv");
  Pattern pattern3(" ");
  Pattern pattern4("[ \\t]+");
  Pattern pattern5("\\b", "f=dump.gv,dump.cpp");
  Pattern pattern6("");
  Pattern pattern7("[[:alpha:]]");
  Pattern pattern8("\\w+");
  Pattern pattern9(Matcher::convert("(?u:\\p{L})"));

  Matcher matcher(pattern1);
  std::string test;
  //
  banner("TEST FIND");
  //
  matcher.pattern(pattern8);
  matcher.input("an apple a day");
  test = "";
  while (matcher.find())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "an/apple/a/day/")
    error("find results");
  //
  matcher.pattern(pattern5);
  matcher.reset("N");
  matcher.input("a a");
  test = "";
  while (matcher.find())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "///")
    error("find with nullable results");
  matcher.reset("");
  //
  matcher.pattern(pattern6);
  matcher.reset("N");
  matcher.input("a a");
  test = "";
  while (matcher.find())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "///")
    error("find with nullable results");
  matcher.reset("");
  //
  banner("TEST SPLIT");
  //
  matcher.pattern(pattern3);
  matcher.input("ab c  d");
  test = "";
  while (matcher.split())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "ab/c//d/")
    error("split results");
  //
  matcher.pattern(pattern3);
  matcher.input("ab c  d ");
  test = "";
  while (matcher.split())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "ab/c//d//")
    error("split results");
  //
  matcher.pattern(pattern4);
  matcher.input("ab c  d");
  test = "";
  while (matcher.split())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "ab/c/d/")
    error("split results");
  //
  matcher.pattern(pattern5);
  matcher.input("ab c  d");
  test = "";
  while (matcher.split())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "/ab/ /c/  /d//")
    error("split results");
  //
  matcher.pattern(pattern6);
  matcher.input("ab c  d");
  test = "";
  while (matcher.split())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "/a/b/ /c/ / /d//")
    error("split results");
  //
  matcher.pattern(pattern6);
  matcher.input("");
  test = "";
  while (matcher.split())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "/")
    error("split results");
  //
  matcher.pattern(pattern7);
  matcher.input("a-b");
  test = "";
  while (matcher.split())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "/-//")
    error("split results");
  //
  matcher.pattern(pattern7);
  matcher.input("a");
  test = "";
  while (matcher.split())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "//")
    error("split results");
  //
  matcher.pattern(pattern7);
  matcher.input("-");
  test = "";
  while (matcher.split())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "-/")
    error("split results");
  //
  matcher.pattern(pattern4);
  matcher.input("ab c  d");
  int n = 2; // split 2
  while (n-- && matcher.split())
    std::cout << matcher.text() << "/";
  std::cout << std::endl << "REST = " << matcher.rest() << std::endl;
  //
  banner("TEST INPUT/UNPUT");
  //
  matcher.pattern(pattern2);
  matcher.input("ab c  d");
  while (!matcher.at_end())
    std::cout << (char)matcher.input() << "/";
  std::cout << std::endl;
  //
  matcher.pattern(pattern2);
  matcher.input("ab c  d");
  test = "";
  while (true)
  {
    if (matcher.scan())
    {
      std::cout << matcher.text() << "/";
      test.append(matcher.text()).append("/");
    }
    else if (!matcher.at_end())
    {
      std::cout << (char)matcher.input() << "?/";
      test.append("?/");
    }
    else
    {
      break;
    }
  }
  std::cout << std::endl;
  if (test != "ab c  d/")
    error("input");
  //
  matcher.pattern(pattern7);
  matcher.input("ab c  d");
  test = "";
  while (true)
  {
    if (matcher.scan())
    {
      std::cout << matcher.text() << "/";
      test.append(matcher.text()).append("/");
    }
    else if (!matcher.at_end())
    {
      std::cout << (char)matcher.input() << "?/";
      test.append("?/");
    }
    else
    {
      break;
    }
  }
  std::cout << std::endl;
  if (test != "a/b/?/c/?/?/d/")
    error("input");
  //
  matcher.pattern(pattern7);
  matcher.input("ab c  d");
  matcher.unput('a');
  test = "";
  while (true)
  {
    if (matcher.scan())
    {
      std::cout << matcher.text() << "/";
      test.append(matcher.text()).append("/");
      if (*matcher.text() == 'b')
        matcher.unput('c');
    }
    else if (!matcher.at_end())
    {
      std::cout << (char)matcher.input() << "?/";
    }
    else
    {
      break;
    }
  }
  std::cout << std::endl;
  if (test != "a/a/b/c/c/d/")
    error("unput");
  //
  matcher.pattern(pattern9);
  matcher.input("ab c  d");
  matcher.wunput(L'ä');
  test = "";
  while (true)
  {
    if (matcher.scan())
    {
      std::cout << matcher.text() << "/";
      test.append(matcher.text()).append("/");
      if (*matcher.text() == 'b')
        matcher.wunput(L'ç');
    }
    else if (!matcher.at_end())
    {
      std::cout << (char)matcher.winput() << "?/";
    }
    else
    {
      break;
    }
  }
  std::cout << std::endl;
  if (test != "ä/a/b/ç/c/d/")
    error("wunput");
  //
  banner("TEST WRAP");
  //
  WrappedMatcher wrapped_matcher;
  wrapped_matcher.pattern(pattern8);
  test = "";
  while (wrapped_matcher.find())
  {
    std::cout << wrapped_matcher.text() << "/";
    test.append(wrapped_matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "Hello/World/How/now/brown/cow/An/apple/a/day/")
    error("wrap");
  //
  banner("TEST REST");
  //
  matcher.pattern(pattern8);
  matcher.input("abc def xyz");
  test = "";
  if (matcher.find())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "abc/" || strcmp(matcher.rest(), " def xyz") != 0)
    error("rest");
  //
  banner("TEST SKIP");
  //
  matcher.pattern(pattern8);
  matcher.input("abc  \ndef xyz");
  test = "";
  if (matcher.scan())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
    matcher.skip('\n');
  }
  if (matcher.scan())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
    matcher.skip('\n');
  }
  std::cout << std::endl;
  if (test != "abc/def/")
    error("skip");
  //
  matcher.input("abc  ¶def¶");
  test = "";
  if (matcher.scan())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
    matcher.skip(L'¶');
  }
  if (matcher.scan())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
    matcher.skip(L'¶');
  }
  //
  matcher.input("abc  xxydef xx");
  test = "";
  if (matcher.scan())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
    matcher.skip("xy");
  }
  if (matcher.scan())
  {
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
    matcher.skip("xy");
  }
  std::cout << std::endl;
  if (test != "abc/def/")
    error("skip");
  //
#ifdef WITH_SPAN
  banner("TEST SPAN");
  //
  matcher.pattern(pattern8);
  matcher.input("##a#b#c##\ndef##\n##ghi\n##xyz");
  test = "";
  while (matcher.find())
  {
    std::cout << matcher.span() << "/";
    test.append(matcher.span()).append("/");
  }
  std::cout << std::endl;
  if (test != "##a#b#c##/def##/##ghi/##xyz/")
    error("span");
  //
  banner("TEST LINE");
  //
  matcher.pattern(pattern8);
  matcher.input("##a#b#c##\ndef##\n##ghi\n##xyz");
  test = "";
  while (matcher.find())
  {
    std::cout << matcher.line() << "/";
    test.append(matcher.line()).append("/");
  }
  std::cout << std::endl;
  if (test != "##a#b#c##/##a#b#c##/##a#b#c##/def##/##ghi/##xyz/")
    error("line");
#endif
  //
  banner("TEST MORE");
  //
  matcher.pattern(pattern7);
  matcher.input("abc");
  test = "";
  while (matcher.scan())
  {
    std::cout << matcher.text() << "/";
    matcher.more();
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "a/ab/abc/")
    error("more");
  //
  banner("TEST LESS");
  //
  matcher.pattern(pattern1);
  matcher.input("abc");
  test = "";
  while (matcher.scan())
  {
    matcher.less(1);
    std::cout << matcher.text() << "/";
    test.append(matcher.text()).append("/");
  }
  std::cout << std::endl;
  if (test != "a/b/c/")
    error("less");
  //
  banner("TEST MATCHES");
  //
  if (Matcher("\\w+", "hello").matches()) // on the fly string matching
    std::cout << "OK";
  else
    error("match results");
  std::cout << std::endl;
  if (Matcher("\\d", "0").matches())
    std::cout << "OK";
  else
    error("match results");
  std::cout << std::endl;
  //
  matcher.pattern(pattern1);
  matcher.input("abc");
  if (matcher.matches())
    std::cout << "OK";
  else
    error("match results");
  std::cout << std::endl;
  //
  matcher.pattern(pattern2);
  matcher.input("abc");
  if (matcher.matches())
    std::cout << "OK";
  else
    error("match results");
  std::cout << std::endl;
  //
  matcher.pattern(pattern6);
  matcher.input("");
  if (matcher.matches())
    std::cout << "OK";
  else
    error("match results");
  std::cout << std::endl;
  //
  matcher.pattern(pattern2);
  matcher.input("---");
  if (!matcher.matches())
    std::cout << "OK";
  else
    error("match results");
  std::cout << std::endl;
  //
  banner("DONE");
  return 0;
}
