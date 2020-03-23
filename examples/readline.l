/* echo input to output using the readline library */

%top{
  #include <stdlib.h>
  #include <stdio.h>
  #include <readline/readline.h>
  #include <readline/history.h>
}

// yyFlexLexer class members
%class{
  const char *prompt;
#if REFLEX_OPTION_flex == true
  // yywrap() in Flex mode (%option flex) to read the next line
  virtual int yywrap() {
    if (line)
    {
      free((void*)line);
      line = readline(prompt);
      if (line != NULL)
      {
        if (*line)
          add_history(line);
        linen.assign(line).push_back('\n');
        in(linen);
      }
    }
    // IMPORTANT: yywrap() == 0 means OK: wrapped after EOF
    return line != NULL ? 0 : 1;
  }
#else
  // when not using Flex mode, wrap() should be defined as follows
  virtual bool wrap() {
    if (line)
    {
      free((void*)line);
      line = readline(prompt);
      if (line != NULL)
      {
        if (*line)
          add_history(line);
        linen.assign(line).push_back('\n');
        in(linen);
      }
    }
    // IMPORTANT: wrap() == true means OK: wrapped after EOF
    return line != NULL;
  }
#endif
  // the line returned by readline() without \n
  char *line;
  // the line with \n appended
  std::string linen;
}

// yyFlexLexer class initialization at construction
%init{
  prompt = NULL;
  line = readline(prompt);
  if (line != NULL)
  {
    if (*line)
      add_history(line);
    linen.assign(line).push_back('\n');
  }
  in(linen);
}

%option flex main nodefault

%%

.+  ECHO; // the entire line
\n  ECHO; // end of the line

%%
