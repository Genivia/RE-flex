// simple unix-like wc (word count) with unicode
%{
#include <stdio.h>
int ch = 0, wd = 0, nl = 0;
%}

nl	\r?\n
wd      \w|[^ \t\r\n]

// the unicode option changes \w to match unicode word chars
%option main unicode

%%

{nl}	ch += yyleng; ++nl;
{wd}+	ch += yyleng; ++wd;
.       ++ch;
<<EOF>>	printf("%8d%8d%8d\n", nl, wd, ch); yyterminate();

%%
