
// test regex [text [buflen [opts]]]

#include <reflex/pattern.h>
#include <reflex/matcher.h>
#include <reflex/stdmatcher.h>
#include <reflex/boostmatcher.h>
#include <sstream>

int main(int argc, char **argv)
{
  if (argc > 1)
  {
    const char *opts;
    if (argc > 4)
      opts = argv[4];
    else
      opts = "";
    if (argc > 2)
    {
      printf("\n** Boost.Regex POSIX mode test\n");
      std::string regex = reflex::regroup(argv[1]);
      printf("\n** using regex regroup = %s\n\n", regex.c_str());
      boost::regex boost_pattern(regex, boost::regex_constants::no_empty_expressions);
      reflex::BoostPosixMatcher boostmatcher(boost_pattern, argv[2], opts);
      if (argc > 3)
        boostmatcher.buffer(strtoul(argv[3], NULL, 10));
      if (!boostmatcher.matches())
        printf("No match\n");
      else
        printf("Match\n");
      boostmatcher.input(argv[2]);
      if (argc > 3)
        boostmatcher.buffer(strtoul(argv[3], NULL, 10));
      while (boostmatcher.scan())
        printf("Scan %zu '%s'\n", boostmatcher.accept(), boostmatcher.text());
      boostmatcher.input(argv[2]);
      if (argc > 3)
        boostmatcher.buffer(strtoul(argv[3], NULL, 10));
      while (boostmatcher.find())
        printf("Find %zu '%s' at %zu,%zu spans %zu..%zu %s\n", boostmatcher.accept(), boostmatcher.text(), boostmatcher.lineno(), boostmatcher.columno(), boostmatcher.first(), boostmatcher.last(), boostmatcher.at_end() ? "at end" : "");
      boostmatcher.input(argv[2]);
      if (argc > 3)
        boostmatcher.buffer(strtoul(argv[3], NULL, 10));
      while (boostmatcher.split())
        printf("Split %zu '%s' at %zu\n", boostmatcher.accept(), boostmatcher.text(), boostmatcher.columno());

      printf("\n** C++11 std::regex ECMA test (not regrouped)\n\n");
      reflex::StdMatcher stdmatcher(argv[1], argv[2], opts);
      if (argc > 3)
        stdmatcher.buffer(strtoul(argv[3], NULL, 10));
      if (!stdmatcher.matches())
        printf("No match\n");
      else
        printf("Match\n");
      stdmatcher.input(argv[2]);
      if (argc > 3)
        stdmatcher.buffer(strtoul(argv[3], NULL, 10));
      while (stdmatcher.scan())
        printf("Scan %zu '%s'\n", stdmatcher.accept(), stdmatcher.text());
      stdmatcher.input(argv[2]);
      if (argc > 3)
        stdmatcher.buffer(strtoul(argv[3], NULL, 10));
      while (stdmatcher.find())
        printf("Find %zu '%s' at %zu,%zu spans %zu..%zu %s\n", stdmatcher.accept(), stdmatcher.text(), stdmatcher.lineno(), stdmatcher.columno(), stdmatcher.first(), stdmatcher.last(), stdmatcher.at_end() ? "at end" : "");
      stdmatcher.input(argv[2]);
      if (argc > 3)
        stdmatcher.buffer(strtoul(argv[3], NULL, 10));
      while (stdmatcher.split())
        printf("Split %zu '%s' at %zu\n", stdmatcher.accept(), stdmatcher.text(), stdmatcher.columno());
    }
    printf("\n** Reflex regex = %s\n\n", argv[1]);
    reflex::Pattern reflex_pattern(argv[1], "mrw;o;f=dump.gv,dump.cpp");
    if (argc > 2)
    {
      reflex::Matcher matcher(reflex_pattern, argv[2], opts);
      if (argc > 3)
        matcher.buffer(strtoul(argv[3], NULL, 10));
      if (!matcher.matches())
        printf("No match\n");
      else
        printf("Match\n");
      matcher.input(argv[2]);
      if (argc > 3)
        matcher.buffer(strtoul(argv[3], NULL, 10));
      while (matcher.scan())
        printf("Scan %zu '%s'\n", matcher.accept(), matcher.text());
      matcher.input(argv[2]);
      if (argc > 3)
        matcher.buffer(strtoul(argv[3], NULL, 10));
      while (matcher.find())
        printf("Find %zu '%s' at %zu,%zu spans %zu..%zu %s\n", matcher.accept(), matcher.text(), matcher.lineno(), matcher.columno(), matcher.first(), matcher.last(), matcher.at_end() ? "at end" : "");
      matcher.input(argv[2]);
      if (argc > 3)
        matcher.buffer(strtoul(argv[3], NULL, 10));
      while (matcher.split())
        printf("Split %zu '%s' at %zu\n", matcher.accept(), matcher.text(), matcher.columno());
    }
  }
  else
  {
    fprintf(stderr, "Usage: test regex [text] [buflen] [\"ANT\"]]\n\n");
  }
  return 0;
}
