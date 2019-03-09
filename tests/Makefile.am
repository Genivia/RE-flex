check_PROGRAMS= \
  lorem \
  test \
  rtest \
  btest \
  stest \
  test_bits \
  test_ranges

AM_CPPFLAGS= \
  -Wall -Wextra -pedantic \
  -I$(top_srcdir)/include \
  $(BOOST_CPPFLAGS)

LDFLAGS= \
  $(BOOST_LDFLAGS)

LDADD= \
  $(top_builddir)/src/libreflex.la \
  $(BOOST_REGEX_LIB)

lorem_SOURCES= \
  lorem.cpp \
  test_lorem_filter.h \
  test_lorem_splitter.h \
  test_lorem_tokenizer.h

test_SOURCES= test.cpp
rtest_SOURCES= rtest.cpp
btest_SOURCES= btest.cpp
stest_SOURCES= stest.cpp
test_bits_SOURCES= test_bits.cpp
test_ranges_SOURCES= test_ranges.cpp

TESTS= \
  test rtest btest stest test_bits test_ranges

EXTRA_DIST =\
  utf8lorem.txt \
  utf16lorem.txt \
  utf32lorem.txt \
  latin1lorem.txt \
  ebcdiclorem.txt
