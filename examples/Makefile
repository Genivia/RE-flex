CPP=c++
BOOST=$(HOME)/boost
BOOSTLIB=$(BOOST)/stage/lib/libboost_regex.a
REFLEX=../bin/reflex
BISON=bison
LIBS=../lib/libreflex.a
# LIBS=../lib/libreflex.a $(BOOSTLIB)
REFLAGS=--matcher=reflex
COFLAGS=-g -O2
CWFLAGS=-Wall -Wunused -Wextra
CIFLAGS=-I. -I../include -I $(BOOST)
CMFLAGS=
# CMFLAGS=-DDEBUG
CFLAGS=$(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS)

all:		flexexample1 flexexample2 flexexample3 flexexample4 flexexample5 flexexample6 flexexample7 flexexample8 reflexexample3 reflexexample4 reflexexample5 reflexexample6 ctokens jtokens ptokens echo wc wcu wcpp wcwc tag tag_lazy tag_lazystates tag_tidy cow cows indent json cards

flexexample1:	flexexample1.l
		$(REFLEX) $(REFLAGS) --flex flexexample1.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)
		./flexexample1 < flexexample1.l

flexexample2:	flexexample2.l
		$(REFLEX) $(REFLAGS) --flex flexexample2.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)
		./flexexample2 < flexexample2.l

flexexample3:	flexexample3.l flexexample3.y
		$(BISON) -d -y flexexample3.y
		$(REFLEX) $(REFLAGS) --flex --bison flexexample3.l
		$(CC) $(CFLAGS) -c y.tab.c
		$(CPP) $(CFLAGS) -o $@ y.tab.o lex.yy.cpp $(LIBS)
		./flexexample3 < flexexample3.test

flexexample4:	flexexample4.l flexexample4.y
		$(BISON) -d -y flexexample4.y
		$(REFLEX) $(REFLAGS) --flex --bison-bridge --header-file flexexample4.l
		$(CPP) $(CFLAGS) -DWITH_NO_C -o $@ y.tab.c lex.yy.cpp $(LIBS)
		./flexexample4 < flexexample4.test

flexexample5:	flexexample5.l flexexample5.y
		$(BISON) -d -y flexexample5.y
		$(REFLEX) $(REFLAGS) --flex --bison-bridge --header-file flexexample5.l
		$(CPP) $(CFLAGS) -o $@ y.tab.c lex.yy.cpp $(LIBS)
		./flexexample5 < flexexample5.test

flexexample6:	flexexample6.l
		$(REFLEX) $(REFLAGS) --flex flexexample6.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)
		./flexexample6 < flexexample6.l

flexexample7:	flexexample7.l flexexample7.y
		$(BISON) -d -y flexexample7.y
		$(REFLEX) $(REFLAGS) --flex --bison-locations flexexample7.l
		$(CC) $(CFLAGS) -c y.tab.c
		$(CPP) $(CFLAGS) -o $@ y.tab.o lex.yy.cpp $(LIBS)
		./flexexample7 < flexexample7.test

flexexample8:	flexexample8.l flexexample8.y
		$(BISON) -d -y flexexample8.y
		$(REFLEX) $(REFLAGS) --flex --bison-locations --bison-bridge --header-file flexexample8.l
		$(CPP) $(CFLAGS) -o $@ y.tab.c lex.yy.cpp $(LIBS)
		./flexexample8 < flexexample8.test

reflexexample3:	reflexexample3.l reflexexample3.y
		$(BISON) -d -y reflexexample3.y
		$(REFLEX) $(REFLAGS) --bison --header-file reflexexample3.l
		$(CPP) $(CFLAGS) -o $@ y.tab.c lex.yy.cpp $(LIBS)
		./reflexexample3 < flexexample3.test

reflexexample4:	reflexexample4.l flexexample4.y
		$(BISON) -d -y flexexample4.y
		$(REFLEX) $(REFLAGS) --bison-bridge --header-file reflexexample4.l
		$(CPP) $(CFLAGS) -DWITH_NO_C -o $@ y.tab.c lex.yy.cpp $(LIBS)
		./reflexexample4 < flexexample4.test

reflexexample5:	reflexexample5.l flexexample5.y
		$(BISON) -d -y flexexample5.y
		$(REFLEX) $(REFLAGS) --bison-bridge --header-file reflexexample5.l
		$(CPP) $(CFLAGS) -o $@ y.tab.c lex.yy.cpp $(LIBS)
		./reflexexample5 < flexexample5.test

reflexexample6:	reflexexample6.l
		$(REFLEX) $(REFLAGS) reflexexample6.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)
		./reflexexample6 < reflexexample6.l

ctokens:	ctokens.l
		$(REFLEX) $(REFLAGS) ctokens.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

jtokens:	jtokens.l
		$(REFLEX) $(REFLAGS) jtokens.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

ptokens:	ptokens.l
		$(REFLEX) $(REFLAGS) ptokens.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

echo:		echo.l
		$(REFLEX) $(REFLAGS) --flex echo.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

wc:		wc.l
		$(REFLEX) $(REFLAGS) --flex wc.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

wcu:		wcu.l
		$(REFLEX) $(REFLAGS) --flex wcu.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

wcpp:		wcpp.l
		$(REFLEX) $(REFLAGS) --flex wcpp.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

wcwc:		wcwc.l
		$(REFLEX) $(REFLAGS) --flex wcwc.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

tag:		tag.l
		$(REFLEX) $(REFLAGS) tag.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

tag_lazy:	tag_lazy.l
		$(REFLEX) $(REFLAGS) tag_lazy.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

tag_lazystates:	tag_lazystates.l
		$(REFLEX) $(REFLAGS) tag_lazystates.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

tag_tidy:	tag_tidy.l
		$(REFLEX) $(REFLAGS) tag_tidy.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

cow:		cow.l
		$(REFLEX) $(REFLAGS) cow.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)
		./cow < cow.l

cows:		cows.l
		$(REFLEX) $(REFLAGS) cows.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)
		./cows < cows.l

indent:		indent.l
		$(REFLEX) $(REFLAGS) indent.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

json:		json.l
		$(REFLEX) $(REFLAGS) json.l
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

cards:		cards.cpp
		$(CPP) $(CFLAGS) -o $@ cards.cpp $(LIBS)

lexer:		lex.yy.cpp
		$(CPP) $(CFLAGS) -o $@ lex.yy.cpp $(LIBS)

.PHONY:		clean
clean:
		-rm -rf *.dSYM
		-rm -f *.o *.gch *.log
		-rm -f lex.yy.h lex.yy.cpp y.tab.h y.tab.c reflex.*.cpp reflex.*.gv reflex.*.txt
		-rm -f flexexample1 flexexample2 flexexample3 flexexample4 flexexample5 flexexample6 flexexample7 flexexample8
		-rm -f reflexexample3 reflexexample4 reflexexample5 reflexexample6
		-rm -f ctokens jtokens ptokens echo wc wcu wcpp wcwc tag tag_lazy tag_lazystates tag_tidy cow cows indent json cards lexer
