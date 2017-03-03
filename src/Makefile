INSTALL_BIN=/usr/local/bin
CPP=c++
INCS=../include/reflex/convert.h ../include/reflex/debug.h ../include/reflex/error.h ../include/reflex/input.h ../include/reflex/pattern.h ../include/reflex/utf8.h
LIBS=../lib/libreflex.a
COFLAGS=-g -O2
CWFLAGS=-Wall -Wunused -Wextra
CIFLAGS=-I. -I../include
CMFLAGS=
# CMFLAGS=-DTIMED
# CMFLAGS=-DWITH_BITS -DTIMED
# CMFLAGS=-DDEBUG
# CMFLAGS=-DDEBUG -DTIMED
CFLAGS=$(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS)

reflex_includes=	reflex.h $(INCS)
reflex_objects=		reflex.o $(LIBS)

.PHONY:			release install clean distclean

release:
			cd ../lib; $(MAKE) libreflex.a
			$(MAKE) reflex
			-cp -f reflex ../bin

install:		release
			-mkdir -p $(INSTALL_BIN)
			-cp -f reflex $(INSTALL_BIN)

reflex:			$(reflex_includes) $(reflex_objects)
			$(CPP) $(CFLAGS) -o $@ $(reflex_objects)

.cc.o:
			$(CPP) $(CFLAGS) -c $<
.cpp.o:
			$(CPP) $(CFLAGS) -c $<

clean:
			-rm -rf *.dSYM
			-rm -f *.o *.gch *.log
			-rm -f reflex

distclean:
			-rm -rf *.dSYM
			-rm -f *.o *.gch *.log
			-rm -f reflex ../bin/reflex
