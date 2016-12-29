/* echo input to output in interactive mode */
%{
#include <stdio.h>
%}
%option noyywrap interactive
%%
.+  ECHO;
%%
int main()
{
  return yyFlexLexer().yylex();
}
