// Reentrant example taken from the Flex documentation

/* This scanner prints "//" comments. */

%x COMMENT

%%

"//"                 push_state(COMMENT);
.|\n

<COMMENT>{
\n                   pop_state();
([^\n]|\\\n)+        out() << text() << std::endl;
}

%%

int main()
{
  Lexer().lex();
  return 0;
}
