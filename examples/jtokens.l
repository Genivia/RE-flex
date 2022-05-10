// Java tokenizer, standardized to meet Java lexical structure requirements
// Reads UTF-8 files (with or without UTF-8 BOM) and UTF-16/32 files (with UTF-16/32 BOM)
// Converts Unicode Escapes \uxxxx first, prior to lexical analysis
//
// Note that identifiers may contain IdentifierIgnorable chars. To remove these,
// use condition states to collect the identifier in a wstring:
// %class{
//   std::wstring id;
// }
// %x ID
// {JavaIdentifierStart}     id.assign(wstr()); start(ID);
// <ID>{JavaIdentifierPart}  id.append(wstr());
// <ID>{IdentifierIgnorable} ;
// <ID>.                     less(size()); start(INITIAL);

%{
#include <reflex/utf8.h>
#include <cstring>
#include <stdio.h>
static bool is_keyword(const char *);
%}

// import Java token definitions
%include "jdefs.l"

// dot matches \n
%option dotall

// enable Unicode pattern matching
%option unicode

// use full/fast option for speed and to eliminate tokenizer startup time
%option full

%%

{IntegerLiteral}       printf("INT         %s\n", text());
{FloatingPointLiteral} printf("FLOAT       %s\n", text());
{CharacterLiteral}     printf("CHARACTER   %s\n", text());
{StringLiteral}        printf("STRING      %s\n", text());
{BooleanLiteral}       printf("BOOL        %s\n", text());
{NullLiteral}          printf("NULL        %s\n", text());
{Separator}            printf("SEPARATOR   %s\n", text());
{Operator}             printf("OPERATOR    %s\n", text());
{Identifier}           if (is_keyword(text()))
                         printf("KEYWORD     %s\n", text());
                       else
                         printf("IDENTIFIER  %s\n", text());
{TraditionalComment}
{EndOfLineComment}
{WhiteSpace}
{Sub}
.                      printf("** ERROR ** '%s' at line %zu\n", text(), lineno());

%%

struct ltstr { bool operator()(const char *s1, const char *s2) const { return strcmp(s1, s2) < 0; } };

static bool is_keyword(const char *text)
{
  static const char *keywords[] = {
    "abstract",
    "continue",
    "for",
    "new",
    "switch",
    "assert",
    "default",
    "if",
    "package",
    "synchronized",
    "boolean",
    "do",
    "goto",
    "private",
    "this",
    "break",
    "double",
    "implements",
    "protected",
    "throw",
    "byte",
    "else",
    "import",
    "public",
    "throws",
    "case",
    "enum",
    "instanceof",
    "return",
    "transient",
    "catch",
    "extends",
    "int",
    "short",
    "try",
    "char",
    "final",
    "interface",
    "static",
    "void",
    "class",
    "finally",
    "long",
    "strictfp",
    "volatile",
    "const",
    "float",
    "native",
    "super",
    "while",
  };
  static std::set<const char*, ltstr> keywordset(keywords, keywords + sizeof(keywords)/sizeof(*keywords));
  static std::set<const char*, ltstr>::const_iterator end = keywordset.end();
  return keywordset.find(text) != end;
}

// Read the file and convert \uxxxx (including UTF-16 surrogate pairs) to UTF-8.
// This is a simple way to do it, by reading the entire input into a buffer.
// There are two other ways to do this without copying the whole file to memory:
// 1. override the reflex::Matcher::input(s, n) method, which passes text to
//    the matcher engine and replace the \uxxxx there.
// 2. create a streambuf class that converts \uxxxx on the fly, similar to the
//    way reflex::BufferedInput::dos_streambuf converts CFLF to LF on the fly.
// The second option is easier to implement than the first.
static size_t convert_unicode_escapes(char *buf, size_t len)
{
  char *t = buf;
  for (const char *s = buf; len > 0; )
  {
    if (s[0] == '\\' && len >= 2)
    {
      if (s[1] == 'u')
      {
        unsigned int c;
        if (len < 6 || sscanf(s + 2, "%4x", &c) == 0)
        {
          printf("Error in \\uxxxx format\n");
          exit(EXIT_FAILURE);
        }
        if (c >= 0xD800 && c < 0xE000)
        {
          s += 6;
          unsigned int d;
          if (len < 6 ||
              s[0] != '\\' ||
              s[1] != 'u' ||
              c > 0xDBFF ||
              sscanf(s + 2, "%4x", &d) == 0 ||
              (d & 0xFC00) != 0xDC00)
          {
            printf("Error in \\uxxxx surrogates\n");
            exit(EXIT_FAILURE);
          }
          c = 0x010000 - 0xDC00 + ((c - 0xD800) << 10) + d;
          len -= 6;
        }
        t += reflex::utf8(c, t);
        s += 6;
        len -= 6;
      }
      else
      {
        *t++ = *s++;
        *t++ = *s++;
        --len;
      }
    }
    else
    {
      *t++ = *s++;
      --len;
    }
  }
  return t - buf;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage: jtokens FILE\n");
    exit(EXIT_FAILURE);
  }
  // open en read the Java source code file
  reflex::Input in(fopen(argv[1], "r"));
  if (in.file() == NULL)
  {
    perror("Cannot open file for reading");
    exit(EXIT_FAILURE);
  }
  // store the Java source code file (UTF-8/16/32 encoded) in a buffer, normalized as UTF-8
  size_t len = in.size();
  if (len == 0)
  {
    printf("Unknown file size\n");
    exit(EXIT_FAILURE);
  }
  char *buf = new char[len + 1];
  if (!in.good() || in.get(buf, len) != len)
  {
    perror("Error in reading");
    exit(EXIT_FAILURE);
  }
  // convert the \uxxxx to UTF-8
  len = convert_unicode_escapes(buf, len);
  buf[len] = '\0';
  // scan the Java source code
  Lexer(buf).lex();
  delete[] buf;
  fclose(in.file());
  return EXIT_SUCCESS;
}
