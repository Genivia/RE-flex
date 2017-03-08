/* echo input to output in interactive mode */
%{
#include <stdio.h>
%}
%option noyywrap interactive
/* enable the following option to match Unicode characters with . (dot) for ECHO
   %option unicode
*/
%%
.+  ECHO;
%%
int main()
{
  /* Flex C++, otherwise we return yylex() (also when using option --bison) */
  return yyFlexLexer().yylex();
}
