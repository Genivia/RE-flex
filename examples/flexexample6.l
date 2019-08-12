// Reentrant example taken from the Flex documentation

/* This scanner prints "//" comments. */

%option flex reentrant stack noyywrap

%x COMMENT

%%

"//"                 yy_push_state(COMMENT, yyscanner);
.|\n

<COMMENT>{
\n                   yy_pop_state(yyscanner);
([^\n]|\\\n)+        ECHO;
}

%%

int main()
{
  yyscan_t scanner;

  yylex_init ( &scanner );
  yylex ( scanner );
  yylex_destroy ( scanner );
  return 0;
}
