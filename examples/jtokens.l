// Java tokenizer, standardized to meet Java lexical structure requirements
// Reads UTF-8 files (with or without UTF-8 BOM) and UTF16 files (with UTF-16 BOM)
// Converts Unicode Escapes \uxxxx first, prior to lexical analysis

%{
#include <reflex/utf8.h>
#include <stdio.h>
static bool is_keyword(const char *);
%}

%i "jdefs.l"
%o dotall unicode
%o fast

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
. printf("** ERROR ** '%s' at line %zu\n", text(), lineno());

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

static size_t convert_unicode_escapes(char *buf, size_t len)
{
  char *t = buf;
  for (const char *s = buf; len > 0; )
  {
    if (s[0] == '\\' && len >= 2)
    {
      if (s[1] == 'u' && len >= 6 )
      {
        unsigned int c;
        sscanf(s + 2, "%4x", &c);
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
    printf("Usage: jtokens FILE\n");
    exit(EXIT_FAILURE);
  }
  reflex::Input in(fopen(argv[1], "r"));
  if (in.file() == NULL)
  {
    perror("Cannot open file for reading");
    exit(EXIT_FAILURE);
  }
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
  len = convert_unicode_escapes(buf, len);
  buf[len] = '\0';
  Lexer(buf).lex();
  delete[] buf;
  fclose(in.file());
  return EXIT_SUCCESS;
}
