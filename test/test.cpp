
// test regex [text [buflen [opts]]]

#include "pattern.h"
#include "matcher.h"
#include "boostmatcher.h"
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
      std::string regex = reflex::BoostMatcher::regroup(argv[1]);
      printf("\n** Boost regex regroup = %s\n\n", regex.c_str());
      boost::regex boost_pattern(regex, boost::regex_constants::no_empty_expressions);
      reflex::BoostMatcher matcher(boost_pattern, argv[2], opts);
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
    printf("\n** Reflex regex = %s\n\n", argv[1]);
    reflex::Pattern reflex_pattern(argv[1], "mrw;f=dump.gv,dump.cpp");
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
