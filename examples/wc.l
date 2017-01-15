/* simple unix-like wc (word count) */
%{
#include <stdio.h>
int ch = 0, wd = 0, nl = 0;
%}

nl	\r?\n
wd	[^ \t\r\n]+

%option main

%%

{nl}	ch += yyleng; ++nl;
{wd} 	ch += yyleng; ++wd;
.	++ch;
<<EOF>>	printf("%8d%8d%8d\n", nl, wd, ch); yyterminate();

%%
