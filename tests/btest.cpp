
#include <reflex/boostmatcher.h>

// #define INTERACTIVE // for interactive mode testing

static void banner(const char *title)
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

class WrappedMatcher : public BoostPosixMatcher {
 public:
  WrappedMatcher() : BoostPosixMatcher(), source(0)
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
  // DFA edge compaction test (only applicable to RE/flex)
  { "[a-cg-ik]z|d|[e-g]|j|y|[x-z]|.|\\n", "", "", "azz", { 1, 6 } },
  // POSIX character classes
  {
    "\\x7E-"
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
    "\\x7E-"
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
  // Pattern option i
  { "(?i:abc)", "", "", "abcABC", { 1, 1 } },
  { "(?i)abc|xyz", "", "", "abcABCxyzXYZ", { 1, 1, 2, 2 } },
  { "(?i:abc)|xyz", "", "", "abcABCxyz", { 1, 1, 2 } },
  { "(?i:abc)|(?i:xyz)", "", "", "abcABCxyzXYZ", { 1, 1, 2, 2 } },
  { "(?i)abc|(?-i:xyz)|(?-i:XYZ)", "", "", "abcABCxyzXYZ", { 1, 1, 2, 3 } },
  { "(?i:abc(?-i:xyz))|ABCXYZ", "", "", "abcxyzABCxyzABCXYZ", { 1, 1, 2 } },
  // Pattern option x
  { "(?x) a\tb\n c | ( xy ) z ?", "", "", "abcxy", { 1, 2 } },
  { "(?x: a b\n c)", "", "", "abc", { 1 } },
  { "(?x) a b c\n|\n# COMMENT\n x y z", "", "", "abcxyz", { 1, 2 } },
  // { "(?x) a b c\n|\n/* COMMENT\n*/ x y z", "", "", "abcxyz", { 1, 2 } }, // Boost does not support /*...*/
  { "(?# test option (?x:... )(?x: a b c)|x y z", "", "", "abcx y z", { 1, 2 } },
  // Pattern option s
  { "(?s).", "", "", "a\n", { 1, 1 } },
  { "(?s:.)", "", "", "a\n", { 1, 1 } },
  { "(?s).", "", "", "a\n", { 1, 1 } },
  // Anchors \A, \Z, ^, and $ (Boost.Regex matches \n, \r and \f as newlines)
  { "\\Aa\\Z", "", "", "a", { 1 } },
  { "^a$", "", "", "a", { 1 } },
  { "(?m)^a$|\\n", "m", "", "a\na", { 1, 2, 1 } },
  { "(?m)^a|a$|a|\\n", "m", "", "aa\naaa", { 1, 2, 4, 1, 3, 2 } },
#ifndef INTERACTIVE
  { "(?m)\\Aa\\Z|\\Aa|a\\Z|^a$|^a|a$|a|^ab$|^ab|ab$|ab|\\n", "m", "", "a\na\naa\naaa\nab\nabab\nababab\na", { 2, 12, 4, 12, 5, 6, 12, 5, 7, 6, 12, 8, 12, 9, 10, 12, 9, 11, 10, 12, 3 } },  // boost has a partial match bug when interactive() blk=1
#endif
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
#if 0 // Boost POSIX mode does not support lazy quantifiers
  // Lazy optional X?
  { "(a|b)?\?a", "", "", "aaba", { 1, 1, 1 } },
  { "a(a|b)?\?(?=a|ab)|ac", "", "", "aababac", { 1, 1, 1, 2 } },
  { "(a|b)?\?(a|b)?\?aa", "", "", "baaaabbaa", { 1, 1, 1 } },
  { "(a|b)?\?(a|b)?\?(a|b)?\?aaa", "", "", "baaaaaa", { 1, 1 } },
  { "a?\?b?a", "", "", "aba", { 1, 1 } }, // 'a' 'ba'
  { "a?\?b?b", "", "", "abb", { 1 } }, // 'abb'
  // Lazy closure X*
  { "a*?a", "", "", "aaaa", { 1, 1, 1, 1 } },
  { "a*?|a|b", "", "", "aab", { 2, 2, 3 } },
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
  { "a?(a|b)*?a", "", "", "aaababa", { 1, 1, 1 } },
  { "(?m)^(a|b)*?a", "m", "", "bba", { 1 } },
  { "(?m)(a|b)*?a$", "m", "", "bba", { 1 } },
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
  { "(?m)^(a|b)+?a", "m", "", "abba", { 1 } },
  { "(?m)(a|b)+?a$", "m", "", "abba", { 1 } },
  // Lazy iterations {n,m}
  { "(a|b){0,3}?aaa", "", "", "baaaaaa", { 1, 1 } },
  { "(a|b){1,3}?aaa", "", "", "baaaaaaa", { 1, 1 } },
  { "(a|b){1,3}?aaa", "", "", "bbbaaaaaaa", { 1, 1 } },
  { "(ab|cd){0,3}?ababab", "", "", "cdabababababab", { 1, 1 } },
  { "(ab|cd){1,3}?ababab", "", "", "cdababababababab", { 1, 1 } },
  { "(a|b){1,}?a|a", "", "", "bbaaa", { 1, 1 } },
  { "(a|b){2,}?a|aa", "", "", "bbbaaaa", { 1, 1 } },
#endif
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
  // { "[ab]+(?=ab)|-|ab", "", "", "aaab-bbab", { 1, 3, 2, 1, 3 } }, // has trailing context (undefined as per POSIX)
  { "(?m)a(?=b?)|bc", "m", "", "aabc", { 1, 1, 2 } },
#ifndef INTERACTIVE
  { "(?m)a(?=\\nb)|a|^b|\\n", "m", "", "aa\nb\n", { 2, 1, 4, 3, 4 } }, // boost has a partial match bug when interactive() blk=1
#endif
  { "(?m)^a(?=b$)|b|\\n", "m", "", "ab\n", { 1, 2, 3 } },
  { "(?m)a(?=\n)|a|\\n", "m", "", "aa\n", { 2, 1, 3 } },
  { "(?m)^( +(?=a)|b)|a|\\n", "m", "", " a\n  a\nb\n", { 1, 2, 3, 1, 2, 3, 1, 3 } },
  // Word boundaries \<, \>, \b, and \B
  { "\\<a\\>|\\<a|a\\>|a|-", "", "", "a-aaa", { 1, 5, 2, 4, 3 } },
  { "\\<.*ab\\>|[ab]*|-|\\n", "", "", "-aaa-aaba-aab-\n-aaa", { 3, 1, 3, 4, 3, 2 } },
#ifndef INTERACTIVE
  { "\\<.*\\>", "", "", "abc def", { 1 } }, // boost has a partial match bug when interactive() blk=1
#endif
  { "\\<.*\\>|-", "", "", "abc-", { 1, 2 } },
  { "\\b.*\\b|-", "", "", "abc-", { 1, 2 } },
  { "-|\\<.*\\>", "", "", "-abc-", { 1, 2, 1 } },
  { "-|\\b.*\\b", "", "", "-abc-", { 1, 2, 1 } },
  { "(-|a)\\<(-|a)\\>(-|a)", "", "", "-a-", { 1 } },
  { "(-|a)\\b(-|a)\\b(-|a)", "", "", "-a-a-a", { 1, 1 } },
  { "(-|a)\\B(-|a)\\B(-|a)", "", "", "---aaa", { 1, 1 } },
  { "\\<(-|a)(-|a)\\>| ", "", "", "aa aa", { 1, 2, 1 } },
  { "\\b(-|a)(-|a)\\b| ", "", "", "aa aa", { 1, 2, 1 } },
#ifndef INTERACTIVE
  { "\\B(-|a)(-|a)\\B|b|#", "", "", "baab#--#", { 2, 1, 2, 3, 1, 3 } }, // boost has a partial match bug when interactive() blk=1
#endif
  { "-\\b(-|a)(-|a)\\b", "", "", "-aa", { 1 } },
  { "a\\b(-|a)(-|a)\\b", "", "", "a-a", { 1 } },
#ifndef INTERACTIVE
  { "a?\\>(-|a)(-|a)\\b| ", "", "", "a-a-a", { 1, 1 } }, // boost has a partial match bug when interactive() blk=1 & does not check \> at start, so accepts more liberally
#endif
  { "\\b(-|a)(-|a)\\bz?| ", "", "", "aa a-z", { 1, 2, 1 } },
  { "(-|a)(-|a)\\bz?| ", "", "", "aa a-z", { 1, 2, 1 } },
  { "a?\\b(-|a)(-|a)\\b| ", "", "", "a-a", { 1 } },
#ifndef INTERACTIVE
  { "-(?=\\<a\\>)|-|a|b", "", "", "-a-ab", { 1, 3, 2, 3, 4 } }, // boost has a partial match bug when interactive() blk=1
#endif
  // Unicode
  { "(©)+", "", "", "©", { 1 } },
  { NULL, NULL, NULL, NULL, { } }
};

int main()
{
  banner("PATTERN TESTS");
  for (const Test *test = tests; test->pattern != NULL; ++test)
  {
    std::string regex;
    try
    {
      regex = BoostPosixMatcher::convert(test->pattern, convert_flag::recap);
    }
    catch (const regex_error& e)
    {
      std::cerr << e.what();
    }
    std::cout << regex << std::endl;
    boost::regex pattern(regex);
    BoostPosixMatcher matcher(pattern, test->cstring, test->mopts);
#ifdef INTERACTIVE
    matcher.interactive(); // test with blk=1
#endif
    printf("Test \"%s\" against \"%s\"\n", test->pattern, test->cstring);
    if (*test->popts)
      printf("With pattern options \"%s\"\n", test->popts);
    if (*test->mopts)
      printf("With matcher options \"%s\"\n", test->mopts);
    size_t i = 0;
    while (matcher.scan())
    {
      printf("  At %zu,%zu;[%zu,%zu]: \"%s\" matches pattern %zu\n", matcher.lineno(), matcher.columno(), matcher.first(), matcher.last(), matcher.text(), matcher.accept());
      if (matcher.accept() != test->accepts[i])
        break;
      ++i;
    }
    if (matcher.accept() != 0 || test->accepts[i] != 0 || !matcher.at_end())
    {
      if (!matcher.at_end())
        printf("ERROR: remaining input rest = '%s'\n", matcher.rest());
      else
        printf("ERROR: accept = %zu text = '%s'\n", matcher.accept(), matcher.text());
      exit(1);
    }
    printf("OK\n\n");
  }
  BoostPosixMatcher pattern1("(\\w+)|(\\W)");
  BoostPosixMatcher pattern2("\\<.*\\>");
  BoostPosixMatcher pattern3(" ");
  BoostPosixMatcher pattern4("[ \\t]+");
  BoostPosixMatcher pattern5("\\b");
  BoostPosixMatcher pattern6("");
  BoostPosixMatcher pattern7("[[:alpha:]]");
  BoostPosixMatcher pattern8("\\w+");
  BoostPosixMatcher pattern9(BoostPosixMatcher::convert("(?u:\\p{L})"));

  BoostPosixMatcher matcher(pattern1);
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
  if (BoostPosixMatcher("\\w+", "hello").matches()) // on the fly string matching
    std::cout << "OK";
  else
    error("match results");
  std::cout << std::endl;
  if (BoostPosixMatcher("\\d", "0").matches())
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
  //
  return 0;
}
