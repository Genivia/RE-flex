// simple unix-like wc (word count) with lexer class data members

%{
  #include <cstdio>
  #include <iostream>
  #include <iomanip>
  using namespace std;
%}

// Lexer class members
%class{
  int ch, wd, nl;
}

// Lexer class initialization at construction
%init{
  ch = wd = nl = 0;
}

nl  \r?\n
wd  [^ \t\r\n]

%option main

%%

{nl}	ch += yyleng; ++nl;
{wd}+	ch += yyleng; ++wd;
.	++ch;
<<EOF>>	*yyout << setw(8) << nl << setw(8) << wd << setw(8) << ch << endl; yyterminate();

%%
