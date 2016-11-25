%{
#include <stdio.h>
%}
%option freespace noyywrap interactive
%%
.+  ECHO;
%%
int main()
{
  return yyFlexLexer().yylex();
}
