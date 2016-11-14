CPP=c++
BOOST=$(HOME)/boost
REFLEX=../bin/reflex
LIBS=../lib/libreflex.a
COFLAGS=-g -O2
CWFLAGS=-Wall -Wunused -Wextra
CIFLAGS=-I. -I../include -I $(HOME)/boost
CMFLAGS=
# CMFLAGS=-DDEBUG
CFLAGS=$(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS)

all:		lorem test rtest btest

lorem:		lorem.cpp
		$(CPP) $(CFLAGS) -o $@ lorem.cpp $(OBJS) $(LIBS) $(BOOST)/stage/lib/libboost_regex.a
		./lorem

test:		test.cpp
		$(CPP) $(CFLAGS) -o $@ test.cpp $(OBJS) $(LIBS) $(BOOST)/stage/lib/libboost_regex.a
		./test '(a|b)*?abb' 'ababb'

rtest:		rtest.cpp
		$(CPP) $(CFLAGS) -o $@ rtest.cpp $(OBJS) $(LIBS)
		./rtest

btest:		btest.cpp
		$(CPP) $(CFLAGS) -o $@ btest.cpp $(OBJS) $(LIBS) $(BOOST)/stage/lib/libboost_regex.a
		./btest

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
		-rm -f lex.yy.h lex.yy.cpp y.tab.h y.tab.c reflex.*.cpp reflex.*.gv
		-rm -f test_regex_history dump.gv dump.pdf dump.cpp
		-rm -f lorem test rtest btest test_bits test_ranges
