
rem example1.l

reflex.exe example.l
cl /Feexample.exe  lex.yy.cpp /EHsc /I ..\include\ /link reflex.lib

rem run

example.exe



