
[![logo][logo-url]][reflex-url]

Flex reimagined.  Fast, flexible, adds Boost.

RE/flex is as fast or faster than Flex.  For example, tokenizing a 2K C file
into 240 tokens takes:

<table>
<tr><th>Tool / library</th><th>Matcher</th><th>Time (μs)</th></tr>
<tr><td>reflex --fast</td><td>RE/flex</td><td>17</td></tr>
<tr><td>flex -+ --full</td><td>Flex</td><td>18</td></tr>
<tr><td>reflex --full</td><td>RE/flex</td><td>39</td></tr>
<tr><td>reflex -m=boost-perl</td><td>Boost.Regex (Perl mode)</td><td>288</td></tr>
<tr><td>reflex -m=boost</td><td>Boost.Regex (POSIX mode)</td><td>486</td></tr>
<tr><td>flex -+</td><td>Flex</td><td>3,968</td></tr>
<tr><td>RE2::Consume</td><td>RE2 (pre-compiled)</td><td>5,088</td></tr>
</table>

Note: *Best times of 10 tests with average time in micro seconds over 100 runs,
(clang-800.0.42.1 with -O2, 2.9 GHz Intel Core i7, 16 GB 2133 MHz LPDDR3).*

RE/flex is a flexible scanner-generator framework for generating regex-centric,
Flex-compatible scanners.  The RE/flex command-line tool is compatible with the
Flex command-line tool.  RE/flex offers:

