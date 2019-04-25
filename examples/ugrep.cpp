// ugrep.cpp
// Unicode-aware grep
//
// The following example command prints all capitalized words in any language:
//   ugrep "\p{Upper}\p{Lower}*" document.txt
//
// Features:
// - Patterns are ERE POSIX syntax compliant.
// - Unicode support for \p{} classes etc.
// - File encoding support for UTF-8/16/32, EBCDIC, and code pages.
//
// c++ -std=c++11 -o ugrep ugrep.cpp -lreflex

// TODO this is work in progress for RE/flex 1.1.6

#include <reflex/matcher.h>

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cout << "Usage: regrep [-q] pattern [file ...]" << std::endl;
    exit(0);
  }

  try
  {
    reflex::Input input;
    if (argc > 2)
    {
      const char *file = argv[2];
      input = fopen(file, "r");
      if (input.file() == NULL)
      {
        perror("Cannot open file for reading");
        exit(2);
      }
      std::cout << "Reading " << file << std::endl;
    }
    else
    {
      input = stdin;
    }
    reflex::Input::file_encoding_type encoding(reflex::Input::file_encoding::plain);
    input.file_encoding(encoding, NULL);
    reflex::convert_flag_type flag = reflex::convert_flag::unicode; // none by default
    reflex::Pattern pattern(reflex::Matcher::convert(std::string(argv[1]), reflex::convert_flag::unicode));
    for (auto& match : reflex::Matcher(pattern, input).find)
    {
      std::cout << match.text() << std::endl;
    }
    if (input.file() != stdin)
      fclose(input.file());
  }
  catch (reflex::regex_error& e)
  {
    std::cerr << e.what();
    exit(2);
  }

  exit(0);
}
