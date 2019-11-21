/* echo input to output in interactive mode */
%{
#include <stdio.h>
%}
%option fast flex noyywrap interactive
/*
   This example uses the implicit "default rule" to echo newlines, which are
   not matched by . (dot).
   Enable the following option to match Unicode characters with . (dot):
   %option unicode
   but dot also matches invalid UTF-8 (e.g. overruns). To match valid Unicode
   only, use \p{Unicode} (or its alternative \X) instead of dot.
   To run ./echo on an EBCDIC file, change the body of main():
      reflex::Input input(stdin, reflex::Input::file_encoding::ebcdic);
      return yyFlexLexer(input).yylex();
   For ISO-8859-1 use reflex::Input::file_encoding::latin
   Interactive input is more user friendly with libreadline, see readline.l
*/
%%
.+  ECHO;
%%
int main()
{
  /* Flex C++ yyFlexLexer, or we can return yylex() when using option --bison */
  return yyFlexLexer().yylex();
}
