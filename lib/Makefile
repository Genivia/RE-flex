INSTALL=/usr/lib
CPP=c++
AR=ar
COFLAGS=-O3
CWFLAGS=-Wall -Wunused -Wextra
CIFLAGS=-I. -I../include
CMFLAGS=
# CMFLAGS=-g -DDEBUG
CFLAGS=$(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS)

release:		libreflex.a libreflex.so

install:		release
			cp -f libreflex.a $(INSTALL)

libreflex.a:		debug.o input.o matcher.o pattern.o utf8.o
			$(AR) -rsc libreflex.a debug.o input.o matcher.o pattern.o utf8.o
libreflex.so:		debug.cpp input.cpp matcher.cpp pattern.cpp utf8.cpp
			$(CPP) $(CFLAGS) -shared -o libreflex.so -fPIC debug.cpp input.cpp matcher.cpp pattern.cpp utf8.cpp

.cc.o:
			$(CPP) $(CFLAGS) -c $<
.cpp.o:
			$(CPP) $(CFLAGS) -c $<

.PHONY:			clean distclean
clean:
			-rm -rf *.dSYM
			-rm -f *.o *.gch *.log
distclean:
			-rm -rf *.dSYM
			-rm -f *.o *.gch *.log
			-rm -f *.a *.so
