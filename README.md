
[![logo][logo-url]][reflex-url]

Flex reimagined.  Fast, extensible, adds Boost.

RE/flex is a flexible scanner-generator framework for generating regex-centric,
Flex-compatible scanners.


What is RE/flex?
----------------

* a *feature-rich replacement* of both [Flex](dinosaur.compilertools.net/#flex)
  and [Lex](dinosaur.compilertools.net/#lex);

* an *enhancement* of [Boost.Regex](www.boost.org/libs/regex) to use its engine
  for matching, seaching, splitting and for scanning of tokens on various types
  of data sources, such as strings, files, and streams of unlimited length;

* a regex library for fast POSIX regular expression matching with extensions to
  POSIX such as lazy quantifiers, word boundary anchors, Unicode UTF-8, and
  much more;

* a flexible regex library that combines the above in a collection of C++ class
  templates for pattern matching, searching, scanning, and splitting of
  strings, files, and streaming data.


Installation
------------

    $ ./build.sh

or use the make command:

    $ cd src; make

This compiles the **reflex** tool and installs it locally in bin/reflex.  You
can add this location to your $PATH variable to enable the new reflex command:

    export PATH=$PATH:/reflex_install_path/bin

Windows users: use bin/reflex.exe.


How do I use RE/flex?
---------------------

There are two ways you can use this project:

1. as a scanner generator for C++, similar to Flex.
2. as an extensible regex matching library for C++.

For the first option, simply build the **reflex** tool and run it on the
command line on a lex specification:

    $ reflex --flex --bison lexspec.l

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

To pattern matching the content of a file that may use UTF-8, 16, or 32
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
```

Same again, but this time with a C++ input stream:

```{.cpp}
#include "boostmatcher.h" // reflex::BoostMatcher, reflex::Input, boost::regex
// use a BoostMatcher to search and display words from a stream
std::ifstream file = fopen("somefile.txt", "r");
reflex::BoostMatcher matcher("\\w+", file);
while (matcher.find() == true)
  std::cout << "Found " << matcher.text() << std::endl;
```

Stuffing the search results into a container using RE/flex iterators:

```{.cpp}
#include "boostmatcher.h" // reflex::BoostMatcher, reflex::Input, boost::regex
#include <vector>         // std::vector
// use a BoostMatcher to convert words of a sentence into a string vector
reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
std::vector<std::string> words(matcher.find.begin(), matcher.find.end());
```


Where do I find the documentation?
----------------------------------

Read more about RE/flex in the [manual][manual-url].


License and copyright
---------------------

RE/flex by Robert van Engelen, Genivia Inc.
Copyright (c) 2015-2016, All rights reserved.   

RE/flex is distributed under the BSD-3 license LICENSE.txt.
Use, modification, and distribution are subject to the BSD-3 license.


Changelog
---------

- Nov 14, 2016: RE/flex 0.9.0 beta released

[logo-url]: https://www.genivia.com/images/reflex-logo.png
[reflex-url]: https://www.genivia.com/get-reflex.html
[manual-url]: https://www.genivia.com/doc/reflex/html
