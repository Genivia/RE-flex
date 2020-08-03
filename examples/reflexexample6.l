// Reentrant example taken from the Flex documentation

/* This scanner prints "//" comments. */

%x COMMENT

%%

"//"                 push_state(COMMENT);
.|\n

<COMMENT>{
\n                   echo(); pop_state();
([^\n]|\\\n)+        echo();
}

%%

int main()
{
  Lexer().lex();
  return 0;
}
