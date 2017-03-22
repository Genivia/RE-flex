/* echo input to output in interactive mode */
%{
#include <stdio.h>
%}
%option noyywrap interactive
/*
   enable the following option to match Unicode characters with . (dot):
   %option unicode
   but dot also matches invalid UTF-8 (e.g. overruns). To match valid Unicode
   only, use \p{Unicode} instead of dot.
*/
%%
.+  ECHO;
%%
int main()
{
  /* Flex C++ yyFlexLexer, or we can return yylex() when using option --bison */
  return yyFlexLexer().yylex();
}
