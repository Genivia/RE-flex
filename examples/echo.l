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
  // Flex C++, otherwise we return yylex() (also when using option --bison)
  return yyFlexLexer().yylex();
}
