// C/C++ tokenizer (plain, no Unicode but see below)

%{
#include <stdio.h>
%}

/* may enable %option unicode before the %include to match unicode identifiers for C++11 */
// %option unicode

// import C/C++ token definitions
%include "cdefs.l"

%option flex
%option nodefault
%option fast
%option main

%%

{WHITESPACE}
{ILCOMMENT}
{MLCOMMENT}
{DIRECTIVE}	printf("DIRECTIVE %s\n", yytext);
{NAME}		printf("NAME      %s\n", yytext);
{UFLT}		printf("FLOAT     %s\n", yytext);
{UINT}		printf("INT       %s\n", yytext);
{CHAR}		printf("CHAR      %s\n", yytext);
{STRING}	printf("STRING    %s\n", yytext);
"{"|"<%"	|
"}"|"%>"	|
"["|"<:"	|
"]"|":>"	|
"("		|
")"		|
"+="		|
"++"		|
"+"		|
"-="		|
"--"		|
"->*"		|
"->"		|
"-"		|
"=="		|
"="		|
"<="		|
"<<="		|
"<<"		|
"<"		|
">="		|
">>="		|
">>"		|
">"		|
"!="		|
"!"		|
","		|
";"		|
"..."		|
".*"		|
"."		|
"^="		|
"^"		|
"~"		|
"*="		|
"*"		|
"/="		|
"/"		|
"%="		|
"%"		|
"&="		|
"&&"		|
"&"		|
"|="		|
"||"		|
"|"		|
"::"		|
":"		|
"?"		printf("PUNCT     %s\n", yytext);
.		printf("*** ERROR '%s' at line %d\n", yytext, yylineno);

%%
