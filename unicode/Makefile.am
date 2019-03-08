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

LEX=$(top_builddir)/bootstrap/bootstrap_reflex
LEX_OUTPUT_ROOT=lex.yy

AM_LFLAGS= --outfile=lex.yy.c













noinst_PROGRAMS= \
  gen_block_scripts \
  gen_language_scripts \
  gen_letter_scripts

gen_block_scripts_SOURCES=gen_block_scripts.ll
gen_block_scripts_la_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include
gen_block_scripts_LDADD=\
  $(top_builddir)/bootstrap/libbootstrap.la

gen_language_scripts_SOURCES=gen_language_scripts.ll
gen_block_scripts_la_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include
gen_language_scripts_LDADD=\
  $(top_builddir)/bootstrap/libbootstrap.la

gen_letter_scripts_SOURCES=gen_letter_scripts.ll
gen_block_scripts_la_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include
gen_letter_scripts_LDADD=\
  $(top_builddir)/bootstrap/libbootstrap.la



noinst_LTLIBRARIES = libunicode.la

libunicode_la_SOURCES = \
  block_scripts.cc \
  language_scripts.cc \
  letter_scripts.cc

libunicode_la_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include

block_scripts.cc: Blocks.txt gen_block_scripts
	./gen_block_scripts < $(top_srcdir)/unicode/Blocks.txt > block_scripts.cc

language_scripts.cc:	Scripts.txt gen_language_scripts
	./gen_language_scripts < $(top_srcdir)/unicode/Scripts.txt > language_scripts.cc

letter_scripts.cc:	UnicodeData.txt gen_letter_scripts
	./gen_letter_scripts < $(top_srcdir)/unicode/UnicodeData.txt > letter_scripts.cc

EXTRA_DIST= \
  Blocks.txt \
  Scripts.txt \
  UnicodeData.txt

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
