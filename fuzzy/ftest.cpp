// test FuzzyMatcher
//
// ftest regex text [max_error[idsb]]
//
// where:
// 0 <= max_error <= 255 is the maximum edit distance^*, defaults to 1
// idsb is one or more optional characters i, d, or s to limit edits to
// insert, delete, and/or substitute, and b to limit matching to ASCII/binary
//
// *) Levenshstein distance: char insertion, substitution, deletion
//
// Build after installing RE/flex:
//   c++ -o ftest ftest.cpp -lreflex
//
// Test run:
//   ./ftest
//   ./ftest 1ids
// Example runs:
//   ./ftest 'abcd' 'axbcd'
//   ./ftest 'abcd' 'axcd'
//   ./ftest 'abcd' 'acd'
//   ./ftest 'a😀cd' 'axcd'
//   ./ftest 'abcd' 'a😀cd'
//   ./ftest 'aλcd' 'a😀cd'
//   ./ftest 'αλcd' 'α😀cd'
//   ./ftest 'aλβd' 'a😀βd'
//   ./ftest 'acd' 'a😀cd'
//   ./ftest 'αcd' 'α😀cd'
//   ./ftest 'aβd' 'a😀βd'
//   ./ftest 'αβd' 'α😀βd'
//   ./ftest 'a😀cd' 'acd'
//   ./ftest 'α😀cd' 'αcd'
//   ./ftest 'a😀βd' 'aβd'
//   ./ftest 'α😀βd' 'αβd'
//   ./ftest 'a\d+z' 'az'
//   ./ftest 'a\d+z' 'a9'
//   ./ftest 'abcd'  'aabcd'
//   ./ftest 'abcd'  'abbcd'
//   ./ftest 'ab_cd' 'abcd'
//   ./ftest 'ab_cd' 'ab-cd'
//   ./ftest 'ab_cd' 'abCd' 2
//   ./ftest 'ab_cd' 'ab_ab_cd' 2

// #define DEBUG // enable debugging to stderr

#include "fuzzymatcher.h"

