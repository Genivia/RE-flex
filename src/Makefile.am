#  Copyright (c) 2018, Michael B Tegtmeyer
#  All rights reserved.
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#      * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#      * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#      * Neither the name of the author nor the names of its contributors may
#        be used to endorse or promote products derived from this software
#        without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
#  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
#  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.




# Convenience library to avoid compiling twice for bootstrapping
# Builds the base library without unicode pieces. Not installable as
# there are missing symbols
noinst_LTLIBRARIES= libbasereflex.la

libbasereflex_la_SOURCES= \
  convert.cpp \
  debug.cpp \
  error.cpp \
  input.cpp \
  matcher.cpp \
  pattern.cpp \
  posix.cpp \
  unicode.cpp \
  utf8.cpp

libbasereflex_la_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include


# Not installed bootstrap targets
# Builds a version of reflex (bootstrap_reflex) with noop unicode stubs. This
# version of reflex is only used to build the 'gen_*_scripts' parsers that
# create the downstream *_scripts.cc files. If parsing in the gen_*_scripts
# requires unicode support in the future from bootstrap_reflex, this
# will likely not work as expected.
noinst_PROGRAMS= \
  bootstrap_reflex \
  gen_block_scripts \
  gen_language_scripts \
  gen_letter_scripts

bootstrap_reflex_SOURCES= \
  reflex.h \
  reflex.cpp \
  bootstrap_unicode.cc

bootstrap_reflex_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include

bootstrap_reflex_LDADD= \
  libbasereflex.la


# noinst programs that make the real unicode sources from 'bootstrap_reflex'.
# Must link against libbasereflex.
LEX=$(builddir)/bootstrap_reflex
LEX_OUTPUT_ROOT=lex.yy

AM_LFLAGS= --outfile=lex.yy.c

gen_block_scripts_SOURCES= \
  gen_block_scripts.ll \
  bootstrap_unicode.cc

gen_block_scripts_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include

gen_block_scripts_LDADD=\
  libbasereflex.la


gen_language_scripts_SOURCES= \
  gen_language_scripts.ll \
  bootstrap_unicode.cc

gen_language_scripts_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include

gen_language_scripts_LDADD=\
  libbasereflex.la


gen_letter_scripts_SOURCES= \
  gen_letter_scripts.ll \
  bootstrap_unicode.cc

gen_letter_scripts_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include

gen_letter_scripts_LDADD=\
  libbasereflex.la


# explicitly encode dependancies for the gen_*_scripts.cc sources on
# the 'bootstrap_reflex' executable.
gen_block_scripts.cc: bootstrap_reflex

gen_language_scripts.cc: bootstrap_reflex

gen_letter_scripts.cc: bootstrap_reflex






# Now that the final unicode gen_*_scripts.cc files are create, build
# the actual installed libreflex target. Links against the convenience library
# libbasereflex
lib_LTLIBRARIES= libreflex.la

libreflex_la_SOURCES= \
  block_scripts.cc \
  language_scripts.cc \
  letter_scripts.cc


libreflex_la_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include

libreflex_la_LDFLAGS= -version-info 1:1:0

libreflex_la_LIBADD= \
  libbasereflex.la

block_scripts.cc: Blocks.txt gen_block_scripts
	$(builddir)/gen_block_scripts < $(srcdir)/Blocks.txt > block_scripts.cc

language_scripts.cc:	Scripts.txt gen_language_scripts
	$(builddir)/gen_language_scripts < $(srcdir)/Scripts.txt > language_scripts.cc

letter_scripts.cc:	UnicodeData.txt gen_letter_scripts
	$(builddir)/gen_letter_scripts < $(srcdir)/UnicodeData.txt > letter_scripts.cc




# Build the final installed reflex executable linked against the final
# installed library

bin_PROGRAMS= reflex

reflex_SOURCES = \
  reflex.h \
  reflex.cpp

reflex_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include \
  -DPLATFORM=\"$(PLATFORM)\"

reflex_LDADD = \
  libreflex.la




# make the manfile here because doc is dependent on Doxygen and the manfile
# is dependent on reflex itself
dist_man1_MANS= reflex.l

# not clear why help2man isn't used here unless it is a WIN or licensing issue
reflex.l: $(top_srcdir)/doc/man.sh $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) $(top_builddir)/src/reflex$(EXEEXT)
	$(top_srcdir)/doc/man.sh reflex reflex.l ${PACKAGE_VERSION}


# Ensure the unicode scripts are distributed with the source
EXTRA_DIST= \
  reflex.l \
  Blocks.txt \
  Scripts.txt \
  UnicodeData.txt

# Clean up after ourselves.
CLEANFILES= \
  block_scripts.cc \
  language_scripts.cc \
  letter_scripts.cc \
  gen_block_scripts.cc \
  gen_language_scripts.cc \
  gen_letter_scripts.cc \
  gen_block_scripts \
  gen_language_scripts \
  gen_letter_scripts

