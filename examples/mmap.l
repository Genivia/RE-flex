// Example RE/flex lexer to tokenize a large C/C++ file faster using mmap(2)
// and buffer(b, n) with zero copy overhead.
//
// Lexer method buffer(b, n) scans n-1 bytes at address b.
//
// WARNING: Do not use text(), span(), rest(), that modify the mmap-ed data!!!
//          Use str() or begin() and size() to extract tokens as strings!
//          Also echo() is safe to use.
//
// When text(), span(), rest() are used, memory b[0..n] will be modified and
// b[n] will be set to zero.  Also unput() should be avoided.
//
// WARNING: Do not use original Flex to do the same with yy_scan_buffer,
//          because Flex requires two zero bytes and the mmap-ed buffer will be
//          modified, i.e. Flex yy_scan_buffer cannot be truly read-only.
//
// mmap is the fastest method to scan a file, but no UTF detection, conversion
// or other code page conversions can be applied.  To do so, we first open the
// file and assign it to a reflex::Input to detect if encoded in UTF-16 or
// UTF-32 and if so use reflex::Input to scan the file instead of mmap-ing.
//
// See also ctokens.l for a Flex-like C/C++ tokenizer without mmap(2)

%top{
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
}

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
    FILE *file = fopen(argv[1], "r"); // or use fopen_s()

    if (file != NULL)
    {
      reflex::Input input(file);

      if (input.file_encoding() == reflex::Input::file_encoding::plain)
      {
        int fd = fileno(file);
        struct stat st;

        if (fstat(fd, &st) == 0 && S_ISREG(st.st_mode) && st.st_size <= 4294967295LL)
        {
          size_t size = static_cast<size_t>(st.st_size);
          char *base = (char*)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);

          if (base != MAP_FAILED)
          {
            Lexer lexer;
            lexer.buffer(base, size + 1); // size + 1 to include non-accessed final byte
            lexer.lex();
            munmap((void*)base, size);
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
      }
      else
      {
        Lexer lexer(input);
        lexer.lex();
      }
      fclose(file);
    }
    else
    {
      perror("could not open the specified file");
    }
  }
}
