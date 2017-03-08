// Split URL into parts - this example requires Boost.Regex

#include <reflex/boostmatcher.h>
#include <iostream>

using namespace reflex;

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "Usage: url 'URL'" << std::endl;
    exit(EXIT_SUCCESS);
  }

  BoostMatcher re("https?://([^/]*)/?([^?#]*)", argv[1]);

  if (re.scan())
  {
    // found a partial match at start, now check if we have a domain
    if (re[1].first != NULL)
    {
      std::string domain(re[1].first, re[1].second);
      std::cout << "domain: " << domain << std::endl;

      // check of we have a path
      if (re[2].first != NULL && re[2].second != 0)
      {
        std::string path(re[2].first, re[2].second);
        std::cout << "path: " << path << std::endl;
      }
    }

    // check if we have a query string
    if (re.input() == '?')
    {
#if 0
      // 1st suggested method: split query string at '&'
      re.pattern("&");
      while (re.split())
        std::cout << "query: " << re << std::endl;
#else
      // 2nd suggested method: capture key-value pairs between the '&'
      re.pattern("([^=&]*)=?([^&]*)&?");
      while (re.scan())
        std::cout <<
          "query key: " << std::string(re[1].first, re[1].second) <<
          ", value: " << std::string(re[2].first, re[2].second) << std::endl;
#endif
    }
    else if (!re.at_end())
    {
      // no query string, must be an # anchor
      std::cout << "anchor: " << re.rest() << std::endl;
    }
  }
  else
  {
    std::cout << "Error, not a http/s URL: " << re.rest() << std::endl;
  }

  return EXIT_SUCCESS;
}
