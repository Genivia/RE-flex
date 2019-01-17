del *.obj
del example10.exe
del parser.*
del lexer.*
del lex.yy.h

win_bison -d -y example10.parser -o parser.cpp
..\vs\reflex --flex --bison-locations --bison-bridge --header-file=lexer.hpp example10.lexer -o lexer.cpp

cl  /Feexample10.exe parser.cpp lexer.cpp  /I ..\include\ /link ..\vs\reflex.lib