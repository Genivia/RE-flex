// An interactive calculator with Unicode identifier variables
// Builds with bison-bridge to pass Lexer object 'lexer' to bison parser
// $ reflex calc.l
// $ bison -y -d calc.y
// $ c++ -o calc y.tab.c lex.yy.cpp -lreflex
// Example:
// $ ./calc
// π = 3.14
// => 3.14
// π/2
// => 1.57

%{
  #include "lex.yy.h"
  #define YYLEX_PARAM lexer
  void yyerror(Lexer *lexer, const char *msg);
%}

%pure-parser
%lex-param { Lexer *lexer }
%parse-param { Lexer *lexer }

%union {
  std::wstring *var;
  double        num;
}

%type <num> expr
%type <var> 'V'
%type <num> 'N'

%right '='
%left '+' '-'
%left '*' '/'
%right NEG

%%

line : line calc '\n'
     | line '\n'          { lexer->out() << "? "; }
     |
     ;
calc : expr               { lexer->out() << "=> " << $1 << std::endl; }
     ;
expr : 'V' '=' expr       { $$ = lexer->map[*$1] = $3; }
     | expr '+' expr      { $$ = $1 + $3; }
     | expr '-' expr      { $$ = $1 - $3; }
     | expr '*' expr      { $$ = $1 * $3; }
     | expr '/' expr      { $$ = $1 / $3; }
     | '-' %prec NEG expr { $$ = -$2; }
     | '(' expr ')'       { $$ = $2; }
     | 'V'                { $$ = lexer->map[*$1]; }
     | 'N'                { $$ = $1; }
     ;

%%

int main()
{
  Lexer lexer(stdin, std::cout);
  return yyparse(&lexer);
}

void yyerror(Lexer *lexer, const char *msg)
{
  fprintf(stderr, "%s at %zu,%zu\n", msg, lexer->lineno(), lexer->columno());
}
