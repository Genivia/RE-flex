/* echo input to output using the readline library (required) */

%top{
  #include <stdlib.h>
  #include <stdio.h>
  #include <readline/readline.h>
  #include <readline/history.h>
%}

%class{
  const char *prompt;
  // we use yywrap() to read the next line
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
    // yywrap() == 0 means OK: wrapped after EOF
    return line == NULL ? 1 : 0;
  }
  // the line returned by readline() without \n
  char *line;
  // the line with \n appended
  std::string linen;
%}

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
%}

%option flex main nodefault

%%

.+  ECHO; // the entire line
\n  ECHO; // end of the line

%%
