
// test regex [text [buflen [opts]]]

#include <reflex/matcher.h>
#include <reflex/boostmatcher.h>
#include <reflex/stdmatcher.h>
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
      std::string regex;
      try
      {
        regex = reflex::BoostPosixMatcher::convert(argv[1], reflex::convert_flag::recap | reflex::convert_flag::unicode);
        printf("\n** using converted regex: %s\n\n", regex.c_str());
        boost::regex boost_pattern(regex, boost::regex_constants::no_empty_expressions);
        reflex::BoostPosixMatcher boostmatcher(boost_pattern, argv[2], opts);
        if (argc > 3)
          boostmatcher.buffer(strtoul(argv[3], NULL, 10));
        if (!boostmatcher.matches())
	{
          printf("No match\n");
	}
        else
	{
          printf("Match:");
	  for (size_t i = 1; boostmatcher[i].first; ++i)
	    printf(" group[%zu]=(%zu,%zu)", i, boostmatcher[i].first - boostmatcher[0].first, boostmatcher[i].second);
	  printf("\n");
	  fflush(stdout);
	  std::cout << boostmatcher << std::endl;
	}
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
      }
      catch (const reflex::regex_error& e)
      {
        std::cerr << e.what();
      }

      printf("\n** C++11 std::regex Ecma mode test\n");
      try
      {
        regex = reflex::StdMatcher::convert(argv[1], reflex::convert_flag::recap | reflex::convert_flag::unicode);
        printf("\n** using converted regex: %s\n\n", regex.c_str());
        reflex::StdMatcher stdmatcher(regex, argv[2], opts);
        if (argc > 3)
          stdmatcher.buffer(strtoul(argv[3], NULL, 10));
        if (!stdmatcher.matches())
	{
          printf("No match\n");
	}
        else
	{
          printf("Match:");
	  for (size_t i = 1; stdmatcher[i].first; ++i)
	    printf(" group[%zu]=(%zu,%zu)", i, stdmatcher[i].first - stdmatcher[0].first, stdmatcher[i].second);
	  printf("\n");
	}
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
      catch (const reflex::regex_error& e)
      {
        std::cerr << e.what();
      }
    }
    try
    {
      std::string regex = reflex::Matcher::convert(argv[1], reflex::convert_flag::unicode);
      printf("\n** RE/flex converted regex = %s\n\n", regex.c_str());
      reflex::Pattern reflex_pattern(regex, "mr;o;f=dump.gv,dump.cpp");
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
    catch (const reflex::regex_error& e)
    {
      std::cerr << e.what();
    }
  }
  else
  {
    fprintf(stderr, "Usage: test regex [text] [buflen] [\"ANT\"]]\n\n");
  }
  return 0;
}
