// Reentrant example taken from the Flex documentation

/* This scanner prints "//" comments. */

%option reentrant stack noyywrap

%x COMMENT

%%

"//"                 yy_push_state( COMMENT, yyscanner);
.|\n

<COMMENT>{
\n                   pop_state();
([^\n]|\\\n)+        out() << text() << std::endl;
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
