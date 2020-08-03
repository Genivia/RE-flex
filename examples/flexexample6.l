/* Reentrant example taken from the Flex documentation */
/* Compile: reflex --flex flexexample6.l */

/* This scanner prints "//" comments. */

%option reentrant stack noyywrap

%x COMMENT

%%

"//"                 yy_push_state(COMMENT, yyscanner);
.|\n

<COMMENT>{
\n                   ECHO; yy_pop_state(yyscanner);
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