int main(int argc, char **argv)
{
  if (argc > 2)
  {
    try
    {
      std::string regex = reflex::Matcher::convert(argv[1], reflex::convert_flag::unicode);
      reflex::Pattern reflex_pattern(regex, "mr");
      const char *text = argv[2];
      uint8_t max = 1;
      uint16_t flags = 0;
      if (argc > 3)
      {
        char *rest;
        unsigned long n = strtoul(argv[3], &rest, 10);
        if (n > 255)
        {
          fprintf(stderr, "max_error too large\n\n");
          exit(EXIT_FAILURE);
        }
        max = static_cast<uint8_t>(n);
        if (*rest != '\0')
        {
          if (strchr(rest, 'i') != NULL)
            flags |= reflex::FuzzyMatcher::INS;
          if (strchr(rest, 'd') != NULL)
            flags |= reflex::FuzzyMatcher::DEL;
          if (strchr(rest, 's') != NULL)
            flags |= reflex::FuzzyMatcher::SUB;
          if (strchr(rest, 'b') != NULL)
            flags |= reflex::FuzzyMatcher::BIN;
        }
      }
      reflex::FuzzyMatcher matcher(reflex_pattern, max | flags, text);
      if (!matcher.matches())
        printf("matches(): no match\n");
      else
        printf("matches(): match (%u edits)\n", matcher.edits());
      matcher.input(text);
      /* scan() may not work well with fuzzy matching, because of possible "gaps" between matches caused by fuzzy matches
         while (matcher.scan())
         printf("scan():    '%s' at %zu (%u edits)\n", matcher.text(), matcher.columno(), matcher.edits());
         matcher.input(text);
       */
      while (matcher.find())
        printf("find():    '%s' at %zu (%u edits)\n", matcher.text(), matcher.columno(), matcher.edits());
      matcher.input(text);
      while (matcher.split())
        printf("split():   '%s' at %zu (%u edits)\n", matcher.text(), matcher.columno(), matcher.edits());
    }
    catch (const reflex::regex_error& e)
    {
      std::cerr << e.what();
    }
  }
  else
  {
    uint8_t max = 1;
    uint16_t flags = 0;
    if (argc > 1)
    {
      char *rest;
      unsigned long n = strtoul(argv[1], &rest, 10);
      if (n > 255)
      {
        fprintf(stderr, "max_error too large\n\n");
        exit(EXIT_FAILURE);
      }
      max = static_cast<uint8_t>(n);
      if (*rest != '\0')
      {
        if (strchr(rest, 'i') != NULL)
          flags |= reflex::FuzzyMatcher::INS;
        if (strchr(rest, 'd') != NULL)
          flags |= reflex::FuzzyMatcher::DEL;
        if (strchr(rest, 's') != NULL)
          flags |= reflex::FuzzyMatcher::SUB;
        if (strchr(rest, 'b') != NULL)
          flags |= reflex::FuzzyMatcher::BIN;
      }
    }
    printf("PATTERN/TEXT DISTANCE=%u TESTING\n", max);
    const char *regex_texts[] = {
      "abcd", "ab_cd",
      "ab_cd", "ab-cd",
      "ab_cd", "abcd",
      "abcd", "abcd_",
      "abcd_", "abcd-",
      "abcd_", "abcd",

      "ab_*cd", "ab_cd",
      "ab_+cd", "ab-cd",
      "ab_+cd", "abcd",
      "abcd_*", "abcd_",
      "abcd_+", "abcd-",
      "abcd_+", "abcd",

      "^abcd$", "ab_cd",
      "^ab_cd$", "ab-cd",
      "^ab_cd$", "abcd",
      "^abcd$", "abcd_",
      "^abcd_$", "abcd-",
      "^abcd_$", "abcd",

      "\\<abcd\\>", "ab_cd",
      "\\<ab_cd\\>", "ab-cd",
      "\\<ab_cd\\>", "abcd",
      "\\<abcd\\>", "abcd_",
      "\\<abcd_\\>", "abcd-",
      "\\<abcd_\\>", "abcd",

      "abcd", "abbcd",
      "abcd", "axcd",
      "abcd", "acd",
      "abcd", "abcdd",
      "abcd", "abcx",
      "abcd", "abc",
      "abcd", "aabcd",

      "αβγδ", "αββγδ",
      "αβγδ", "αωγδ",
      "αβγδ", "αγδ",
      "αβγδ", "αβγδδ",
      "αβγδ", "αβγω",
      "αβγδ", "αβγ",
      "αβγδ", "ααβγδ",

      "aβcδ", "aββcδ",
      "aβcδ", "aωcδ",
      "aβcδ", "acδ",
      "aβcδ", "aβcδδ",
      "aβcδ", "aβcω",
      "aβcδ", "aβc",
      "aβcδ", "aaβcδ",

      "αργσ", "αρργσ",
      "αργσ", "αβγσ",
      "αργσ", "αγσ",
      "αργσ", "αργσσ",
      "αργσ", "αργω",
      "αργσ", "αργ",
      "αργσ", "ααργσ",

      "abcd", "abβcd",
      "abcd", "aωcd",
      "abcd", "abcdδ",
      "abcd", "abcω",
      "abcd", "aαbcd",

      "aβcd", "aβbcd",
      "aβcd", "abβcd",
      "aβcd", "axcd",
      "aβcd", "acd",

      ".bcd", "abβcd",
      ".bcd", "aωcd",
      ".bcd", "abcdδ",
      ".bcd", "abcω",
      ".bcd", "aαbcd",
      ".βcd", "aβbcd",
      ".βcd", "abβcd",
      ".βcd", "axcd",
      ".βcd", "acd",

      NULL, NULL };
    int it = 0;
    int hits = 0;
    while (regex_texts[it] != NULL)
    {
      try
      {
        std::string regex = reflex::Matcher::convert(regex_texts[it++], reflex::convert_flag::unicode);
        reflex::Pattern reflex_pattern(regex, "mr");
        const char *text = regex_texts[it++];
        reflex::FuzzyMatcher matcher(regex, max | flags, text);
        printf("'%s'/'%s'\n", regex.c_str(), text);
        if (!matcher.matches())
        {
          printf("  matches(): no match\n");
        }
        else
        {
          printf("  matches(): match (%u edits)\n", matcher.edits());
          ++hits;
        }
        matcher.input(text);
        while (matcher.find())
        {
          printf("  find():    '%s' at %zu (%u edits)\n", matcher.text(), matcher.columno(), matcher.edits());
          ++hits;
        }
      }
      catch (const reflex::regex_error& e)
      {
        std::cerr << e.what();
      }
    }
    printf("FUZZY HITS = %.3g%%\n", 100.*hits/it);
  }
  return 0;
}
