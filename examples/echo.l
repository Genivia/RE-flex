%{
#include <stdio.h>
%}
%option noyywrap interactive
%%
.+ ECHO;
%%
int main()
{
  return yyFlexLexer().yylex();
}
