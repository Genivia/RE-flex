// Reentrant example taken from the Flex documentation

/* This scanner prints "//" comments. */

%option reentrant stack noyywrap
%x COMMENT

%%

"//"                 yy_push_state( COMMENT, yyscanner);
.|\n

<COMMENT>\n          yy_pop_state( yyscanner );
<COMMENT>[^\n]+      *yyout << yytext << std::endl;

%%

int main ( int argc, char * argv[] )
{
  yyscan_t scanner;

  yylex_init ( &scanner );
  yylex ( scanner );
  yylex_destroy ( scanner );
  return 0;
}
