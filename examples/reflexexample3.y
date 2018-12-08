/* Parser to convert "C" assignments to lisp using Bison in C. */
/* Compile: bison -d -y reflexexample3.y */

%{
#include <stdio.h>
#include "lex.yy.h"
void yyerror(const char*);
%}

%union {
    int   num;
    char *str;
}

%token <str> STRING
%token <num> NUMBER

%%

assignments : assignment
            | assignment assignments
            ;
assignment  : STRING '=' NUMBER ';' { printf("(setf %s %d)\n", $1, $3); }
            ;

%%

int main()
{
  yyparse();
  return 0;
}

void yyerror(const char *msg)
{
  fprintf(stderr, "%s at line %zu\n", msg, YY_SCANNER.matcher().lineno()); /* reflex-generated global `YY_SCANNER` */
}
