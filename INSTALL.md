[![logo][logo-url]][reflex-url]

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

Optional:

- To use Boost.Regex as a regex engine with the RE/flex library and scanner
  generator, install [Boost][boost-url] and link your code against
  `libboost_regex.a`

- To visualize the FSM graphs generated with **reflex** option `--graphs-file`,
  install [Graphviz dot][dot-url].


Examples
--------

The examples require the [Bison][bison-url] tool to demonstrate scanning with
parsing using the **reflex** `--bison` and `--bison-bridge` options.

To build the examples, first build the reflex tool and then execute:

    cd examples
    make

If `libboost_regex.a` could not be found, then edit lib/Makefile to change the
installation location of Boost so that `boost/regex.hpp` and `libboost_regex.a`
can be properly located:

    BOOST=$(HOME)/boost
    LIBS=../lib/reflex.a $(BOOST)/stage/lib/libboost_regex.a
    CIFLAGS=-I. -I../include -I $(HOME)/boost


Files
-----

The RE/flex distribution directory tree should at a minimum contain the
following files:

    reflex/
    |
    |__ bin/
    |   |__ reflex.exe
    |
    |__ doc/
    |   |__ Doxyfile
    |   |__ README.md
    |   |__ footer.html
    |   |__ header.html
    |   |__ reflex-logo.png
    |   |__ html/
    |   |   |__ index.html
    |   |__ man/
    |       |__ reflex.1
    |
    |__ examples/
    |   |__ Makefile
    |   |__ ...
    |
    |__ include/
    |   |__ reflex/
    |       |__ abslexer.h
    |       |__ absmatcher.h
    |       |__ bits.h
    |       |__ boostmatcher.h
    |       |__ convert.h
    |       |__ debug.h
    |       |__ error.h
    |       |__ flexlexer.h
    |       |__ input.h
    |       |__ matcher.h
    |       |__ pattern.h
    |       |__ posix.h
    |       |__ ranges.h
    |       |__ setop.h
    |       |__ stdmatcher.h
    |       |__ timer.h
    |       |__ traits.h
    |       |__ unicode.h
    |       |__ utf8.h
    |
    |__ lib/
    |   |__ Makefile
    |   |__ convert.cpp
    |   |__ debug.cpp
    |   |__ error.cpp
    |   |__ input.cpp
    |   |__ matcher.cpp
    |   |__ pattern.cpp
    |   |__ posix.cpp
    |   |__ unicode.cpp
    |   |__ utf8.cpp
    |
    |__ src/
    |   |__ Makefile
    |   |__ reflex.cpp
    |   |__ reflex.h
    |
    |__ test/
    |   |__ Makefile
    |   |__ ...
    |
    |__ unicode/
        |__ Makefile
        |__ Scripts.txt
        |__ UnicodeData.txt
        |__ language_scripts.cpp
        |__ language_scripts.l
        |__ letter_scripts.cpp
        |__ letter_scripts.l


License and copyright
---------------------

RE/flex by Robert van Engelen, Genivia Inc.
Copyright (c) 2015-2017, All rights reserved.   

RE/flex is distributed under the BSD-3 license LICENSE.txt.
Use, modification, and distribution are subject to the BSD-3 license.

[logo-url]: https://www.genivia.com/images/reflex-logo.png
[reflex-url]: https://www.genivia.com/get-reflex.html
[manual-url]: https://www.genivia.com/doc/reflex/html
[flex-url]: http://dinosaur.compilertools.net/#flex
[lex-url]: http://dinosaur.compilertools.net/#lex
[bison-url]: http://dinosaur.compilertools.net/#bison
[dot-url]: http://www.graphviz.org
[FSM-url]: https://www.genivia.com/images/reflex-FSM.png
[boost-url]: http://www.boost.org