* a *feature-rich replacement* of both [Flex](dinosaur.compilertools.net/#flex)
  and [Lex](dinosaur.compilertools.net/#lex), preserving the compatibility with
  the Bison (Yacc) parser generators (see the feature list below).

* an *enhancement* of [Boost.Regex](www.boost.org/libs/regex) to use its engine
  for matching, seaching, splitting and for scanning of tokens on various types
  of data sources, such as strings, files, and streams;

* a *regex library* for fast POSIX regular expression matching with extensions
  to POSIX such as lazy quantifiers, word boundary anchors, Unicode UTF-8, and
  more;

* a *flexible regex framework* that combines the above in a collection of C++
  class templates for pattern matching, searching, scanning, and splitting of
  strings, files, and streaming data.

The RE/flex repo includes tokenizers for Java, Python, and C/C++.

The RE/flex software is fully self-contained.  No other libraries are required.
Boost.Regex is optional to use as a regex engine.


List of features
----------------

- Fully compatible with Flex to eliminate a learning curve, making a transition
  to RE/flex frustration-free.
- Extensive documentation in the online [manual][manual-url].
- Generates MT-safe (reentrant) code by default.
- Generates clean source code that defines a C++ Lexer class derived from an
  abstract lexer class.
- RE/flex generates lex.yy.cpp files while Flex generates lex.yy.cc files (in
  C++ mode with flex option -+), to distinguish the differences.
- Configurable Lexer class generation to customize the interface for various
  parsers, including Yacc and Bison.
- Works with Bison and supports reentrant, bison-bridge and bison-locations.
- Generates scanners for lexical analysis on files, C++ streams, and (wide)
  strings.
- Generates Graphviz files to visualize FSMs with the Graphviz dot tool.
- Includes many examples, such as a tokenizer for C/C++ code, a tokenizer for
  Python code, a tokenizer for Java code, and more.
- Adds an extensible hierarchy of pattern matcher engines, with a choice of
  regex engines, including the RE/flex regex engine and Boost.Regex.
- Adds lazy quantifiers to the POSIX regular expression syntax, so not more
  hacks to work around the greedy repetitions in Flex.
- Adds word boundary anchors to the POSIX regular expression syntax.
- Adds Unicode support, which is integrated with UTF-8 pattern matching.
- Adds Unicode property matching `\p{C}` and C++11, Java, C#, and Python
  Unicode properties for identifier name matching.
- Adds indent `\i` and dedent `\j` regex patterns to match rules on text with
  indentation, including `\t` (tab) adjustments.
- Adds `%class` and `%init` to customize the generated Lexer classes.
- Adds `%include` to modularize lex specifications.
- Automatic internal conversion of UTF-16/32 to UTF-8 for matching Unicode on
  UTF-encoded input files, no need to define `YY_INPUT` for UTF conversions.
- Converts the official Unicode scripts Scripts.txt and UnicodeData.txt to
  UTF-8 patterns by applying a RE/flex scanner to convert these scripts to C++
  code.  Future Unicode standards can be automatically converted using these
  scanners that are written in RE/flex itself.
- Conversion of regex expressions, for regex engines that lack regex features.
- The RE/flex regex library makes C++11 std::regex and Boost.Regex much easier
  to use in plain C++ code for pattern matching on (wide) strings, files, and
  streams.


Installation
------------

    $ ./build.sh

or use the 'make' command to do the same:

    $ cd src; make

This compiles the **reflex** tool and installs it locally in reflex/bin.  You
can add this location to your $PATH variable to enable the new reflex command:

    export PATH=$PATH:/reflex_install_path/bin

The `libreflex.a` and `libreflex.so` libraries are saved in reflex/lib.  Link
against one of these libraries when you use the RE/flex regex engine in your
code.  The RE/flex header files are located in reflex/include/reflex.

To install the library and the 'reflex' command in /usr/local/lib and
/usr/local/bin:

    $ sudo ./install.sh

or use the 'make' command to do the same:

    $ cd lib; sudo make install
    $ cd src; sudo make install

Windows users: use reflex/bin/reflex.exe.

Optional libraries to install:

- To use Boost.Regex as a regex engine with the RE/flex library and scanner
  generator, install [Boost][boost-url] and link your code against
  `libboost_regex.a`

- To visualize the FSM graphs generated with **reflex** option `--graphs-file`,
  install [Graphviz dot][dot-url].


How do I use RE/flex?
---------------------

There are two ways you can use this project:

1. as a scanner generator for C++, similar to Flex;
2. as an extensible regex matching library for C++.

For the first option, simply build the **reflex** tool and run it on the
command line on a lex specification:

    $ reflex --flex --bison --graphs-file lexspec.l

This generates a scanner for Bison from the Flex specification `lexspec.l` and
saves the finite state machine (FSM) as a Graphviz `.gv` file that can be
visualized with the [Graphviz dot][dot-url] tool:

    $ dot -Tpdf reflex.INITIAL.gv > reflex.INITIAL.pdf
    $ open reflex.INITIAL.pdf

![Visualize DFA graphs with Graphviz dot][FSM-url]

Several examples are included to get you started.  See the [manual][manual-url]
for more details.

For the second option, simply use the new RE/flex matcher classes to start
pattern matching on strings, wide strings, files, and streams.

You can select matchers that are based on different regex engines:

- RE/flex regex: `#include <reflex/matcher.h>` and use `reflex::Matcher`;
- Boost.Regex: `#include <reflex/boostmatcher.h>` and use
  `reflex::BoostMatcher` or `reflex::BoostPosixMatcher`;
- C++11 std::regex: `#include <reflex/stdmatcher.h>` and use
  `reflex::StdMatcher` or `reflex::StdPosixMatcher`.

Each matcher may differ in regex syntax features (see the full documentation), 
but they have the same methods and iterators:

- `matches()` true if the input from begin to end matches the regex pattern;
- `find()` search input and return true if a match was found;
- `scan()` scan input and return true if input at current position matches;
- `split()` split input at the next match;
- `find.begin()`...`find.end()` filter iterator;
- `scan.begin()`...`scan.end()` tokenizer iterator;
- `split.begin()`...`split.end()` splitter iterator.

For example:

```{.cpp}
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex
// use a BoostMatcher to check if the birthdate string is a valid date
if (reflex::BoostMatcher("\\d{4}-\\d{2}-\\d{2}", birthdate).matches())
  std::cout << "Valid date!" << std::endl;
```

To search a string for words `\w+`:

```{.cpp}
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex
// use a BoostMatcher to search for words in a sentence
reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
while (matcher.find() == true)
  std::cout << "Found " << matcher.text() << std::endl;
```

The `split` method is roughly the inverse of the `find` method and returns text
located between matches.  For example using non-word matching `\W+`:

```{.cpp}
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex
// use a BoostMatcher to search for words in a sentence
reflex::BoostMatcher matcher("\\W+", "How now brown cow.");
while (matcher.split() == true)
  std::cout << "Found " << matcher.text() << std::endl;
```

To pattern match the content of a file that may use UTF-8, 16, or 32
encodings:

```{.cpp}
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex
// use a BoostMatcher to search and display words from a FILE
FILE *fd = fopen("somefile.txt", "r");
if (fd == NULL)
  exit(EXIT_FAILURE);
reflex::BoostMatcher matcher("\\w+", fd);
while (matcher.find() == true)
  std::cout << "Found " << matcher.text() << std::endl;
fclose(fd);
```

Same again, but this time with a C++ input stream:

```{.cpp}
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex
// use a BoostMatcher to search and display words from a stream
std::ifstream file("somefile.txt", std::ifstream::in);
reflex::BoostMatcher matcher("\\w+", file);
while (matcher.find() == true)
  std::cout << "Found " << matcher.text() << std::endl;
file.close();
```

Stuffing the search results into a container using RE/flex iterators:

```{.cpp}
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex
#include <vector>         // std::vector
// use a BoostMatcher to convert words of a sentence into a string vector
reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
std::vector<std::string> words(matcher.find.begin(), matcher.find.end());
```

Use C++11 range-based loops with RE/flex iterators:

```{.cpp}
#include <reflex/stdmatcher.h> // reflex::StdMatcher, reflex::Input, std::regex
// use a StdMatcher to to search for words in a sentence
for (auto& match : reflex::StdMatcher("\\w+", "How now brown cow.").find)
  std::cout << "Found " << match.text() << std::endl;
```

RE/flex also allows you to convert expressive regex syntax forms such as `\p`
Unicode classes, character class set operations such as `[a-z--[aeiou]]`,
escapes such as `\X`, and `(?x)` mode modifiers, to a regex string that the
underlying regex library understands and can use:

- `std::string reflex::Matcher::convert(const std::string& regex)`
- `std::string reflex::BoostMatcher::convert(const std::string& regex)`
- `std::string reflex::StdMatcher::convert(const std::string& regex)`

For example:

```{.cpp}
    #include <reflex/matcher.h> // reflex::Matcher, reflex::Input, reflex::Pattern
    // use a Matcher to check if sentence is in Greek:
    static const reflex::Pattern pattern(reflex::Matcher::convert("[\\p{Greek}\\p{Zs}\\pP]+"));
    if (reflex::Matcher(pattern, sentence).matches())
      std::cout << "This is Greek" << std::endl;
```

Conversion is fast (it runs in linear time in the size of the regex), but it is
not without some overhead.  Making converted regex patterns `static` as shown
above saves the cost of conversion to just once to support many matchings.

You can use `convert` with option `reflex::convert_flag::unicode` to make `.`
(dot), `\w`, `\s` and so on match Unicode.


Where do I find the documentation?
----------------------------------

Read more about RE/flex in the [manual][manual-url].


TODO / Work in progress / Nice to have
--------------------------------------

- Option to support Unicode without UTF-8 (RE/flex matcher/FSM is prepared)


License and copyright
---------------------

RE/flex by Robert van Engelen, Genivia Inc.
Copyright (c) 2015-2017, All rights reserved.   

RE/flex is distributed under the BSD-3 license LICENSE.txt.
Use, modification, and distribution are subject to the BSD-3 license.


Changelog
---------

- Nov 14, 2016: 0.9.0  beta released
- Nov 15, 2016: 0.9.1  improved portability
- Nov 17, 2016: 0.9.2  improvements and fixes for minor issues
- Nov 19, 2016: 0.9.3  replaces `%import` with `%include`, adds freespace option `-x`, fixes minor issues
- Nov 20, 2016: 0.9.4  fixes minor issues, added new examples/json.l
- Nov 25, 2016: 0.9.5  bug fixes and improvements
- Dec  1, 2016: 0.9.6  portability improvements
- Dec  6, 2016: 0.9.7  bug fixes, added option `--regexp-file`, Python tokenizer
- Dec  9, 2016: 0.9.8  fixes minor issues, improved reflex tool options `--full` and `--fast`, generates scanner with FSM table or a fast scanner with FSM code, respectively
- Jan  8, 2017: 0.9.9  bug fixes and improved Flex compatibility
- Jan 15, 2017: 0.9.10 improved compatibility with Flex options, fixed critical issue with range unions
- Jan 25, 2017: 0.9.11 added C++11 std::regex matching engine support, moved .h files to include/reflex, requires `#include <reflex/xyz.h>` from now on, fixed `errno_t` portability issue
- Mar  3, 2017: 0.9.12 refactored and improved, includes new regex converters for regex engines that lack regex features such as Unicode character classes
- Mar  4, 2017: 0.9.13 improved warning and error messages
- Mar  6, 2017: 0.9.14 option -v stats with execution timings, bug fixes

[logo-url]: https://www.genivia.com/images/reflex-logo.png
[reflex-url]: https://www.genivia.com/get-reflex.html
[manual-url]: https://www.genivia.com/doc/reflex/html
[flex-url]: http://dinosaur.compilertools.net/#flex
[lex-url]: http://dinosaur.compilertools.net/#lex
[bison-url]: http://dinosaur.compilertools.net/#bison
[dot-url]: http://www.graphviz.org
[FSM-url]: https://www.genivia.com/images/reflex-FSM.png
[boost-url]: http://www.boost.org
