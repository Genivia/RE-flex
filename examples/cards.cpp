#include <reflex/pattern.h>
#include <reflex/matcher.h>

static reflex::Pattern card_patterns(
  "(?# MasterCard)(5[1-5]\\d{14})|"                    // 1st group = MC
  "(?# Visa)(4\\d{12}(?:\\d{3})?)|"                    // 2nd group = VISA
  "(?# AMEX)(3[47]\\d{13})|"                           // 3rd group = AMEX
  "(?# Discover)(6011\\d{14})|"                        // 4th group = Discover
  "(?# Diners Club)((?:30[0-5]|36\\d|38\\d)\\d{11})"); // 5th group = Diners

static const char *card_data =
  "mark 5212345678901234\n"
  "vinny 4123456789012\n"
  "victor 4123456789012345\n"
  "amy 371234567890123\n"
  "dirk 601112345678901234\n"
  "doc 38812345678901 end\n";

void get_card_numbers(void)
{
  std::vector<std::string> cards[5];
  std::cout << "Patterns (line,col);[first,last]:" << std::endl;

  // C++11 we can use a range based loop that is much simpler:
  // for (auto& match : reflex::Matcher(card_patterns, card_data).find)
  reflex::Matcher matcher(card_patterns, card_data);
  for (reflex::Matcher::iterator match = matcher.find.begin(); match != matcher.find.end(); ++match)
  {
    cards[match->accept() - 1].push_back(match->text());
    std::cout
      << match->lineno()
      << ","
      << match->columno()
      << ";["
      << match->first()
      << ","
      << match->last()
      << "]: "
      << match->text()
      << std::endl;
  }

  std::cout << "Cards:" << std::endl;
  for (int i = 0; i < 5; ++i)
    for (std::vector<std::string>::const_iterator j = cards[i].begin(); j != cards[i].end(); ++j)
      std::cout << i << ": " << *j << std::endl;
  std::cout << "where\n0 = MasterCard\n1 = Visa\n2 = AMEX\n3 = Discover\n4 = Diners Club" << std::endl;
}

int main()
{
  get_card_numbers();
  return EXIT_SUCCESS;
}

