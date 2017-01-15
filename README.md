
[![logo][logo-url]][reflex-url]

Flex reimagined.  Fast, flexible, adds Boost.

RE/flex is a flexible scanner-generator framework for generating regex-centric,
Flex-compatible scanners.  The RE/flex command-line tool is compatible with the
Flex command-line tool.  RE/flex offers:

* a *feature-rich replacement* of both [Flex](dinosaur.compilertools.net/#flex)
  and [Lex](dinosaur.compilertools.net/#lex), preserving the compatibility with
  the Bison (Yacc) parser generators (see the feature list below).

* an *enhancement* of [Boost.Regex](www.boost.org/libs/regex) to use its engine
  for matching, seaching, splitting and for scanning of tokens on various types
  of data sources, such as strings, files, and streams of unlimited length;

* a *regex library* for fast POSIX regular expression matching with extensions
  to POSIX such as lazy quantifiers, word boundary anchors, Unicode UTF-8, and
  much more;

* a *flexible regex framework* that combines the above in a collection of C++
  class templates for pattern matching, searching, scanning, and splitting of
  strings, files, and streaming data.

The repo includes tokenizers for Java, Python, and C/C++.


List of features
----------------

- Fully compatible with Flex to eliminate a learning curve, making a transition
  to RE/flex frustration-free.
- Extensive documentation in the online [manual][manual-url].
- RE/flex generates lex.yy.cpp files while Flex generates lex.yy.cc files (in
  C++ mode with option -+), to distinguish the differences.
- Generates MT-safe (reentrant) code by default.
- Generates clean source code that defines a C++ Lexer class derived from an abstract lexer class.
- Configurable Lexer class generation to customize the interface for various parsers, including Yacc and Bison.
- Works with Bison and supports reentrant, bison-bridge and bison-locations.
- Generates scanners for lexical analysis on files, C++ streams, and (wide) strings.
- Generates Graphviz files to visualize FSMs with the Graphviz dot tool.
- Includes many examples, such as a tokenizer for C/C++ code, a tokenizer for Python code, a tokenizer for Java code, and more.
- Adds an extensible hierarchy of pattern matcher engines, with a choice of
  regex engines, such as Boost.Regex and RE/flex regex.
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
- Automatic internal conversion from UTF-16/32 to UTF-8 for matching Unicode on
  UTF-encoded input files, no need to define `YY_INPUT` for UTF conversions.
- Converts the official Unicode scripts Scripts.txt and UnicodeData.txt to
  UTF-8 patterns by applying a RE/flex scanner to convert these scripts to C++
  code.  Future Unicode standards can be automatically converted using these
  scanners that are written in RE/flex itself.
- The RE/flex regex library makes Boost.Regex much easier to use in plain C++
  code for pattern matching on (wide) strings, files, and streams of
  potentially unlimited length (e.g. to search streaming data).


Installation
------------

    $ ./build.sh

or use the make command:

    $ cd src; make

This compiles the **reflex** tool and installs it in reflex/bin.  You can add
this location to your $PATH variable to enable the new reflex command:

    export PATH=$PATH:/reflex_install_path/bin

The `libreflex.a` and `libreflex.so` libraries are saved in reflex/lib.  Link
against one of these libraries when you use the RE/flex regex engine in your
code.  Header files are located in reflex/include.

Windows users: use reflex/bin/reflex.exe.

Optional:

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
pattern matching on strings, wide strings, files, and real streams (that are
potentially of unlimited length):

```{.cpp}
#include "boostmatcher.h" // reflex::BoostMatcher, reflex::Input, boost::regex
// use a BoostMatcher to check if the birthdate string is a valid date
if (reflex::BoostMatcher("\\d{4}-\\d{2}-\\d{2}", birthdate).matches())
  std::cout << "Valid date!" << std::endl;
```

To search a string for words `\w+`:

```{.cpp}
#include "boostmatcher.h" // reflex::BoostMatcher, reflex::Input, boost::regex
// use a BoostMatcher to search for words in a sentence
reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
while (matcher.find() == true)
  std::cout << "Found " << matcher.text() << std::endl;
```

The `split` method is roughly the inverse of the `find` method and returns text
located between matches.  For example using non-word matching `\W+`:

```{.cpp}
#include "boostmatcher.h" // reflex::BoostMatcher, reflex::Input, boost::regex
// use a BoostMatcher to search for words in a sentence
reflex::BoostMatcher matcher("\\W+", "How now brown cow.");
while (matcher.split() == true)
  std::cout << "Found " << matcher.text() << std::endl;
```

To pattern match the content of a file that may use UTF-8, 16, or 32
encodings:

```{.cpp}
#include "boostmatcher.h" // reflex::BoostMatcher, reflex::Input, boost::regex
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
#include "boostmatcher.h" // reflex::BoostMatcher, reflex::Input, boost::regex
// use a BoostMatcher to search and display words from a stream
std::ifstream file("somefile.txt", std::ifstream::in);
reflex::BoostMatcher matcher("\\w+", file);
while (matcher.find() == true)
  std::cout << "Found " << matcher.text() << std::endl;
file.close();
```

Stuffing the search results into a container using RE/flex iterators:

```{.cpp}
#include "boostmatcher.h" // reflex::BoostMatcher, reflex::Input, boost::regex
#include <vector>         // std::vector
// use a BoostMatcher to convert words of a sentence into a string vector
reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
std::vector<std::string> words(matcher.find.begin(), matcher.find.end());
```

Use C++11 range-based loops with RE/flex iterators:

```{.cpp}
#include "boostmatcher.h" // reflex::BoostMatcher, reflex::Input, boost::regex
// use a BoostMatcher to to search for words in a sentence
for (auto& match : reflex::BoostMatcher("\\w+", "How now brown cow.").find)
  std::cout << "Found " << match.text() << std::endl;
```


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

[logo-url]: https://www.genivia.com/images/reflex-logo.png
[reflex-url]: https://www.genivia.com/get-reflex.html
[manual-url]: https://www.genivia.com/doc/reflex/html
[flex-url]: http://dinosaur.compilertools.net/#flex
[lex-url]: http://dinosaur.compilertools.net/#lex
[bison-url]: http://dinosaur.compilertools.net/#bison
[dot-url]: http://www.graphviz.org
[FSM-url]: https://www.genivia.com/images/reflex-FSM.png
[boost-url]: http://www.boost.org
