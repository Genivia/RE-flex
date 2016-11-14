
Installation
============

First read doc/html/index.html for more information about this software release.

Download Boost from <http://www.boost.org/users/download> and make sure to
build Boost::regex to obtain `libboost_regex.a`.

Then build the reflex library and the reflex tool as follows:

    cd src
    make

This compiles the reflex tool and installs it locally in bin/reflex.  You can
add this location to your $PATH variable to enable the new reflex command:

    export PATH=$PATH:/reflex_install_path/bin

For windows users, please use reflex.exe located in bin/reflex.exe.


Examples
--------

The examples require the bison tool to demonstrate scanning with parsing,
including the --bison and --bison-bridge options.  Download bison from
<http://www.gnu.org/software/bison>

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
    |       |__ index.html
    |       |__ ...
    |
    |__ examples/
    |   |__ Makefile
    |   |__ ...
    |
    |__ include/
    |   |__ abslexer.h
    |   |__ absmatcher.h
    |   |__ bits.h
    |   |__ boostmatcher.h
    |   |__ debug.h
    |   |__ flexlexer.h
    |   |__ input.h
    |   |__ matcher.h
    |   |__ pattern.h
    |   |__ ranges.h
    |   |__ setop.h
    |   |__ traits.h
    |   |__ utf8.h
    |
    |__ lib/
    |   |__ Makefile
    |   |__ debug.cpp
    |   |__ input.cpp
    |   |__ matcher.cpp
    |   |__ pattern.cpp
    |   |__ utf8.cpp
    |
    |__ src/
    |   |__ Makefile
    |   |__ language_scripts.cpp
    |   |__ language_scripts.h
    |   |__ language_scripts.l
    |   |__ letter_scripts.cpp
    |   |__ letter_scripts.h
    |   |__ letter_scripts.l
    |   |__ pl_scripts.cpp
    |   |__ pl_scripts.h
    |   |__ pl_scripts.l
    |   |__ reflex.cpp
    |   |__ reflex.h
    |
    |__ test/
    |   |__ Makefile
    |   |__ ...
    |
    |__ unicode/
	|__ Scripts.txt
	|__ UnicodeData.txt


Copyright and License
---------------------

RE/flex by Robert van Engelen, Genivia Inc.
Copyright (c) 2015-2016, All rights reserved.   

RE/flex is distributed under the BSD-3 license LICENSE.txt.
Use, modification, and distribution are subject to the BSD-3 license.
