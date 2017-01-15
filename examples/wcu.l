// simple unix-like wc (word count) with unicode
%{
#include <stdio.h>
int ch = 0, wd = 0, nl = 0;
%}

// the unicode option changes \w and . to match unicode
%option main unicode full

nl	\r?\n
wd      (\w|\p{Punctuation})+

%%

{nl}	ch += yyleng; ++nl;
{wd} 	ch += yyleng; ++wd;
.       ch += yyleng;
<<EOF>>	printf("%8d%8d%8d\n", nl, wd, ch); yyterminate();

%%
