// Example RE/flex lexer to tokenize a large C/C++ file faster using mmap(2)
// and buffer(b, n) with zero copy overhead.
//
// Lexer method buffer(b, n) scans n-1 bytes at address b.
//
// WARNING: Do not use text() and rest(), which modify the mmap-ed data!!!
//          Use str() or begin() and size() to extract tokens as strings!
//
// When text() or rest() are used, memory b[0..n] will be modified and b[n]
// will be set to zero.  Also unput() should be avoided.  But echo() is safe.
//
// WARNING: Do not use original Flex to do the same with yy_scan_buffer,
//          because Flex requires two zero bytes and the mmap-ed buffer will be
//          modified, i.e. Flex yy_scan_buffer cannot be truly read-only.
//
// This is the fastest method to scan a file, but no UTF detection, conversion
// or other code page conversions can be applied.  To do so, we should open the
// file first and assign it to a reflex::Input to detect if encoded in UTF-16
// or UTF-32 and if so use reflex::Input to scan the file instead of mmap-ing.
//
// See also ctokens.l for a Flex-like C/C++ tokenizer without mmap(2)

%top{
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
%}

/* may add %option unicode before the %include to match unicode identifiers */

%include "cdefs.l"
%option nodefault
%option fast

%%

{WHITESPACE}
{ILCOMMENT}
{MLCOMMENT}
{DIRECTIVE}	out() << "DIRECTIVE " << str() << std::endl;
{NAME}		out() << "NAME      " << str() << std::endl;
{UFLT}		out() << "FLOAT     " << str() << std::endl;
{UINT}		out() << "INT       " << str() << std::endl;
{CHAR}		out() << "CHAR      " << str() << std::endl;
{STRING}	out() << "STRING    " << str() << std::endl;
"{"|"<%"	|
"}"|"%>"	|
"["|"<:"	|
"]"|":>"	|
"("		|
")"		|
"+="		|
"++"		|
"+"		|
"-="		|
"--"		|
"->*"		|
"->"		|
"-"		|
"=="		|
"="		|
"<="		|
"<<="		|
"<<"		|
"<"		|
">="		|
">>="		|
">>"		|
">"		|
"!="		|
"!"		|
","		|
";"		|
"..."		|
".*"		|
"."		|
"^="		|
"^"		|
"~"		|
"*="		|
"*"		|
"/="		|
"/"		|
"%="		|
"%"		|
"&="		|
"&&"		|
"&"		|
"|="		|
"||"		|
"|"		|
"::"		|
":"		|
"?"		out() << "PUNCT     " << str() << std::endl;
.		out() << "*** ERROR at line " << lineno() << std::endl;

%%

int main(int argc, char **argv)
{
  if (argc >= 2)
  {
    int fd = open(argv[1], O_RDONLY);

    if (fd >= 0)
    {
      struct stat st;
      int status = fstat(fd, &st);

      if (status >= 0)
      {
        size_t size = st.st_size;
        char *base = (char*)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);

        if (base != NULL)
        {
          Lexer lexer;
          lexer.buffer(base, size + 1);
          lexer.lex();
        }
        else
        {
          perror("could not mmap the specified file");
        }
      }
      else
      {
        perror("could not stat the specified file");
      }
      close(fd);
    }
    else
    {
      perror("could not open the specified file");
    }
  }
}
