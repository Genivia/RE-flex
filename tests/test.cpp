
// test regex [text [buflen [opts]]]

#include <reflex/matcher.h>
#include <reflex/boostmatcher.h>
#include <reflex/pcre2matcher.h>
#include <reflex/stdmatcher.h>
#include <sstream>

// ALL or RE/flex only
#define ALL

int main(int argc, char **argv)
{
  if (argc > 1)
  {
    const char *opts;
    if (argc > 4)
      opts = argv[4];
    else
      opts = "";
#ifdef ALL
    if (argc > 2)
    {
      const char *text = argv[2];
      printf("\n** Boost.Regex POSIX mode test");
      std::string regex;
      try
      {
        regex = reflex::BoostPosixMatcher::convert(argv[1], reflex::convert_flag::recap | reflex::convert_flag::unicode | reflex::convert_flag::notnewline);
        printf(" with converted regex: %s\n\n", regex.c_str());
        boost::regex boost_pattern(regex, boost::regex_constants::no_empty_expressions);
        reflex::BoostPosixMatcher boostmatcher(boost_pattern, text, opts);
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
        boostmatcher.input(text);
        if (argc > 3)
          boostmatcher.buffer(strtoul(argv[3], NULL, 10));
        while (boostmatcher.scan())
          printf("Scan %zu '%s'\n", boostmatcher.accept(), boostmatcher.text());
        boostmatcher.input(text);
        if (argc > 3)
          boostmatcher.buffer(strtoul(argv[3], NULL, 10));
        while (boostmatcher.find())
          printf("Find %zu '%s' at %zu,%zu spans %zu..%zu %s\n", boostmatcher.accept(), boostmatcher.text(), boostmatcher.lineno(), boostmatcher.columno(), boostmatcher.first(), boostmatcher.last(), boostmatcher.at_end() ? "till end" : "");
        boostmatcher.input(text);
        if (argc > 3)
          boostmatcher.buffer(strtoul(argv[3], NULL, 10));
        while (boostmatcher.split())
          printf("Split %zu '%s' at %zu\n", boostmatcher.accept(), boostmatcher.text(), boostmatcher.columno());
      }
      catch (const reflex::regex_error& e)
      {
        std::cerr << e.what();
      }

      printf("\n** Boost.Regex Perl mode test");
      try
      {
        regex = reflex::BoostPerlMatcher::convert(argv[1], reflex::convert_flag::recap | reflex::convert_flag::unicode | reflex::convert_flag::notnewline);
        printf(" with converted regex: %s\n\n", regex.c_str());
        boost::regex boost_pattern(regex, boost::regex_constants::no_empty_expressions);
        reflex::BoostPerlMatcher boostmatcher(boost_pattern, text, opts);
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
        boostmatcher.input(text);
        if (argc > 3)
          boostmatcher.buffer(strtoul(argv[3], NULL, 10));
        while (boostmatcher.scan())
          printf("Scan %zu '%s'\n", boostmatcher.accept(), boostmatcher.text());
        boostmatcher.input(text);
        if (argc > 3)
          boostmatcher.buffer(strtoul(argv[3], NULL, 10));
        while (boostmatcher.find())
          printf("Find %zu '%s' at %zu,%zu spans %zu..%zu %s\n", boostmatcher.accept(), boostmatcher.text(), boostmatcher.lineno(), boostmatcher.columno(), boostmatcher.first(), boostmatcher.last(), boostmatcher.at_end() ? "at end" : "");
        boostmatcher.input(text);
        if (argc > 3)
          boostmatcher.buffer(strtoul(argv[3], NULL, 10));
        while (boostmatcher.split())
          printf("Split %zu '%s' at %zu\n", boostmatcher.accept(), boostmatcher.text(), boostmatcher.columno());
      }
      catch (const reflex::regex_error& e)
      {
        std::cerr << e.what();
      }

      printf("\n** PCRE2 Perl mode test");
      try
      {
        regex = reflex::PCRE2Matcher::convert(argv[1], reflex::convert_flag::recap | reflex::convert_flag::unicode | reflex::convert_flag::notnewline);
        printf(" with converted regex: %s\n\n", regex.c_str());
        std::string pcre2_pattern(regex);
        reflex::PCRE2Matcher pcre2matcher(pcre2_pattern, text, opts);
        if (argc > 3)
          pcre2matcher.buffer(strtoul(argv[3], NULL, 10));
        if (!pcre2matcher.matches())
	{
          printf("No match\n");
	}
        else
	{
          printf("Match:");
	  for (size_t i = 1; pcre2matcher[i].first; ++i)
	    printf(" group[%zu]=(%zu,%zu)", i, pcre2matcher[i].first - pcre2matcher[0].first, pcre2matcher[i].second);
	  printf("\n");
	  fflush(stdout);
	  std::cout << pcre2matcher << std::endl;
	}
        pcre2matcher.input(text);
        if (argc > 3)
          pcre2matcher.buffer(strtoul(argv[3], NULL, 10));
        while (pcre2matcher.scan())
          printf("Scan %zu '%s'\n", pcre2matcher.accept(), pcre2matcher.text());
        pcre2matcher.input(text);
        if (argc > 3)
          pcre2matcher.buffer(strtoul(argv[3], NULL, 10));
        while (pcre2matcher.find())
          printf("Find %zu '%s' at %zu,%zu spans %zu..%zu %s\n", pcre2matcher.accept(), pcre2matcher.text(), pcre2matcher.lineno(), pcre2matcher.columno(), pcre2matcher.first(), pcre2matcher.last(), pcre2matcher.at_end() ? "at end" : "");
        pcre2matcher.input(text);
        if (argc > 3)
          pcre2matcher.buffer(strtoul(argv[3], NULL, 10));
        while (pcre2matcher.split())
          printf("Split %zu '%s' at %zu\n", pcre2matcher.accept(), pcre2matcher.text(), pcre2matcher.columno());
      }
      catch (const reflex::regex_error& e)
      {
        std::cerr << e.what();
      }

      printf("\n** C++11 std::regex Ecma mode test");
      try
      {
        regex = reflex::StdMatcher::convert(argv[1], reflex::convert_flag::recap | reflex::convert_flag::unicode | reflex::convert_flag::notnewline);
        printf(" with converted regex: %s\n\n", regex.c_str());
        reflex::StdMatcher stdmatcher(regex, text, opts);
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
        stdmatcher.input(text);
        if (argc > 3)
          stdmatcher.buffer(strtoul(argv[3], NULL, 10));
        while (stdmatcher.scan())
          printf("Scan %zu '%s'\n", stdmatcher.accept(), stdmatcher.text());
        stdmatcher.input(text);
        if (argc > 3)
          stdmatcher.buffer(strtoul(argv[3], NULL, 10));
        while (stdmatcher.find())
          printf("Find %zu '%s' at %zu,%zu spans %zu..%zu %s\n", stdmatcher.accept(), stdmatcher.text(), stdmatcher.lineno(), stdmatcher.columno(), stdmatcher.first(), stdmatcher.last(), stdmatcher.at_end() ? "at end" : "");
        stdmatcher.input(text);
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
#endif
    try
    {
      std::string regex = reflex::Matcher::convert(argv[1], reflex::convert_flag::unicode | reflex::convert_flag::notnewline);
      printf("\n** RE/flex converted regex = %s\n", regex.c_str());
      reflex::Pattern reflex_pattern(regex, "mr;o;f=dump.gv,dump.cpp"); // include ;g; or ;gg; to show cut DFA after analysis
      printf("\nDFA parse=%zu(%g) analysis=(%g) nodes=%zu(%g) edges=%zu(%g) words=%zu(%g)\n", regex.size(), reflex_pattern.parse_time(), reflex_pattern.analysis_time(), reflex_pattern.nodes(), reflex_pattern.nodes_time(), reflex_pattern.edges(), reflex_pattern.edges_time(), reflex_pattern.words(), reflex_pattern.words_time());
      if (argc > 2)
      {
        const char *text = argv[2];
        reflex::Matcher matcher(reflex_pattern, text, opts);
        if (argc > 3)
          matcher.buffer(strtoul(argv[3], NULL, 10));
        if (!matcher.matches())
          printf("No match\n");
        else
          printf("Match %zu\n", matcher.accept());
        matcher.input(text);
        if (argc > 3)
          matcher.buffer(strtoul(argv[3], NULL, 10));
        while (matcher.scan())
          printf("Scan %zu '%s'\n", matcher.accept(), matcher.text());
        matcher.input(text);
        if (argc > 3)
          matcher.buffer(strtoul(argv[3], NULL, 10));
        while (matcher.find())
          printf("Find %zu '%s' at %zu,%zu spans %zu..%zu %s\n", matcher.accept(), matcher.text(), matcher.lineno(), matcher.columno(), matcher.first(), matcher.last(), matcher.at_end() ? "at end" : "");
        matcher.input(text);
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
