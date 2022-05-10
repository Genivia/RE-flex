// C++ CSV reader that supports ASCII, ISO-8859-1, and UTF (with BOM) files
// scans one CSV row at a time to populate a vector of string fields, each field contains UTF-8
// Written by Robert van Engelen

%top{
  #include <cstring>
  #include <vector>
}

%option fast freespace unicode
%option lexer=CSV

// CSV lexer class members
%class{
 public:
  void set_delim(int c) { delim = c; } // tab, space, comma, semicolon, or pipe

 private:
  int                      delim; // delimiter character (tab, space, comma, semicolon, or pipe)
  std::string              field; // each field has ASCII/UTF-8 text
  std::vector<std::string> row;   // row of fields

 private:
  void append(int c)            { field.push_back(c); }
  void append(const char *text) { field.append(text); }
  void next_column()            { process_field(); field.clear(); }
  void next_row()               { next_column(); process_row(); row.clear(); }
  void process_field();
  void process_row();
}

// CSV lexer class initialization at construction with constructor argument and initializer
%ctorarg="int ch"
%ctorinit="delim(ch)"

// choice of possible delimiters: tab, space, comma, semicolon, and pipe
delim   [\t ,;|]

// character data of quoted field
chars   ( [^"] | \"\" | \\\p{ASCII} )*

// non-quoted field data (excluding permitted delimiters)
field   ( [^\t\n\r ",;|] | \\\p{ASCII} )*

%%

{delim}         { if (chr() == delim) next_column(); else append(chr()); }

\"{chars}\"     |
{field}         { append(text()); }

[\r\n]+         { next_row(); }

<<EOF>>         { next_row(); return 0; }

.               // ignore invalid character

%%

void CSV::process_field()
{
  // here we could do some cleanup work on the string field, for example:
  // 1. remove enclosing quotes, if present:
  if (field.size() >= 2 && field.at(0) == '"')
  {
    field.erase(0, 1);
    field.erase(field.size() - 1, 1);
  }
  // 2. replace "" by a single ":
  size_t pos = 0;
  while ((pos = field.find("\"\"", pos)) != std::string::npos)
  {
    field.replace(pos, 2, "\"");
    ++pos;
  }
  // 3. remove backslash from escaped characters
  pos = 0;
  while ((pos = field.find("\\", pos)) != std::string::npos)
  {
    field.erase(pos, 1);
    ++pos;
  }
  // 4. remove problematic control characters (to export this data as another CSV)
  const char *control =
    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x0b\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f";
  pos = 0;
  while ((pos = field.find_first_of(control, pos)) != std::string::npos)
    field.erase(pos, 1);
  // 5. trim field by removing leading and trailing space
  const char *whitespace = "\t\n\r ";
  pos = field.find_first_not_of(whitespace);
  if (pos != std::string::npos)
  {
    field.erase(0, pos);
    field.erase(field.find_last_not_of(whitespace) + 1);
  }
  else
  {
    field.clear();
  }
  // 6. replace \t, \r and \n by space
  pos = 0;
  while ((pos = field.find_first_of("\t\n\r", pos)) != std::string::npos)
  {
    field.replace(pos, 1, " ");
    ++pos;
  }
  // add the normalized field to the row
  row.push_back(field);
}

void CSV::process_row()
{
  // print the row of fields, line by line
  for (std::vector<std::string>::const_iterator i = row.begin(); i != row.end(); ++i)
    std::cout << *i << "\n";
  std::cout << "--------------------------------------------------------------------------------" << std::endl;
}

int main(int argc, char **argv)
{
  FILE *fd = stdin;
  int delim = ',';
  // handle arguments
  for (int i = 1; i < argc; ++i)
  {
    if (argv[i][0] == '-')
    {
      if (argv[i][1] == 'd')
      {
        if (argv[i][2] == '\0')
        {
          if (++i < argc)
            delim = argv[i][0];
        }
        else if (argv[i][2] == 't')
        {
          delim = '\t';
        }
        else if (argv[i][2] == 's')
        {
          delim = ' ';
        }
        else
        {
          delim = argv[i][2];
        }
        if (strchr("\t ,;|", delim) == NULL)
        {
          std::cerr << "Invalid delimiter" << std::endl;
          exit(EXIT_FAILURE);
        }
      }
      else
      {
          std::cerr << "Usage: csv [-dt|-ds|-d,|-d;|-d|] [FILE]" << std::endl;
          exit(EXIT_FAILURE);
      }
    }
    else if (fd != stdin)
    {
      std::cerr << "Invalid argument " << argv[i] << std::endl;
      exit(EXIT_FAILURE);
    }
    else if ((fd = fopen(argv[i], "r")) == NULL)
    {
      std::cerr << "Cannot open " << argv[i] << " for reading" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  // create a CSV lexer, assume that CSVs are encoded in ASCII/ISO-8859-1 unless a UTF BOM is present
  CSV csv(delim, reflex::Input(fd, reflex::Input::file_encoding::latin));
  if (csv.lex() != 0)
  {
    std::cerr << "Error in CSV file at line " << csv.lineno() << std::endl;
    exit(EXIT_FAILURE);
  }
  if (fd != stdin)
    fclose(fd);
  return 0;
}
