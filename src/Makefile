INSTALL=/usr/local/bin
CPP=c++
INCS=../include/reflex/debug.h ../include/reflex/input.h ../include/reflex/pattern.h ../include/reflex/utf8.h
LIBS=../lib/libreflex.a
REFLEX=./reflex
COFLAGS=-O2
CWFLAGS=-Wall -Wunused -Wextra
CIFLAGS=-I. -I../include
CMFLAGS=
# CMFLAGS=-DTIMED
# CMFLAGS=-DWITH_BITS -DTIMED
# CMFLAGS=-g -DDEBUG
# CMFLAGS=-g -DDEBUG -DTIMED
CFLAGS=$(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS)

reflex_incs=		reflex.h letter_scripts.h language_scripts.h pl_scripts.h $(INCS)
reflex_objs=		reflex.o letter_scripts.o language_scripts.o pl_scripts.o $(LIBS)

release:
			cd ../lib; $(MAKE) libreflex.a
			$(MAKE) reflex
			-cp -f reflex ../bin

install:		release
			-cp -f reflex $(INSTALL)

reflex:			$(reflex_incs) $(reflex_objs)
			$(CPP) $(CFLAGS) -o $@ $(reflex_objs)

language_scripts.cpp:	language_scripts.l
			$(REFLEX) language_scripts.l
			$(CPP) $(CFLAGS) -o gen_language_scripts_cpp lex.yy.cpp $(LIBS)
			./gen_language_scripts_cpp < ../unicode/Scripts.txt > language_scripts.cpp
			-rm -f lex.yy.cpp gen_language_scripts_cpp

letter_scripts.cpp:	letter_scripts.l
			$(REFLEX) letter_scripts.l
			$(CPP) $(CFLAGS) -o gen_letter_scripts_cpp lex.yy.cpp $(LIBS)
			./gen_letter_scripts_cpp < ../unicode/UnicodeData.txt > letter_scripts.cpp
			-rm -f lex.yy.cpp gen_letter_scripts_cpp

.cc.o:
			$(CPP) $(CFLAGS) -c $<
.cpp.o:
			$(CPP) $(CFLAGS) -c $<

.PHONY:			clean distclean
clean:
			-rm -rf *.dSYM
			-rm -f *.o *.gch *.log
			-rm -f reflex
distclean:
			-rm -rf *.dSYM
			-rm -f *.o *.gch *.log
			-rm -f reflex ../bin/reflex
