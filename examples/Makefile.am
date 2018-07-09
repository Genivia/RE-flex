# These examples require Bison or Yacc
#
# To quickly build the examples:
# > make -f Make
#
# Or use configure and make to build:
# > ./configure && make
# > cd examples
# > make
#
# To build examples3.0 requires Bison 3.0 or greater:
# > make -f Make examples3.0

REFLEX    = $(top_builddir)/src/reflex
REFLAGS   =
LIBREFLEX = $(top_builddir)/lib/libreflex.a
CPPFLAGS  = -I. -I$(top_srcdir)/include

all-local:	examples

.PHONY:		examples

examples:	flexexample1 \
		flexexample2 \
		flexexample3 \
		flexexample4 \
		flexexample5 \
		flexexample6 \
		flexexample7 \
		flexexample8 \
		reflexexample3 \
		reflexexample4 \
		reflexexample5 \
		reflexexample6 \
		reflexexample7 \
		reflexexample8 \
		ctokens \
		jtokens \
		ptokens \
		echo \
		calc \
		wc \
		wcu \
		wcpp \
		wcwc \
		tag \
		tag_lazy \
		tag_lazystates \
		tag_unicode \
		tag_tidy \
		cow \
		cows \
		indent \
		json \
		csv \
		braille \
		unicode \
		cards

flexexample1:	flexexample1.l
		$(REFLEX) $(REFLAGS) --flex $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)
		./flexexample1 < flexexample1.l

flexexample2:	flexexample2.l
		$(REFLEX) $(REFLAGS) --flex $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)
		./flexexample2 < flexexample2.l

flexexample3:	flexexample3.l flexexample3.y
		$(YACC) -d flexexample3.y
		$(REFLEX) $(REFLAGS) --flex --bison $<
		$(CC) $(CPPFLAGS) $(CXXFLAGS) -c y.tab.c
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ y.tab.o lex.yy.cpp $(LIBREFLEX)
		./flexexample3 < flexexample3.test

flexexample4:	flexexample4.l flexexample4.y
		$(YACC) -d flexexample4.y
		$(REFLEX) $(REFLAGS) --flex --bison-bridge --header-file $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DWITH_NO_C -o $@ y.tab.c lex.yy.cpp $(LIBREFLEX)
		./flexexample4 < flexexample4.test

flexexample5:	flexexample5.l flexexample5.y
		$(YACC) -d flexexample5.y
		$(REFLEX) $(REFLAGS) --flex --bison-bridge --header-file $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ y.tab.c lex.yy.cpp $(LIBREFLEX)
		./flexexample5 < flexexample5.test

flexexample6:	flexexample6.l
		$(REFLEX) $(REFLAGS) --flex $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)
		./flexexample6 < flexexample6.l

flexexample7:	flexexample7.l flexexample7.y
		$(YACC) -d flexexample7.y
		$(REFLEX) $(REFLAGS) --flex --bison-locations $<
		$(CC) $(CPPFLAGS) $(CXXFLAGS) -c y.tab.c
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ y.tab.o lex.yy.cpp $(LIBREFLEX)
		./flexexample7 < flexexample7.test

flexexample8:	flexexample8.l flexexample8.y
		$(YACC) -d flexexample8.y
		$(REFLEX) $(REFLAGS) --flex --bison-locations --bison-bridge --header-file $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ y.tab.c lex.yy.cpp $(LIBREFLEX)
		./flexexample8 < flexexample8.test

reflexexample3:	reflexexample3.l reflexexample3.y
		$(YACC) -d reflexexample3.y
		$(REFLEX) $(REFLAGS) --bison --header-file $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ y.tab.c lex.yy.cpp $(LIBREFLEX)
		./reflexexample3 < flexexample3.test

reflexexample4:	reflexexample4.l flexexample4.y
		$(YACC) -d flexexample4.y
		$(REFLEX) $(REFLAGS) --bison-bridge --header-file $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DWITH_NO_C -o $@ y.tab.c lex.yy.cpp $(LIBREFLEX)
		./reflexexample4 < flexexample4.test

reflexexample5:	reflexexample5.l flexexample5.y
		$(YACC) -d flexexample5.y
		$(REFLEX) $(REFLAGS) --bison-bridge --header-file $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ y.tab.c lex.yy.cpp $(LIBREFLEX)
		./reflexexample5 < flexexample5.test

reflexexample6:	reflexexample6.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)
		./reflexexample6 < reflexexample6.l

reflexexample7:	reflexexample7.l flexexample7.y
		$(YACC) -d flexexample7.y
		$(REFLEX) $(REFLAGS) --bison-locations $<
		$(CC) $(CPPFLAGS) $(CXXFLAGS) -c y.tab.c
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ y.tab.o lex.yy.cpp $(LIBREFLEX)
		./reflexexample7 < flexexample7.test

reflexexample8:	reflexexample8.l flexexample8.y
		$(YACC) -d flexexample8.y
		$(REFLEX) $(REFLAGS) --bison-locations --bison-bridge --header-file $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ y.tab.c lex.yy.cpp $(LIBREFLEX)
		./reflexexample8 < flexexample8.test

ctokens:	ctokens.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

jtokens:	jtokens.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

ptokens:	ptokens.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

echo:		echo.l
		$(REFLEX) $(REFLAGS) --flex $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

readline:	readline.l
		$(REFLEX) $(REFLAGS) --flex $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX) -lreadline

calc:		calc.l calc.y
		$(REFLEX) $(REFLAGS) $<
		$(YACC) -d calc.y
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ y.tab.c lex.yy.cpp $(LIBREFLEX)

wc:		wc.l
		$(REFLEX) $(REFLAGS) --flex $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

wcu:		wcu.l
		$(REFLEX) $(REFLAGS) --flex $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

wcpp:		wcpp.l
		$(REFLEX) $(REFLAGS) --flex $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

wcwc:		wcwc.l
		$(REFLEX) $(REFLAGS) --flex $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

tag:		tag.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

tag_lazy:	tag_lazy.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

tag_lazystates:	tag_lazystates.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

tag_unicode:	tag_unicode.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

tag_tidy:	tag_tidy.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

cow:		cow.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)
		./cow < cow.l

cows:		cows.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)
		./cows < cows.l

indent:		indent.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

json:		json.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

csv:		csv.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

braille:	braille.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

unicode:	unicode.l
		$(REFLEX) $(REFLAGS) $<
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ lex.yy.cpp $(LIBREFLEX)

cards:		cards.cpp
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ cards.cpp $(LIBREFLEX)

url:		url.cpp
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I $(BOOSTINC) -o $@ url.cpp $(LIBREFLEX) $(BOOSTLIB)

.PHONY:		clean

clean-local:
		-rm -rf *.dSYM
		-rm -f *.o *.gch *.log
		-rm -f lex.yy.h lex.yy.cpp *.tab.h *.tab.c *.tab.hxx *.tab.cxx reflex.*.cpp reflex.*.gv reflex.*.txt
		-rm -f flexexample? reflexexample?
		-rm -f ctokens jtokens ptokens echo readline calc wc wcu wcpp wcwc tag tag_lazy tag_lazystates tag_unicode tag_tidy cow cows indent json csv braille unicode cards url
