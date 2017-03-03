#
# requires Boost.Regex and std::regex
#
CPP=c++
BOOST=/opt/local/include
BOOSTLIB=/opt/local/lib/libboost_regex-mt.dylib
REFLEX=../bin/reflex
LIBS=../lib/libreflex.a
COFLAGS=-g -O3
CWFLAGS=-Wall -Wunused -Wextra -Wno-unused-label -Wno-strict-overflow
CIFLAGS=-I. -I../include -I $(BOOST)
CMFLAGS=
# CMFLAGS=-DDEBUG
CFLAGS=$(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS)

all:		test_ranges lorem test rtest btest stest

lorem:		lorem.cpp
		$(CPP) $(CFLAGS) -o $@ lorem.cpp $(OBJS) $(LIBS) $(BOOSTLIB)
		./lorem

test:		test.cpp
		$(CPP) $(CFLAGS) -o $@ test.cpp $(OBJS) $(LIBS) $(BOOSTLIB)
		./test '(a|b)*abb' 'ababb'

rtest:		rtest.cpp
		$(CPP) $(CFLAGS) -o $@ rtest.cpp $(OBJS) $(LIBS)
		./rtest

btest:		btest.cpp
		$(CPP) $(CFLAGS) -o $@ btest.cpp $(OBJS) $(LIBS) $(BOOSTLIB)
		./btest

stest:		stest.cpp
		$(CPP) -std=c++11 $(CFLAGS) -o $@ stest.cpp $(OBJS) $(LIBS)
		./stest

test_bits:	test_bits.cpp
		$(CPP) $(CFLAGS) -o $@ test_bits.cpp $(OBJS) $(LIBS)
		./test_bits

test_ranges:	test_ranges.cpp
		$(CPP) $(CFLAGS) -o $@ test_ranges.cpp $(OBJS) $(LIBS)
		./test_ranges

.PHONY:		clean
clean:
		-rm -rf *.dSYM
		-rm -f *.o *.gch *.log
		-rm -f lex.yy.h lex.yy.cpp y.tab.h y.tab.c
		-rm -rf *.dSYM
		-rm -f *.o *.gch *.log
		-rm -f lex.yy.h lex.yy.cpp y.tab.h y.tab.c reflex.*.cpp reflex.*.gv reflex.*.txt
		-rm -f test_regex_history dump.gv dump.pdf dump.cpp
		-rm -f lorem test rtest btest stest test_bits test_ranges
