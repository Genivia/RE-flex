// C/C++ tokenizer (plain, no Unicode)

%{
#include <stdio.h>
%}

/* may add %option unicode before the %include to match unicode identifiers */

%i "cdefs.l"
%o flex
%o nodefault
%o fast
%o main

%%

{WHITESPACE}
{ILCOMMENT}
{MLCOMMENT}
{DIRECTIVE}	printf("DIRECTIVE %s\n", yytext);
{NAME}		printf("NAME      %s\n", yytext);
{INT}		printf("INT       %s\n", yytext);
{FLOAT}		printf("FLOAT     %s\n", yytext);
{CHAR}		printf("CHAR      %s\n", yytext);
{STRING}	printf("STRING    %s\n", yytext);
"+"		|
"+="		|
"++"		|
"-"		|
"-="		|
"--"		|
"->"		|
"="		|
"=="		|
"<"		|
"<="		|
"<<"		|
"<<="		|
">"		|
">="		|
">>"		|
">>="		|
"!"		|
"!="		|
","		|
";"		|
"."		|
"..."		|
"^"		|
"^="		|
"~"		|
"*"		|
"*="		|
"/"		|
"/="		|
"%"		|
"%="		|
"&"		|
"&="		|
"&&"		|
"|"		|
"|="		|
"||"		|
"("		|
")"		|
"["		|
"]"		|
"{"		|
"}"		|
"?"		|
":"		|
"::"		printf("PUNCT     %s\n", yytext);
.		printf("*** ERROR '%s' at line %d\n", yytext, yylineno);

%%
