// example to scan DOS-formatted files using
// reflex::BufferedInput::dos_streambuf to replace CRLF (\r\n) by LF (\n)
// streams do not support UTF-16/32 normalization to UTF-8 though!!
//
// usage:
// $ ./dos somefile.c

%top{
#include <cstdio>
#include <streambuf>
#include <cstring>
}

%option nodefault
%option fast

%%

\n      out() << "<LF>\n";
\r      out() << "<CR>";
\t      out() << "<TAB>";
\s      out() << " ";
\S+     out() << str();

%%

int main(int argc, char **argv)
{
  FILE *file = stdin;
  if (argc > 1)
  {
    file = fopen(argv[1], "rb");
    if (!file)
    {
      perror("Cannot open file for reading\n");
      exit(EXIT_FAILURE);
    }
  }
  reflex::BufferedInput::dos_streambuf streambuf(file);
  std::istream stream(&streambuf);
  Lexer lexer(&stream);
  lexer.lex();
  if (file != stdin)
    fclose(file);
}
