#include <regex>

int main()
{
  std::regex_constants::match_flag_type f = std::regex_constants::match_continuous | std::regex_constants::match_not_null;
  std::regex r("ab", std::regex::extended);
  const char *t = "ab";
  std::cregex_iterator i = std::cregex_iterator(t, t + strlen(t), r, f);
  return 0;
}
