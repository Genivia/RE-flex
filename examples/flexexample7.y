/* Parser to convert "C" assignments to lisp. */
/* Compile: bison -d -y flexexample7.y */

%{
#include <stdio.h>
void yyerror(const char*);
%}

%locations

%union {
    int num;
    char* str;
}

%{
/* patches old bison versions that don't produce correct YYLEX */
extern int yylex(YYSTYPE*, YYLTYPE*);
#define YYLEX_PARAM &yylval, &yylloc
%}

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
  fprintf(stderr, "%s at (%d,%d) to (%d,%d)\n", msg, yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column);
}
