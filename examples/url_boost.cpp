// Split a URL into parts: host, path, list of query key-value pairs, and #-anchor
// This example requires Boost.Regex
//
// Example:
//   url https://www.genivia.com:443/doc/reflex/html/index.html#regex
//   host: www.genivia.com
//   port: 443
//   path: doc/reflex/html/index.html
//   anchor: regex

#include <reflex/boostmatcher.h>
#include <iostream>

using namespace reflex;

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "Usage: url 'URL'" << std::endl;
    exit(EXIT_FAILURE);
  }

  // create a matcher with a regex and string input argv[1]
  BoostMatcher re("https?://([^:/]*):?(\\d*)/?([^?#]*)", argv[1]);

  // scan the input from the beginning
  if (re.scan())
  {
    // found a partial match at start, now check if we have a host
    if (re[1].first != NULL)
    {
      // capture group 1 is not empty, so we have a host
      std::string host(re[1].first, re[1].second);
      std::cout << "host: " << host << std::endl;

      // if capture group2 is not empty we have a port
      if (re[2].first != NULL && re[2].second != 0)
      {
        std::string port(re[2].first, re[2].second);
        std::cout << "port: " << port << std::endl;
      }

      // if capture group 3 is not empty we have a path
      if (re[3].first != NULL && re[3].second != 0)
      {
        std::string path(re[3].first, re[3].second);
        std::cout << "path: " << path << std::endl;
      }
    }

    // check if we have a query string, i.e. if input is now at a '?'
    if (re.input() == '?')
    {
      // now switch patterns to match the rest of the input
      // i.e. a query string or an anchor
#if 0
      // 1st method: a pattern to split query strings at '&'
      re.pattern("&");
      while (re.split())
        std::cout << "query: " << re << std::endl;
#else
      // 2nd method: a pattern to capture key-value pairs between the '&'
      re.pattern("([^=&]*)=?([^&]*)&?");
      while (re.scan())
        std::cout <<
          "query key: " << std::string(re[1].first, re[1].second) <<
          ", value: " << std::string(re[2].first, re[2].second) << std::endl;
#endif
    }
    else if (!re.at_end())
    {
      // not a query string and not the end, we expect an # anchor
      std::cout << "anchor: " << re.rest() << std::endl;
    }
  }
  else
  {
    std::cout << "Error, not a http/s URL: " << re.rest() << std::endl;
  }

  exit(EXIT_SUCCESS);
}
