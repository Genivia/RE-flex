/******************************************************************************\
* Copyright (c) 2016, Robert van Engelen, Genivia Inc. All rights reserved.    *
*                                                                              *
* Redistribution and use in source and binary forms, with or without           *
* modification, are permitted provided that the following conditions are met:  *
*                                                                              *
*   (1) Redistributions of source code must retain the above copyright notice, *
*       this list of conditions and the following disclaimer.                  *
*                                                                              *
*   (2) Redistributions in binary form must reproduce the above copyright      *
*       notice, this list of conditions and the following disclaimer in the    *
*       documentation and/or other materials provided with the distribution.   *
*                                                                              *
*   (3) The name of the author may not be used to endorse or promote products  *
*       derived from this software without specific prior written permission.  *
*                                                                              *
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF         *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO   *
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,       *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, *
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;  *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,     *
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR      *
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF       *
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                   *
\******************************************************************************/

/**
@file      reflex.cpp
@brief     RE/flex scanner generator replacement for Flex/Lex
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2017, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#include "reflex.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Unicode scripts                                                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "letter_scripts.h"
#include "language_scripts.h"
#include "pl_scripts.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Static data                                                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// Command-line reflex options and .l spec %options
static const char *options_table[] = {
  "array",
  "always_interactive",
  "batch",
  "bison",
  "bison_bridge",
  "bison_locations",
  "case_insensitive",
  "class",
  "debug",
  "default",
  "dotall",
  "extra_type",
  "fast",
  "flex",
  "freespace",
  "full",
  "graphs_file",
  "header_file",
  "input",
  "interactive",
  "lex",
  "lex-compat",
  "lexer",
  "main",
  "matcher",
  "namespace",
  "never_interactive",
  "noarray",
  "nodebug",
  "nodefault",
  "nodotall",
  "noinput",
  "noline",
  "nomain",
  "nopointer",
  "nostack",
  "nostdinit",
  "nounistd",
  "nounput",
  "nowarn",
  "noyylineno",
  "noyymore",
  "noyywrap",
  "outfile",
  "pattern",
  "pointer",
  "posix",
  "prefix",
  "reentrant",
  "regexp_file",
  "stack",
  "stdinit",
  "stdout",
  "tables_file",
  "tabs",
  "unicode",
  "unput",
  "verbose",
  "warn",
  "yyclass",
  "yylineno",
  "yymore",
  "yywrap",
  "7bit",
  "8bit",
  NULL
};

#ifdef OS_WIN
static const char *newline = "\r\n";
#else
static const char *newline = "\n";
#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Inline helper functions                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// Check word character.
/// @returns nonzero if argument c is in [A-Za-z0-9_], zero otherwise.
inline int isword(int c)
{
  return isalnum(c) || c == '_';
}

/// Check alphabetical or underscore character.
/// @returns nonzero if argument c is in [A-Za-z_], zero otherwise.
inline int isalphaun(int c)
{
  return isalpha(c) || c == '_';
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Static helper functions                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// Display version information and exit.
static void version(void)
{
  std::cout << "reflex " REFLEX_VERSION " " PLATFORM << std::endl;
  exit(EXIT_SUCCESS);
}

/// Display help information with an optional diagnostic message and exit.
static void help(const char *message = NULL, const char *arg = NULL)
{
  if (message)
    std::cerr
      << "reflex: "
      << message
      << (arg != NULL ? arg : "")
      << std::endl;
  std::cerr << "Usage: reflex [OPTIONS] [FILE]\n\
\n\
   Scanner:\n\
        -+, --flex\n\
                generate Flex-compatible C++ scanner\n\
        -a, --dotall\n\
                dot in patterns match newline\n\
        -B, --batch\n\
                generate scanner for batch input by reading the entire input\n\
        -f, --full\n\
                generate full scanner with FSM opcode tables\n\
        -F, --fast\n\
                generate fast scanner with FSM code\n\
        -i, --case-insensitive\n\
                ignore case in patterns\n\
        -I, --interactive, --always-interactive\n\
                generate interactive scanner\n\
        -m NAME, --matcher=NAME\n\
                use matcher NAME library [reflex|boost|boost-perl|...]\n\
        --pattern=NAME\n\
                use custom pattern class NAME with custom matcher option -m\n\
        -u, --unicode\n\
                . (dot), \\s, and \\w match unicode, groups UTF-8 characters\n\
        -x, --freespace\n\
                ignore space in patterns\n\
\n\
   Generated code:\n\
        --namespace=NAME\n\
                namespace NAME for generated scanner class\n\
        --lexer=NAME\n\
                lexer class NAME instead of Lexer or yyFlexLexer\n\
        --lex=NAME\n\
                lex function NAME instead of lex or yylex\n\
        --class=NAME\n\
                user-defined class NAME\n\
        --yyclass=NAME\n\
                generate Flex-compatible scanner with user-defined class NAME\n\
        --main\n\
                generate main() to invoke lex() or yylex()\n\
        -L, --noline\n\
                suppress #line directives in scanner\n\
        -P NAME, --prefix=NAME\n\
                use NAME as prefix to FlexLexer class name and its members\n\
        --nostdinit\n\
                initialize input to std::cin instead of stdin\n\
        --bison\n\
                generate global yylex() scanner for bison\n\
        --bison-bridge\n\
                generate reentrant yylex() scanner for bison pure parser\n\
        --bison-locations\n\
                include bison yylloc support\n\
        -R, --reentrant\n\
                generate Flex-compatible yylex() reentrant scanner functions\n\
                NOTE: adds functions only, reflex scanners are always reentrant\n\
\n\
   Files:\n\
        -o FILE, --outfile=FILE\n\
                specify output FILE instead of lex.yy.cpp\n\
        -t, --stdout\n\
                write scanner on stdout instead of lex.yy.cpp\n\
        --graphs-file[=FILE]\n\
                write the scanner's DFA to FILE.gv for the Graphviz dot tool\n\
        --header-file[=FILE]\n\
                write a C++ header FILE.h in addition to the scanner\n\
        --regexp-file[=FILE]\n\
                write the scanner's regular expression patterns to FILE.txt\n\
        --tables-file[=FILE]\n\
                write the scanner's FSM opcode tables or FSM code to FILE.cpp\n\
\n\
   Debugging:\n\
        -d, --debug\n\
                enable debug mode in scanner\n\
        -s, --nodefault\n\
                suppress default rule that ECHOs unmatched text\n\
        -v, --verbose\n\
                display a summary of scanner statistics to stdout\n\
        -w, --nowarn\n\
                do not generate warnings\n\
\n\
   Miscellaneous:\n\
        -c      do-nothing POSIX option\n\
        -n      do-nothing POSIX option\n\
        -?, -h, --help\n\
                display this help message\n\
        -V, --version\n\
                display reflex release version\n\
\n\
   Lex/Flex-compatible options that are enabled by default or have no effect:\n\
        --c++                  default\n\
        --lex-compat           n/a\n\
        --never-interactive    default\n\
        --nounistd             n/a\n\
        --noyywrap             n/a\n\
        --posix                n/a\n\
        --stack                n/a\n\
        --warn                 default\n\
        --yylineno             default\n\
        --yymore               default\n\
        --yywrap               default\n\
        --7bit                 n/a\n\
        --8bit                 n/a\n\
" << std::endl;
  exit(message ? EXIT_FAILURE : EXIT_SUCCESS);
}

/// Abort with an error message.
static void abort(const char *error, const char *arg = NULL)
{
  std::cerr
    << "reflex: error "
    << error
    << (arg != NULL ? arg : "")
    << std::endl;
  exit(EXIT_FAILURE);
}

/// Add file extension if not present.
static std::string file_ext(std::string& name, const char *ext)
{
  size_t n = name.size();
  size_t m = strlen(ext);
  if (n > m && (name.at(n - m - 1) != '.' || name.compare(n - m, m, ext) != 0))
    name.append(".").append(ext);
  return name;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Main                                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// Main program instantiates Reflex class and runs Reflex::main(argc, argv).
int main(int argc, char **argv)
{
  Reflex().main(argc, argv);
  return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Reflex class public methods                                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// Main program.
void Reflex::main(int argc, char **argv)
{
  init(argc, argv);
  parse();
  write();
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Reflex class private/protected methods                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// Reflex initialization.
void Reflex::init(int argc, char **argv)
{
  for (const char *const *i = options_table; *i != NULL; ++i)
    options[*i] = "";

  // first add scripts for letters, languages, etc.
  letter_scripts(scripts);
  language_scripts(scripts);

  // ASCII and Unicode ranges
  scripts["ASCII"]                  = reflex::utf8(0x00, 0x7f);
  scripts["Unicode"]                = reflex::utf8(0x00, 0x10ffff);
  scripts["Non_ASCII_Unicode"]      = reflex::utf8(0x80, 0x10ffff);

  // basic Unicode properties
  scripts["Letter"]                 = scripts["L"]
                                    = scripts["L&"] + "|"
                                    + scripts["Lm"] + "|"
                                    + scripts["Lo"];
  scripts["Mark"]                   = scripts["M"]
                                    = scripts["Mn"] + "|"
                                    + scripts["Mc"] + "|"
                                    + scripts["Me"];
  scripts["Separator"]              = scripts["Z"]
                                    = scripts["Zs"] + "|"
                                    + scripts["Zl"] + "|"
                                    + scripts["Zp"];
  scripts["Symbol"]                 = scripts["S"]
                                    = scripts["Sm"] + "|"
                                    + scripts["Sc"] + "|"
                                    + scripts["Sk"] + "|"
                                    + scripts["So"];
  scripts["Number"]                 = scripts["N"]
                                    = scripts["Nd"] + "|"
                                    + scripts["Nl"] + "|"
                                    + scripts["No"];
  scripts["Punctuation"]            = scripts["P"]
                                    = scripts["Pd"] + "|"
                                    + scripts["Ps"] + "|"
                                    + scripts["Pe"] + "|"
                                    + scripts["Pi"] + "|"
                                    + scripts["Pf"] + "|"
                                    + scripts["Pc"] + "|"
                                    + scripts["Po"];
  scripts["Other"]                  = scripts["C"] // do not include unassigned Cn
                                    = scripts["Cc"] + "|"
                                    + scripts["Cf"];

  // basic Unicode sub-properties
  scripts["Lowercase_Letter"]       = scripts["Ll"];
  scripts["Uppercase_Letter"]       = scripts["Lu"];
  scripts["Titlecase_Letter"]       = scripts["Lt"];
  scripts["Modifier_Letter"]        = scripts["Lm"];
  scripts["Other_Letter"]           = scripts["Lo"];
  scripts["Non_Spacing_Mark"]       = scripts["Mn"];
  scripts["Spacing_Combining_Mark"] = scripts["Mc"];
  scripts["Enclosing_Mark"]         = scripts["Me"];
  scripts["Space_Separator"]        = scripts["Zs"];
  scripts["Line_Separator"]         = scripts["Zl"];
  scripts["Paragraph_Separator"]    = scripts["Zp"];
  scripts["Math_Symbol"]            = scripts["Sm"];
  scripts["Currency_Symbol"]        = scripts["Sc"];
  scripts["Modifier_Symbol"]        = scripts["Sk"];
  scripts["Other_Symbol"]           = scripts["So"];
  scripts["Decimal_Digit_Number"]   = scripts["Nd"];
  scripts["Letter_Number"]          = scripts["Nl"];
  scripts["Other_Number"]           = scripts["No"];
  scripts["Dash_Punctuation"]       = scripts["Pd"];
  scripts["Open_Punctuation"]       = scripts["Ps"];
  scripts["Close_Punctuation"]      = scripts["Pe"];
  scripts["Initial_Punctuation"]    = scripts["Pi"];
  scripts["Final_Punctuation"]      = scripts["Pf"];
  scripts["Connector_Punctuation"]  = scripts["Pc"];
  scripts["Other_Punctuation"]      = scripts["Po"];
  scripts["Control"]                = scripts["Cc"];
  scripts["Format"]                 = scripts["Cf"];

  // now add scripts for some common programming languages
  pl_scripts(scripts);

  conditions.push_back("INITIAL");
  inclusive.insert(0);
  in = &std::cin;
  out = &std::cout;
  lineno = 0;

  for (int i = 1; i < argc; ++i)
  {
    const char *arg = argv[i];
    if (*arg == '-'
#ifdef OS_WIN
     || *arg == '/'
#endif
     )
    {
      bool is_grouped = true;
      while (is_grouped && *++arg)
      {
        switch (*arg)
        {
          case '-':
            ++arg;
            if (strcmp(arg, "help") == 0)
              help();
            if (strcmp(arg, "version") == 0)
              version();
            if (strcmp(arg, "c++") != 0)
            {
              const char *val = strchr(arg, '=');
              size_t len = strlen(arg);
              if (val != NULL)
                len = val - arg;
              std::string name(arg, len);
              size_t pos;
              while ((pos = name.find('-')) != std::string::npos)
                name[pos] = '_';
              Map::iterator i = options.find(name);
              if (i == options.end())
                help("unknown option --", arg);
              if (val != NULL)
                i->second = val + 1;
              else
                i->second = "true";
            }
            is_grouped = false;
            break;
          case '+':
            options["flex"] = "true";
            break;
          case 'a':
            options["dotall"] = "true";
            break;
          case 'B':
            options["batch"] = "true";
            break;
          case 'c':
            break;
          case 'd':
            options["debug"] = "1";
            break;
          case 'f':
            options["full"] = "true";
            break;
          case 'F':
            options["fast"] = "true";
            break;
          case '?':
          case 'h':
            help();
            break;
          case 'I':
            options["interactive"] = "true";
            break;
          case 'i':
            options["case_insensitive"] = "true";
            break;
          case 'l':
            options["lex-compat"] = "true";
            break;
          case 'L':
            options["noline"] = "true";
            break;
          case 'm':
            ++arg;
            if (*arg)
              options["matcher"] = &arg[*arg == '='];
            else if (i < argc && argv[++i] && *argv[i] != '-')
              options["matcher"] = argv[i];
            else
              help("missing NAME with -m NAME");
            is_grouped = false;
            break;
          case 'n':
            break;
          case 'o':
            ++arg;
            if (*arg)
              options["outfile"] = &arg[*arg == '='];
            else if (i < argc && argv[++i] && *argv[i] != '-')
              options["outfile"] = argv[i];
            else
              help("missing FILE with -o FILE");
            is_grouped = false;
            break;
          case 'P':
            ++arg;
            if (*arg)
              options["prefix"] = &arg[*arg == '='];
            else if (i < argc && argv[++i] && *argv[i] != '-')
              options["prefix"] = argv[i];
            else
              help("missing NAME with -P NAME");
            is_grouped = false;
            break;
          case 'R':
            options["reentrant"] = "true";
            break;
          case 's':
            options["nodefault"] = "true";
            break;
          case 't':
            options["stdout"] = "true";
            break;
          case 'u':
            options["unicode"] = "true";
            break;
          case 'V':
            version();
            break;
          case 'v':
            options["verbose"] = "true";
            break;
          case 'w':
            options["nowarn"] = "true";
            break;
          case 'x':
            options["freespace"] = "true";
            break;
          case 'X':
            options["posix"] = "true";
            break;
          default:
            help("unknown option -", arg);
        }
      }
    }
    else
    {
      if (!infile.empty())
        help("one FILE, also found ", argv[i]);
      infile = argv[i];
    }
  }
}

/// Parse lex specification input.
void Reflex::parse(void)
{
  std::ifstream ifs;
  if (!infile.empty())
  {
    ifs.open(infile.c_str(), std::ifstream::in);
    if (!ifs.is_open())
      abort("opening file ", infile.c_str());
    in = &ifs;
  }
  parse_section_1();
  parse_section_2();
  parse_section_3();
  if (ifs.is_open())
    ifs.close();
}

/// Parse %include file.
void Reflex::include(const std::string& filename)
{
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ifstream::in);
  if (!ifs.is_open())
    abort("opening file ", infile.c_str());
  std::string save_infile = infile;
  infile = filename;
  std::istream *save_in = in;
  in = &ifs;
  size_t save_lineno = lineno;
  lineno = 0;
  parse_section_1();
  if (ifs.is_open())
    ifs.close();
  infile = save_infile;
  in = save_in;
  lineno = save_lineno;
}

/// Fetch next line from the input, return true if ok.
bool Reflex::getline(void)
{
  if (in->eof())
    return false;
  if (!in->good())
    abort("in reading");
  ++lineno;
  std::getline(*in, line);
  linelen = line.length();
  while (linelen > 0 && isspace(line.at(linelen - 1)))
    --linelen;
  line.resize(linelen);
  if (in->eof() && line.empty())
    return false;
  return true;
}

/// Advance pos over white space and comments, return true if ok.
bool Reflex::skipcomment(size_t& pos)
{
  while (true)
  {
    (void)ws(pos);
    if (pos + 1 < linelen && line.at(pos) == '/' && line.at(pos + 1) == '/')
    {
      pos = linelen;
    }
    else if (pos + 1 < linelen && line.at(pos) == '/' && line.at(pos + 1) == '*')
    {
      while (true)
      {
        while (pos + 1 < linelen && (line.at(pos) != '*' || line.at(pos + 1) != '/'))
          ++pos;
        if (pos + 1 < linelen)
          break;
        if (!getline())
          return false;
        pos = 0;
      }
      pos += 2;
      if (pos >= linelen)
      {
        if (!getline())
          return false;
        pos = 0;
      }
      continue;
    }
    if (pos < linelen)
      return true;
    if (!getline())
      return false;
    pos = 0;
  }
}

/// Advance pos to match case-insensitive string s followed by whitespace, return true if OK.
bool Reflex::as(size_t& pos, const char *s)
{
  if (pos >= linelen || std::tolower(line.at(pos)) != *s++)
    return false;
  do
    ++pos;
  while (pos < linelen && std::tolower(line.at(pos)) == *s++);
  return ws(pos);
}

/// Advance pos over whitespace, returns true if OK.
bool Reflex::ws(size_t& pos)
{
  if (pos >= linelen || (pos > 0 && !isspace(line.at(pos))))
    return false;
  while (pos < linelen && isspace(line.at(pos)))
    ++pos;
  return true;
}

/// Advance pos over '=' and whitespace, return true if OK.
bool Reflex::eq(size_t& pos)
{
  (void)ws(pos);
  if (pos + 1 >= linelen || line.at(pos) != '=')
    return false;
  ++pos;
  (void)ws(pos);
  return true;
}

/// Advance pos to end of line while skipping whitespace, return true if end of line.
bool Reflex::nl(size_t& pos)
{
  while (pos < linelen && isspace(line.at(pos)))
    ++pos;
  return pos >= linelen;
}

/// Check if current line starts a block of code or a comment.
bool Reflex::iscode(void)
{
  return linelen > 0 && ((isspace(line.at(0)) && options["freespace"].empty()) || line == "%{" || !line.compare(0, 2, "//") || !line.compare(0, 2, "/*"));
}

/// Check if current line starts a block of %top code.
bool Reflex::istopcode(void)
{
  size_t pos = 0;
  return line == "%top{" || (as(pos, "%top") && pos < linelen && line.at(pos) == '{');
}

/// Check if current line starts a block of %class code.
bool Reflex::isclasscode(void)
{
  size_t pos = 0;
  return line == "%class{" || (as(pos, "%class") && pos < linelen && line.at(pos) == '{');
}

/// Check if current line starts a block of %init code.
bool Reflex::isinitcode(void)
{
  size_t pos = 0;
  return line == "%init{" || (as(pos, "%init") && pos < linelen && line.at(pos) == '{');
}

/// Advance pos over name, return name.
std::string Reflex::getname(size_t& pos)
{
  if (pos >= linelen || !isword(line.at(pos)))
    return "";
  size_t loc = pos++;
  while (pos < linelen)
  {
    if (line.at(pos) == '-') // convert - to _
      line[pos] = '_';
    else if (!isword(line.at(pos)))
      break;
    ++pos;
  }
  return line.substr(loc, pos - loc);
}

/// Advance pos over quoted string, return string.
std::string Reflex::getstring(size_t& pos)
{
  if (pos >= linelen || line.at(pos) != '"')
    return "";
  size_t loc = ++pos;
  while (pos < linelen)
  {
    if (line.at(pos) == '"')
      break;
    ++pos;
  }
  return line.substr(loc, pos - loc);
}

// convert (X) to (?:X) for regex engines to ignore groups
// convert lookahead X/Y to X(?=Y)
// convert . to UTF-8 (.|[\xc0-\xff][\x80-\xbf]+) if unicode flag is set
// convert \s, \l, \u, \w to UTF-8 pattern if unicode flag is set
// convert \X to UTF-8 pattern
// convert UTF-8-sequence to (?:UTF-8-sequence) if unicode flag is set
// convert \p{Script} to UTF-8
// convert \p{IsScript} to UTF-8
// convert \u{xxxx} and \x{xxxx} to UTF-8
// convert [...\u{xxxx}...\p{Script}...] => [...]|\u{xxxx}|\p{Script}
// convert # to \# if freespace flag is set
// permit line continuation with \ and indent at end of regex
std::string Reflex::getregex(size_t& pos)
{
  std::string regex;
  size_t loc = pos;
  size_t lev = 0;
  size_t lap = 0;
  bool blk = pos == 0;
  while (pos < linelen)
  {
    int c = line.at(pos);
    if (isspace(c) && options["freespace"].empty())
    {
      if (!blk && lev == 0)
        break;
      if (blk)
        warning("regular expression opens with white space");
    }
    else if (
        !blk &&
        !options["freespace"].empty() &&
        ( (c == '{' && (pos + 1 == linelen || isspace(line.at(pos + 1)))) ||
          (c == '|' && pos + 1 == linelen) ||
          (c == '/' && pos + 1 < linelen && (line.at(pos + 1) == '/' || line.at(pos + 1) == '*'))
        ))
    {
      --pos;
      break;
    }
    else
    {
      switch (c)
      {
        case '\\':
          if (pos + 1 < linelen)
          {
            ++pos;
            if (line.at(pos) == '"')
            {
              regex.append(line.substr(loc, pos - loc - 1)).append("\""); // translate \"
              loc = pos + 1;
            }
            else if (line.at(pos) == 'Q')
            {
              while (pos + 2 < linelen && (line.at(++pos) != '\\' || line.at(pos + 1) != 'E'))
                continue;
              if (pos >= linelen || line.at(pos) != '\\')
                error("missing closing \\E: ", line.substr(loc).c_str());
            }
            else if (pos + 2 < linelen && line.at(pos) == 'p' && line.at(pos + 1) == '{') // translate \p{X}
            {
              size_t k = pos;
              while (k + 1 < linelen && line.at(++k) != '}')
                continue;
              std::string name;
              if (pos + 4 < linelen && line.at(pos + 2) == 'I' && line.at(pos + 3) == 's')
                name = line.substr(pos + 4, k - pos - 4);
              else
                name = line.substr(pos + 2, k - pos - 2);
              if (name == "Word" && !options["unicode"].empty())
              {
                regex.append(line.substr(loc, pos - loc - 1)).append("(?:").append(scripts["L"]).append("|").append(scripts["Nd"]).append("|").append(scripts["Pc"]).append(")");
                loc = k + 1;
              }
              else
              {
                Map::const_iterator i = scripts.find(name);
                if (i != scripts.end())
                {
                  regex.append(line.substr(loc, pos - loc - 1)).append("(?:").append(i->second).append(")");
                  loc = k + 1;
                }
              }
              pos = k;
            }
            else if (pos + 2 < linelen && (line.at(pos) == 'u' || line.at(pos) == 'x') && line.at(pos + 1) == '{') // translate \u{X} and \x{X}
            {
              reflex::unicode_t wc = static_cast<reflex::unicode_t>(std::strtoul(line.c_str() + pos + 2, NULL, 16));
              if (wc > 0x7f)
              {
                char buf[7];
                buf[reflex::utf8(wc, buf)] = '\0';
                regex.append(line.substr(loc, pos - loc - 1)).append("(?:").append(buf).append(")");
                while (line.at(++pos) != '}')
                  continue;
                loc = pos + 1;
              }
            }
            else if (line.at(pos) >= '1' && line.at(pos) <= '7') // translate octals \123 to \0123
            {
              regex.append(line.substr(loc, pos - loc)).append("0");
              loc = pos;
            }
            else if (line.at(pos) == 's' && !options["unicode"].empty()) // translate \s to UTF-8 pattern
            {
              regex.append(line.substr(loc, pos - loc - 1)).append("(?:\\s|").append(scripts["Zs"]).append(")");
              loc = pos + 1;
            }
            else if (line.at(pos) == 'l' && !options["unicode"].empty()) // translate \l to UTF-8 pattern
            {
              regex.append(line.substr(loc, pos - loc - 1)).append("(?:").append(scripts["Ll"]).append(")");
              loc = pos + 1;
            }
            else if (line.at(pos) == 'u' && !options["unicode"].empty()) // translate \u to UTF-8 pattern
            {
              regex.append(line.substr(loc, pos - loc - 1)).append("(?:").append(scripts["Lu"]).append(")");
              loc = pos + 1;
            }
            else if (line.at(pos) == 'w' && !options["unicode"].empty()) // translate \w to UTF-8 pattern
            {
              regex.append(line.substr(loc, pos - loc - 1)).append("(?:\\w|").append(scripts["L"]).append("|").append(scripts["Nd"]).append("|").append(scripts["Pc"]).append(")");
              loc = pos + 1;
            }
            else if (line.at(pos) == 'X') // translate \X to match any UTF-8
            {
              regex.append(line.substr(loc, pos - loc)).append("(?:[\\x00-\\xbf]|[\\xc0-\\xff][\\x80-\\xbf]+)");
              loc = pos + 1;
            }
            else if (isalpha(line.at(pos)))
            {
              if (((options["matcher"].empty() || options["matcher"] == "reflex") && strchr("ABDHLSUWZabcdefijhlnprstuvwx", line.at(pos)) == NULL)
               || (!options["matcher"].empty() && options["matcher"] != "reflex" && strchr("ABDHLNPSUVWZabcdefghlnprstuvwxz", line.at(pos)) == NULL))
                warning("unsupported escape sequence: ", line.substr(pos - 1, 2).c_str());
            }
          }
          else
          {
            // line ends in \ and continues on the next line
            regex.append(line.substr(loc, pos - loc));
            if (!getline())
              error("EOF encountered inside a pattern");
            if (line == "%%")
              error("%% section ending encountered inside a pattern");
            pos = 0;
            (void)ws(pos); // skip indent
            loc = pos;
          }
          break;
        case '/':
          regex.append(line.substr(loc, pos - loc)).append("(?="); // translate / to (?=
          lap = lev + 1;
          loc = pos + 1;
          break;
        case '(':
          if (pos + 1 < linelen && line.at(pos + 1) != '?')
          {
            regex.append(line.substr(loc, pos - loc)).append("(?:"); // translate ( to (?:
            loc = pos + 1;
          }
          ++lev;
          break;
        case ')':
          if (lap == lev + 1)
          {
            regex.append(line.substr(loc, pos - loc)).append(")"); // translate ) to ))
            loc = pos;
            lap = 0;
          }
          --lev;
          break;
        case '|':
          if (lap == lev + 1)
          {
            regex.append(line.substr(loc, pos - loc)).append(")"); // translate | to )|
            loc = pos;
            lap = 0;
          }
          break;
        case '[':
          if (pos + 1 < linelen && line.at(pos + 1) == '^')
          {
            pos += 2;
            while (pos + 1 < linelen)
            {
              if (line.at(pos) == '\\' && pos + 1 < linelen)
              {
                ++pos;
              }
              else if (line.at(pos) == '[' && pos + 1 < linelen && line.at(pos + 1) == ':')
              {
                pos += 2;
                while (pos + 1 < linelen && (line.at(pos) != ':' || line.at(pos + 1) != ']'))
                  ++pos;
                ++pos;
              }
              ++pos;
              if (pos >= linelen || line.at(pos) == ']')
                break;
            }
          }
          else
          {
            // collect UTF-8 char sequences, \p{X}, \u{X}, \x{X} and \u{X}-\u{Y}, translate, and append as alternations
            regex.append(line.substr(loc, pos - loc));
            loc = pos++;
            std::string lifted;
            reflex::unicode_t wc = -1;
            bool range = false;
            size_t size = regex.size();
            size_t prev = pos;
            while (pos + 1 < linelen)
            {
              if (line.at(pos) == '\\' && pos + 1 < linelen)
              {
                size_t k = pos + 1;
                int c = line.at(k);
                if (k + 1 < linelen && line.at(k + 1) == '{')
                  while (++k < linelen && line.at(k) != '}')
                    continue;
                switch (c)
                {
                  case 'p':
                    if (k > pos + 1) // translate and lift \p{X} from [ ]
                    {
                      std::string name;
                      if (pos + 5 < linelen && line.at(pos + 3) == 'I' && line.at(pos + 4) == 's')
                        name = line.substr(pos + 5, k - pos - 5);
                      else
                        name = line.substr(pos + 3, k - pos - 3);
                      if (name == "Word" && !options["unicode"].empty())
                      {
                        if (lifted.empty())
                          regex.append("(?:");
                        regex.append(line.substr(loc, pos - loc));
                        lifted.append("|").append(scripts["L"]).append("|").append(scripts["Nd"]).append("|").append(scripts["Pc"]);
                        loc = k + 1;
                      }
                      else
                      {
                        Map::const_iterator i = scripts.find(name);
                        if (i != scripts.end())
                        {
                          if (lifted.empty())
                            regex.append("(?:");
                          regex.append(line.substr(loc, pos - loc));
                          lifted.append("|").append(i->second);
                          loc = k + 1;
                        }
                      }
                      pos = k;
                    }
                    wc = -1;
                    break;
                  case 'u':
                  case 'x':
                    if (k > pos + 1) // translate \u{X} and \x{X} and lift from [ ]
                    {
                      if (range)
                      {
                        if (lifted.empty())
                          regex.append("(?:");
                        regex.append(line.substr(loc, prev - loc));
                        lifted.append("|").append(reflex::utf8(wc, static_cast<reflex::unicode_t>(std::strtoul(line.c_str() + pos + 3, NULL, 16))));
                        pos = k;
                        loc = k + 1;
                        range = false;
                      }
                      else
                      {
                        wc = static_cast<reflex::unicode_t>(std::strtoul(line.c_str() + pos + 3, NULL, 16));
                        if (k + 1 < linelen && line.at(k + 1) != '-')
                        {
                          if (wc > 0x7f)
                          {
                            char buf[7];
                            buf[reflex::utf8(wc, buf)] = '\0';
                            if (lifted.empty())
                              regex.append("(?:");
                            regex.append(line.substr(loc, pos - loc));
                            lifted.append("|").append(buf);
                            pos = k;
                            loc = k + 1;
                          }
                        }
                        else
                        {
                          prev = pos;
                          pos = k;
                        }
                      }
                    }
                    else if (c == 'u' && !options["unicode"].empty()) // translate \u and lift from [ ]
                    {
                      if (lifted.empty())
                        regex.append("(?:");
                      regex.append(line.substr(loc, pos - loc));
                      lifted.append("|").append(scripts["Lu"]);
                      ++pos;
                      loc = pos + 1;
                      wc = -1;
                    }
                    break;
                  case 's':
                    if (!options["unicode"].empty()) // translate \s and lift from [ ]
                    {
                      if (lifted.empty())
                        regex.append("(?:");
                      regex.append(line.substr(loc, pos - loc));
                      lifted.append("|\\s|").append(scripts["Zs"]);
                      ++pos;
                      loc = pos + 1;
                    }
                    wc = -1;
                    break;
                  case 'l':
                    if (!options["unicode"].empty()) // translate \l and lift from [ ]
                    {
                      if (lifted.empty())
                        regex.append("(?:");
                      regex.append(line.substr(loc, pos - loc));
                      lifted.append("|").append(scripts["Ll"]);
                      ++pos;
                      loc = pos + 1;
                    }
                    wc = -1;
                    break;
                  case 'w':
                    if (!options["unicode"].empty()) // translate \w and lift from [ ]
                    {
                      if (lifted.empty())
                        regex.append("(?:");
                      regex.append(line.substr(loc, pos - loc));
                      lifted.append("|\\w|").append(scripts["L"]).append("|").append(scripts["Nd"]).append("|").append(scripts["Pc"]);
                      ++pos;
                      loc = pos + 1;
                    }
                    wc = -1;
                    break;
                  case 'X': // translate \X and lift from [ ]
                    if (lifted.empty())
                      regex.append("(?:");
                    regex.append(line.substr(loc, pos - loc));
                    lifted.append("|[\\x00-\\xbf]|[\\xc0-\\xff][\\x80-\\xbf]+");
                    ++pos;
                    loc = pos + 1;
                    wc = -1;
                    break;
                  default:
                    if (c >= '1' && c <= '7') // translate octals \123 to \0123
                    {
                      regex.append(line.substr(loc, pos - loc)).append("\\0");
                      loc = pos + 1;
                    }
                    ++pos;
                    wc = -1; // FIXME: UTF-8 or octal may be part of a bracket range
                    break;
                }
                range = false;
              }
              else if ((line.at(pos) & 0x80) == 0x80 && !options["unicode"].empty()) // lift UTF-8 char sequence from [ ]
              {
                if (lifted.empty())
                  regex.append("(?:");
                regex.append(line.substr(loc, pos - loc));
                lifted.append("|");
                lifted.push_back(line.at(pos));
                int c;
                while (++pos < linelen && ((c = line.at(pos)) & 0xc0) == 0x80)
                  lifted.push_back(c);
                loc = pos--;
              }
              else if (line.at(pos) == '[' && pos + 1 < linelen && line.at(pos + 1) == ':')
              {
                pos += 2;
                while (pos + 1 < linelen && (line.at(pos) != ':' || line.at(pos + 1) != ']'))
                  ++pos;
                ++pos;
              }
              else if (wc >= 0 && line.at(pos) == '-')
              {
                range = true;
              }
              else
              {
                wc = line.at(pos);
                range = false;
                prev = pos;
              }
              ++pos;
              if (pos >= linelen || line.at(pos) == ']')
                break;
            }
            if (lifted.empty())
              regex.append(line.substr(loc, pos - loc + 1));
            else if (pos > loc || regex.size() > size + 4)
              regex.append(line.substr(loc, pos - loc)).append("]").append(lifted).append(")");
            else
              regex = regex.substr(0, regex.size() - 1).append(lifted.substr(1)).append(")");
            loc = pos + 1;
          }
          if (pos >= linelen || line.at(pos) != ']')
            error("missing closing ]: ...", line.substr(loc).c_str());
          break;
        case '"':
          regex.append(line.substr(loc, pos - loc)).append("\\Q"); // translate to \Q...\E
          loc = ++pos;
          while (pos < linelen && line.at(pos) != '"')
          {
            if (line.at(pos) == '\\' && pos + 1 < linelen && line.at(pos + 1) == '"')
            {
              regex.append(line.substr(loc, pos - loc));
              loc = ++pos;
            }
            ++pos;
          }
          if (pos >= linelen || line.at(pos) != '"')
            error("missing closing \": \"...", line.substr(loc).c_str());
          regex.append(line.substr(loc, pos - loc)).append("\\E");
          loc = pos + 1;
          break;
        case '{':
          regex.append(line.substr(loc, pos - loc));
          loc = pos;
          if (pos + 1 < linelen && isalphaun(line.at(pos + 1)))
          {
            ++pos;
            std::string name = getname(pos);
            if (!name.empty() && pos < linelen && line.at(pos) == '}')
            {
              Map::const_iterator i = definitions.find(name);
              if (i == definitions.end())
                error("undefined name: ", name.c_str());
              regex.append("(?:").append(i->second).append(")");
              loc = pos + 1;
            }
          }
          else
          {
            while (pos + 1 < linelen && line.at(++pos) != '}')
              if (!isdigit(line.at(pos)) && line.at(pos) != ',')
                break;
            if (line.at(pos) != '}')
            {
              if (pos + 1 < linelen)
                error("error in { }: ...", line.substr(loc).c_str());
              else
                error("missing closing }: ...", line.substr(loc).c_str());
            }
            if (pos + 1 < linelen && line.at(pos + 1) == '?' && options["matcher"] == "boost")
              warning("unsupported lazy quantifier: ", line.substr(pos, 2).c_str());
          }
          break;
        case '#':
          if (!options["freespace"].empty()) // escape # in freespace mode, otherwise ends up as a comment
          {
            regex.append(line.substr(loc, pos - loc)).append("\\#");
            loc = pos + 1;
          }
          break;
        case '.':
          if (!options["unicode"].empty())
          {
            regex.append(line.substr(loc, pos - loc)).append("(?:.|[\\xc0-\\xff][\\x80-\\xbf]+)");
            loc = pos + 1;
          }
          break;
        case '*':
        case '+':
        case '?':
          if (pos + 1 < linelen && line.at(pos + 1) == '?' && options["matcher"] == "boost")
            warning("unsupported lazy quantifier: ", line.substr(pos, 2).c_str());
          break;
        default:
          if ((c & 0xc0) == 0xc0 && !options["unicode"].empty()) // group UTF-8 sequence
          {
            regex.append(line.substr(loc, pos - loc)).append("(?:").push_back(c);
            while (((c = line.at(++pos)) & 0xc0) == 0x80)
              regex.push_back(c);
            regex.append(")");
            loc = pos;
          }
      }
      blk = false;
    }
    ++pos;
  }
  if (lev > 0)
    error("missing closing )");
  regex.append(line.substr(loc, pos - loc));
  loc = regex.size();
  while (loc > 0 && isspace(regex.at(loc - 1)))
    --loc;
  regex.resize(loc);
  if (lap)
    regex.append(")");
  return regex;
}

Reflex::Starts Reflex::getstarts(size_t& pos)
{
  pos = 0;
  Starts starts;
  if (linelen > 1 && line.at(0) == '<' && isalphaun(line.at(1)))
  {
    do
    {
      ++pos;
      std::string name = getname(pos);
      Start start;
      for (start = 0; start < conditions.size() && name != conditions.at(start); ++start)
        continue;
      if (start == conditions.size())
        error("undeclared start condition ", name.c_str());
      starts.insert(start);
    } while (pos + 1 < linelen && line.at(pos) == ',');
    if (pos + 1 >= linelen || line.at(pos) != '>')
      error("bad start condition: ", line.c_str());
    ++pos;
  }
  else if (linelen > 1 && line.at(0) == '<' && line.at(1) == '*' && line.at(2) == '>')
  {
    for (Start start = 0; start < conditions.size(); ++start)
      starts.insert(start);
    pos = 3;
  }
  else
  {
    starts = inclusive;
  }
  return starts;
}

std::string Reflex::getcode(size_t& pos)
{
  std::string code;
  size_t blk = 0, lev = 0;
  enum { CODE, STRING, CHAR, COMMENT } tok = CODE;
  if (pos == 0 && (line == "%{" || istopcode() || isclasscode() || isinitcode()))
  {
    ++blk;
    pos = linelen;
  }
  else
  {
    (void)ws(pos);
    if (pos < linelen)
      code.append(line.substr(pos));
  }
  while (true)
  {
    while (pos >= linelen)
    {
      if (!getline())
        error("EOF encountered inside an action");
      pos = 0;
      if (tok == CODE)
      {
        if ((blk > 0 || lev > 0) && line == "%%")
          error("%% section ending encountered inside an action");
        if (line == "%{")
        {
          code.append(newline);
          ++blk;
        }
        else if (line == "%}")
        {
          code.append(newline);
          if (blk > 0)
            --blk;
          if (blk == 0 && lev == 0)
          {
            if (!getline())
              error("EOF encountered inside an action");
            return code;
          }
        }
        else
        {
          if (blk == 0 && lev == 0 && linelen > 0 && (!isspace(line.at(0)) || !options["freespace"].empty()))
            return code;
          code.append(newline).append(line);
        }
      }
      else
      {
        code.append(newline).append(line);
      }
    }
    switch (line.at(pos))
    {
      case '{':
        if (tok == CODE)
          ++lev;
        break;
      case '}':
        if (tok == CODE && lev > 0)
          --lev;
        break;
      case '"':
        if (tok == CODE)
          tok = STRING;
        else if (tok == STRING)
          tok = CODE;
        break;
      case '\'':
        if (tok == CODE)
          tok = CHAR;
        else if (tok == CHAR)
          tok = CODE;
        break;
      case '/':
        if (tok == CODE && pos + 1 < linelen)
        {
          if (line.at(pos + 1) == '/')
            pos = linelen;
          else if (line.at(pos + 1) == '*')
            tok = COMMENT;
        }
        break;
      case '*':
        if (tok == COMMENT && pos + 1 < linelen && line.at(pos + 1) == '/')
          tok = CODE;
        break;
      case '\\':
        ++pos;
        break;
    }
    ++pos;
  }
  return code;
}

void Reflex::error(const char *message, const char *arg, size_t at_lineno)
{
  std::cerr
    << infile
    << ":"
    << (at_lineno ? at_lineno : lineno)
    << ": "
    << message
    << (arg != NULL ? arg : "")
    << std::endl;
  exit(EXIT_FAILURE);
}

void Reflex::warning(const char *message, const char *arg, size_t at_lineno)
{
  if (options["nowarn"].empty())
    std::cerr
      << infile
      << ":"
      << (at_lineno ? at_lineno : lineno)
      << ": warning, "
      << message
      << (arg != NULL ? arg : "")
      << std::endl;
}

void Reflex::parse_section_1(void)
{
  if (!getline())
    return;
  while (line != "%%")
  {
    if (linelen == 0)
    {
      if (!getline())
        return;
    }
    else
    {
      if (iscode())
      {
        size_t pos = 0;
        size_t this_lineno = lineno;
        std::string code = getcode(pos);
        section_1.push_back(Code(code, infile, this_lineno));
      }
      else if (istopcode())
      {
        size_t pos = 0;
        size_t this_lineno = lineno;
        std::string code = getcode(pos);
        section_top.push_back(Code(code, infile, this_lineno));
      }
      else if (isclasscode())
      {
        size_t pos = 0;
        size_t this_lineno = lineno;
        std::string code = getcode(pos);
        section_class.push_back(Code(code, infile, this_lineno));
      }
      else if (isinitcode())
      {
        size_t pos = 0;
        size_t this_lineno = lineno;
        std::string code = getcode(pos);
        section_init.push_back(Code(code, infile, this_lineno));
      }
      else
      {
        if (linelen > 1 && line.at(0) == '%')
        {
          size_t pos = 1;
          if (as(pos, "include"))
          {
            do
            {
              std::string filename = getstring(pos);
              if (filename.empty())
                error("bad file name");
              include(filename);
            } while (!nl(pos));
          }
          else
          {
            pos = 1;
            if (as(pos, "state"))
            {
              do
              {
                std::string name = getname(pos);
                if (name.empty())
                  error("bad start condition name");
                inclusive.insert(conditions.size());
                conditions.push_back(name);
              } while (!nl(pos));
            }
            else
            {
              pos = 1;
              if (as(pos, "xstate"))
              {
                do
                {
                  std::string name = getname(pos);
                  if (name.empty())
                    error("bad start condition name");
                  conditions.push_back(name);
                } while (!nl(pos));
              }
              else
              {
                bool option = false;
                pos = 1;
                if (as(pos, "option"))
                  option = true;
                else
                  pos = 1;
                do
                {
                  std::string name = getname(pos);
                  if (name.empty())
                    error("bad %option name");
                  Map::iterator i = options.find(name);
                  if (i == options.end())
                    error("unrecognized %option: ", name.c_str());
                  (void)ws(pos);
                  if (eq(pos) && pos < linelen) // %option OPTION = NAME
                  {
                    if (line.at(pos) == '"') // %option OPTION = "NAME"
                      i->second = getstring(pos);
                    else
                      i->second = getname(pos);
                    (void)ws(pos);
                  }
                  else
                  {
                    i->second = "true";
                  }
                  if (!option && !nl(pos))
                    error("trailing text after %option: ", name.c_str());
                } while (!nl(pos));
              }
            }
          }
        }
        else
        {
          size_t pos = 0;
          std::string name;
          std::string regex;
          if ((name = getname(pos)).empty() || !ws(pos) || (regex = getregex(pos)).empty() || !nl(pos))
            error("bad line in section 1: ", line.c_str());
          if (definitions.find(name) != definitions.end())
            error("name defined twice: ", name.c_str());
          definitions[name] = regex;
        }
        if (!getline())
          return;
      }
    }
  }
}

void Reflex::parse_section_2(void)
{
  if (in->eof())
    error("missing %% section 2");
  bool init = true;
  std::stack<Starts> scopes;
  if (!getline())
    return;
  while (line != "%%")
  {
    if (linelen == 0)
    {
      if (!getline())
        break;
    }
    else
    {
      size_t pos = 0;
      if (init && iscode())
      {
        std::string code = getcode(pos);
        if (scopes.empty())
        {
          section_2[0].push_back(Code(code, infile, lineno));
        }
        else
        {
          for (Starts::const_iterator i = scopes.top().begin(); i != scopes.top().end(); ++i)
            section_2[*i].push_back(Code(code, infile, lineno));
        }
      }
      else if (line == "}" && !scopes.empty())
      {
        scopes.pop();
        if (!getline())
          break;
      }
      else
      {
        if (!skipcomment(pos) || line == "%%")
          break;
        Starts starts = getstarts(pos);
        if (pos + 1 == linelen && line.at(pos) == '{')
        {
          scopes.push(starts);
          if (!getline())
            error("EOF encountered inside an action");
          init = true;
        }
        else
        {
          bool no_starts = pos == 0;
          std::string regex = getregex(pos);
          if (regex.empty())
            error("bad line in section 2: ", line.c_str());
          size_t rule_lineno = lineno;
          std::string code = getcode(pos);
          if (no_starts && scopes.empty() && regex == "<<EOF>>")
          {
            for (Start start = 0; start < conditions.size(); ++start)
              rules[start].push_back(Rule(regex, Code(code, infile, rule_lineno))); // only the first <<EOF>> code will be used
          }
          else if (no_starts && !scopes.empty())
          {
            for (Starts::const_iterator start = scopes.top().begin(); start != scopes.top().end(); ++start)
              rules[*start].push_back(Rule(regex, Code(code, infile, rule_lineno)));
          }
          else
          {
            for (Starts::const_iterator start = starts.begin(); start != starts.end(); ++start)
              rules[*start].push_back(Rule(regex, Code(code, infile, rule_lineno)));
          }
          init = false;
        }
      }
    }
  }
  if (!scopes.empty())
    error("missing closing } of start condition scope in section 2");
  patterns.resize(conditions.size());
  for (Start start = 0; start < conditions.size(); ++start)
  {
    const char *sep = "";
    for (Rules::const_iterator rule = rules[start].begin(); rule != rules[start].end(); ++rule)
    {
      if (rule->regex != "<<EOF>>")
      {
        patterns[start].append(sep).append("(").append(rule->regex).append(")");
        sep = "|";
      }
    }
  }
}

void Reflex::parse_section_3(void)
{
  if (in->eof())
    error("missing %% section 3");
  while (getline())
    section_3.push_back(Code(line, infile, lineno));
}

void Reflex::write(void)
{
  if (!options["yyclass"].empty())
    options["flex"] = "true";
  if (options["prefix"].empty() && !options["flex"].empty())
    options["prefix"] = "yy";
  if (options["lexer"].empty())
    options["lexer"] = options["prefix"] + (!options["flex"].empty() ? "FlexLexer" : "Lexer");
  if (options["lex"].empty())
    options["lex"] = options["prefix"] + "lex";
  if (options["header_file"] == "true")
  {
    if (options["prefix"].empty())
      options["header_file"] = "lex.yy.h";
    else
      options["header_file"] = std::string("lex.").append(options["prefix"]).append(".h");
  }
  if (options["matcher"] == "reflex")
    options["matcher"].clear();
  else if (!options["matcher"].empty() && options["matcher"] != "boost" && options["matcher"] != "boost-perl")
    warning("using custom matcher ", options["matcher"].c_str());
  std::ofstream ofs;
  if (options["stdout"].empty())
  {
    if (options["outfile"].empty())
    {
      if (options["prefix"].empty())
        options["outfile"] = "lex.yy.cpp";
      else
        options["outfile"] = std::string("lex.").append(options["prefix"]).append(".cpp");
    }
    ofs.open(options["outfile"].c_str(), std::ofstream::out);
    if (!ofs.is_open())
      abort("opening file ", options["outfile"].c_str());
    out = &ofs;
  }
  *out << "// " << options["outfile"] << " generated by reflex " REFLEX_VERSION " from " << infile << std::endl;
  write_prelude();
  write_section_top();
  write_class();
  write_section_1();
  write_lexer();
  write_main();
  write_section_3();
  if (!out->good())
    abort("in writing");
  if (options["matcher"].empty() && (!options["full"].empty() || !options["fast"].empty()) && options["tables_file"].empty() && options["stdout"].empty())
    write_banner("TABLES");
  if (ofs.is_open())
    ofs.close();
  write_stats();
  if (!options["regexp_file"].empty())
  {
    std::ofstream ofs;
    bool append = false;
    for (Start start = 0; start < conditions.size(); ++start)
    {
      if (!append)
      {
        std::string filename;
        if (options["regexp_file"] == "true")
        {
          filename = "reflex.";
          filename.append(conditions[start]).append(".txt");
        }
        else
        {
          filename = file_ext(options["regexp_file"], "txt");
          append = true;
        }
        if (filename.compare(0, 7, "stdout.") == 0)
        {
          out = &std::cout;
        }
        else
        {
          ofs.open(filename.c_str(), std::ofstream::out);
          if (!ofs.is_open())
            abort("opening file ", filename.c_str());
          out = &ofs;
        }
      }
      *out << "\"(?m";
      if (!options["case_insensitive"].empty())
        *out << "i";
      if (!options["dotall"].empty())
        *out << "s";
      if (!options["freespace"].empty())
        *out << "x";
      *out << ")";
      write_regex(patterns[start]);
      *out << "\"" << std::endl;
      if (!ofs.good())
        abort("in writing");
      if (!append && ofs.is_open())
        ofs.close();
    }
    if (append && ofs.is_open())
      ofs.close();
  }
  if (!options["header_file"].empty())
  {
    ofs.open(options["header_file"].c_str(), std::ofstream::out);
    if (!ofs.is_open())
      abort("opening file ", options["header_file"].c_str());
    out = &ofs;
    *out <<
      "// " << options["header_file"] << " generated by reflex " REFLEX_VERSION " from " << infile << std::endl << std::endl <<
      "#ifndef " << options["prefix"] << "REFLEX_HEADER_H" << std::endl <<
      "#define " << options["prefix"] << "REFLEX_HEADER_H" << std::endl <<
      "#define " << options["prefix"] << "IN_HEADER 1" << std::endl;
    write_prelude();
    write_section_top();
    write_class();
    if (!options["reentrant"].empty() || !options["bison_bridge"].empty())
    {
      if (!options["bison_locations"].empty())
        write_banner("BISON BRIDGE LOCATIONS");
      else if (!options["bison_bridge"].empty())
        write_banner("BISON BRIDGE");
      else
        write_banner("REENTRANT");
      *out << "typedef ";
      if (!options["namespace"].empty())
        *out << options["namespace"] << "::";
      if (!options["yyclass"].empty())
        *out << options["yyclass"];
      else if (!options["class"].empty())
        *out << options["class"];
      else
        *out << options["lexer"];
      *out <<
        " yyscanner_t;\n"
        "typedef void *yyscan_t;\n\n"
        "#ifndef YY_EXTERN_C\n"
        "#define YY_EXTERN_C\n"
        "#endif\n\n";
      if (!options["bison_locations"].empty())
        *out << "YY_EXTERN_C int yylex(YYSTYPE*, YYLTYPE*, yyscan_t);\n";
      else if (!options["bison_bridge"].empty())
        *out << "YY_EXTERN_C int yylex(YYSTYPE*, yyscan_t);\n";
      else
        *out << "YY_EXTERN_C int yylex(yyscan_t);\n";
      *out <<
        "YY_EXTERN_C void yylex_init(yyscan_t*);\n"
        "YY_EXTERN_C void yylex_destroy(yyscan_t);\n";
    }
    else if (!options["bison_locations"].empty())
    {
      write_banner("BISON LOCATIONS");
      *out <<
        "extern ";
      if (!options["namespace"].empty())
        *out << options["namespace"] << "::";
      if (!options["yyclass"].empty())
        *out << options["yyclass"];
      else if (!options["class"].empty())
        *out << options["class"];
      else
        *out << options["lexer"];
      *out <<
        " YY_SCANNER;\n\n"
        "#ifndef YY_EXTERN_C\n"
        "#define YY_EXTERN_C\n"
        "#endif\n\n"
        "YY_EXTERN_C int yylex(YYSTYPE*, YYLTYPE*);\n";
    }
    else if (!options["bison"].empty())
    {
      write_banner("BISON");
      if (!options["flex"].empty())
        *out <<
          "extern char *" << options["prefix"] << "text;\n"
          "extern yy_size_t " << options["prefix"] << "leng;\n"
          "extern int " << options["prefix"] << "lineno;\n";
      *out <<
        "extern ";
      if (!options["namespace"].empty())
        *out << options["namespace"] << "::";
      if (!options["yyclass"].empty())
        *out << options["yyclass"];
      else if (!options["class"].empty())
        *out << options["class"];
      else
        *out << options["lexer"];
      *out <<
        " YY_SCANNER;\n\n"
        "#ifndef YY_EXTERN_C\n"
        "#define YY_EXTERN_C\n"
        "#endif\n\n";
      if (!options["flex"].empty())
        *out << "YY_EXTERN_C int " << options["prefix"] << "lex(void);\n";
      else
        *out << "YY_EXTERN_C int yylex(void);\n";
    }
    *out << std::endl << "#endif" << std::endl;
    if (!out->good())
      abort("in writing");
    ofs.close();
  }
}

void Reflex::write_banner(const char *title)
{
  size_t i;
  *out << std::endl;
  for (i = 0; i < 80; i++)
    out->put('/');
  *out << std::endl << "//";
  for (i = 0; i < 76; i++)
    out->put(' ');
  *out << "//" << std::endl << "//  ";
  out->width(74);
  *out << std::left << title;
  *out << "//" << std::endl << "//";
  for (i = 0; i < 76; i++)
    out->put(' ');
  *out << "//" << std::endl;
  for (i = 0; i < 80; i++)
    out->put('/');
  *out << std::endl << std::endl;
}

void Reflex::write_prelude(void)
{
  if (!out->good())
    return;
  write_banner("OPTIONS USED");
  for (Map::const_iterator option = options.begin(); option != options.end(); ++option)
  {
    if (!option->second.empty())
    {
      *out << "#define REFLEX_OPTION_";
      out->width(20);
      *out << std::left << option->first;
      *out << option->second << std::endl;
    }
  }
}

void Reflex::write_class(void)
{
  if (!out->good())
    return;
  std::string matcher;
  write_banner("REGEX MATCHER");
  if (options["matcher"] == "boost")
  {
    *out << "#include \"boostmatcher.h\"" << std::endl;
    matcher = "reflex::BoostPosixMatcher";
  }
  else if (options["matcher"] == "boost-perl")
  {
    *out << "#include \"boostmatcher.h\"" << std::endl;
    matcher = "reflex::BoostPerlMatcher";
  }
  else if (options["matcher"].empty())
  {
    *out << "#include \"matcher.h\"" << std::endl;
    matcher = "reflex::Matcher";
  }
  else
  {
    *out << "#include \"" << options["matcher"] << ".h\"" << std::endl;
    matcher = options["matcher"];
  }
  std::string lex = options["lex"];
  std::string base;
  if (!options["flex"].empty())
  {
    write_banner("FLEX-COMPATIBLE ABSTRACT LEXER CLASS AND FLEX DEFINITIONS");
    *out << "#include \"flexlexer.h\"" << std::endl;
    size_t num_rules = 0;
    for (Start start = 0; start < conditions.size(); ++start)
      num_rules += rules[start].size();
    for (Start start = 0; start < conditions.size(); ++start)
      *out << "#define " << conditions[start] << " (" << start << ")" << std::endl;
    *out << "#define YY_NUM_RULES (" << num_rules << ")" << std::endl;
    if (!options["bison"].empty() && options["reentrant"].empty() && options["bison_bridge"].empty() && options["bison_locations"].empty())
      *out <<
        "#undef yytext\n"
        "#undef yyleng\n"
        "#undef yylineno\n";
    if (!options["namespace"].empty())
      *out << std::endl << "namespace " << options["namespace"] << " {" << std::endl;
    *out << "typedef reflex::FlexLexer" << "<" << matcher << "> FlexLexer;" << std::endl;
    if (!options["namespace"].empty())
      *out << "}" << std::endl;
    base = "FlexLexer";
  }
  else
  {
    write_banner("ABSTRACT LEXER CLASS");
    *out << "#include \"abslexer.h\"" << std::endl;
    base.append("reflex::AbstractLexer<").append(matcher).append(">");
  }
  write_banner("LEXER CLASS");
  std::string lexer = options["lexer"];
  if (!options["flex"].empty())
  {
    if (!options["namespace"].empty())
      *out << "namespace " << options["namespace"] << " {" << std::endl << std::endl;
    *out <<
      "class " << lexer << " : public " << base << " {\n";
    write_section_class();
    *out <<
      " public:\n"
      "  " << lexer << "(\n"
      "      const reflex::Input& input = stdin,\n"
      "      std::ostream        *os    = NULL)\n"
      "    :\n"
      "      " << base << "(input, os)\n";
    write_section_init();
    if (!options["bison_bridge"].empty() || !options["bison_locations"].empty())
    {
      *out <<
        "  virtual int yylex(void)\n"
        "  {\n"
        "    LexerError(\"" << lexer << "::yylex invoked but %option bison-bridge or bison-location used\");\n"
        "    return 0;\n"
        "  }\n"
        "  virtual int " << lex << "(YYSTYPE& yylval)";
    }
    else
    {
      if (options["lex"] != "yylex")
        *out <<
          "  virtual int yylex(void)\n"
          "  {\n"
          "    LexerError(\"" << lexer << "::yylex invoked but %option lex=" << lex << " used\");\n"
          "    return 0;\n"
          "  }\n";
      *out <<
        "  virtual int " << lex << "(void)";
    }
    if (options["yyclass"].empty())
      *out << ";\n";
    else
      *out <<
        "\n"
        "  {\n"
        "    LexerError(\"" << lexer << "::" << lex << " invoked but %option yyclass=" << options["yyclass"] << " used\");\n"
        "    return 0;\n"
        "  }"
        << std::endl;
    if (options["bison_bridge"].empty())
      *out <<
        "  int " << lex << "(\n"
        "      const reflex::Input& input,\n"
        "      std::ostream        *os = NULL)\n"
        "  {\n"
        "    in(input);\n"
        "    out(os ? *os : std::cout);\n"
        "    return " << lex << "();\n"
        "  }"
        << std::endl;
    *out <<
      "};" << std::endl;
    if (!options["namespace"].empty())
      *out << std::endl << "}" << std::endl;
  }
  else
  {
    if (!options["namespace"].empty())
      *out << "namespace " << options["namespace"] << " {" << std::endl << std::endl;
    *out <<
      "class " << lexer << " : public " << base << " {\n";
    write_section_class();
    *out <<
      " public:\n"
      "  " << lexer << "(\n"
      "      const reflex::Input& input = " << (options["nostdinit"].empty() ? "stdin" : "std::cin") << ",\n" <<
      "      std::ostream&        os    = std::cout)\n"
      "    :\n"
      "      " << base << "(input, os)\n";
    write_section_init();
    for (Start start = 0; start < conditions.size(); ++start)
      *out <<
        "  static const int " << conditions[start] << " = " << start << ";\n";
    if (!options["bison_bridge"].empty() || !options["bison_locations"].empty())
      *out <<
        "  virtual int " << lex << "(YYSTYPE& yylval)";
    else
      *out <<
        "  virtual int " << lex << "(void)";
    if (options["class"].empty())
      *out << ";\n";
    else
      *out << " = 0;\n";
    if (options["bison_bridge"].empty())
      *out <<
        "  int " << lex << "(\n"
        "      const reflex::Input& input,\n"
        "      std::ostream&        os = std::cout)\n"
        "  {\n"
        "    in(input);\n"
        "    out(os);\n"
        "    return " << lex << "();\n"
        "  }\n";
    *out <<
      "};" << std::endl;
    if (!options["namespace"].empty())
      *out << std::endl << "}" << std::endl;
  }
}

void Reflex::write_section_top(void)
{
  if (!section_top.empty())
  {
    write_banner("SECTION 1: %top{ user code %}");
    write_code(section_top);
  }
}

void Reflex::write_section_class(void)
{
  if (!section_class.empty())
    write_code(section_class);
}

void Reflex::write_section_init(void)
{
  *out << "  {\n";
  if (!section_init.empty())
    write_code(section_init);
  if (!options["debug"].empty())
    *out << "    set_debug(" << options["debug"] << ");\n";
  *out << "  }" << std::endl;
}

void Reflex::write_section_1(void)
{
  if (!section_1.empty())
  {
    write_banner("SECTION 1: %{ user code %}");
    write_code(section_1);
  }
}

void Reflex::write_section_3(void)
{
  if (!section_3.empty())
  {
    write_banner("SECTION 3: user code");
    write_code(section_3);
  }
}

void Reflex::write_code(const Codes& codes)
{
  if (!out->good())
    return;
  size_t this_lineno = 0;
  for (Codes::const_iterator code = codes.begin(); code != codes.end(); ++code)
  {
    if (code->lineno != this_lineno && options["noline"].empty())
    {
      *out << "#line " << code->lineno;
      if (!code->file.empty())
        *out << " \"" << code->file << "\"";
      *out << std::endl;
    }
    *out << code->line << std::endl;
    this_lineno = code->lineno + 1;
  }
}

void Reflex::write_code(const Code& code)
{
  if (options["noline"].empty())
  {
    *out << "#line " << code.lineno;
    if (!infile.empty())
      *out << " \"" << infile << "\"";
    *out << std::endl;
  }
  if (!code.line.empty())
    *out << code.line << std::endl;
}

void Reflex::write_lexer(void)
{
  if (!out->good())
    return;
  if (!options["reentrant"].empty() || !options["bison_bridge"].empty())
  {
    if (!options["bison_locations"].empty())
      write_banner("BISON BRIDGE LOCATIONS");
    if (!options["bison_bridge"].empty())
      write_banner("BISON BRIDGE");
    else
      write_banner("REENTRANT");
    *out << "typedef ";
    if (!options["namespace"].empty())
      *out << options["namespace"] << "::";
    if (!options["yyclass"].empty())
      *out << options["yyclass"];
    else if (!options["class"].empty())
      *out << options["class"];
    else
      *out << options["lexer"];
    *out <<
      " yyscanner_t;\n\n"
      "typedef void *yyscan_t;\n\n"
      "#ifndef YY_EXTERN_C\n"
      "#define YY_EXTERN_C\n"
      "#endif\n\n";
    if (!options["bison_locations"].empty())
      *out <<
        "YY_EXTERN_C int yylex(YYSTYPE *lvalp, YYLTYPE *llocp, yyscan_t scanner)\n"
        "{\n"
        "  llocp->first_line = llocp->last_line;\n"
        "  llocp->first_column = llocp->last_column;\n"
        "  int res = static_cast<yyscanner_t*>(scanner)->" << options["lex"] << "(*lvalp);\n"
        "  llocp->last_line = static_cast<yyscanner_t*>(scanner)->matcher().lineno();\n"
        "  llocp->last_column = static_cast<yyscanner_t*>(scanner)->matcher().columno();\n"
        "  return res;\n"
        "}\n\n";
    else if (!options["bison_bridge"].empty())
      *out <<
        "YY_EXTERN_C int yylex(YYSTYPE *lvalp, yyscan_t scanner)\n"
        "{\n"
        "  return static_cast<yyscanner_t*>(scanner)->" << options["lex"] << "(*lvalp);\n"
        "}\n\n";
    else
      *out <<
        "YY_EXTERN_C int yylex(yyscan_t scanner)\n"
        "{\n"
        "  return static_cast<yyscanner_t*>(scanner)->" << options["lex"] << "();\n"
        "}\n\n";
    *out <<
      "YY_EXTERN_C void yylex_init(yyscan_t *scanner)\n"
      "{\n"
      "  *scanner = static_cast<yyscan_t>(new yyscanner_t);\n"
      "}\n\n"
      "YY_EXTERN_C void yylex_destroy(yyscan_t scanner)\n"
      "{\n"
      "  delete static_cast<yyscanner_t*>(scanner);\n"
      "}\n";
  }
  else if (!options["bison_locations"].empty())
  {
    write_banner("BISON LOCATIONS");
    if (!options["namespace"].empty())
      *out << options["namespace"] << "::";
    if (!options["yyclass"].empty())
      *out << options["yyclass"];
    else if (!options["class"].empty())
      *out << options["class"];
    else
      *out << options["lexer"];
    *out <<
      " YY_SCANNER;\n\n"
      "#ifndef YY_EXTERN_C\n"
      "#define YY_EXTERN_C\n"
      "#endif\n\n"
      "YY_EXTERN_C int yylex(YYSTYPE *lvalp, YYLTYPE *llocp)\n"
      "{\n"
      "  llocp->first_line = llocp->last_line;\n"
      "  llocp->first_column = llocp->last_column;\n"
      "  int res = YY_SCANNER." << options["lex"] << "(*lvalp);\n"
      "  llocp->last_line = YY_SCANNER.matcher().lineno();\n"
      "  llocp->last_column = YY_SCANNER.matcher().columno();\n"
      "  return res;\n"
      "}\n\n";
  }
  else if (!options["bison"].empty())
  {
    write_banner("BISON");
    if (!options["namespace"].empty())
      *out << options["namespace"] << "::";
    if (!options["yyclass"].empty())
      *out << options["yyclass"];
    else if (!options["class"].empty())
      *out << options["class"];
    else
      *out << options["lexer"];
    *out <<
      " YY_SCANNER;\n\n"
      "#ifndef YY_EXTERN_C\n"
      "#define YY_EXTERN_C\n"
      "#endif\n\n";
    if (!options["flex"].empty())
      *out <<
        "char *" << options["prefix"] << "text;\n"
        "yy_size_t " << options["prefix"] << "leng;\n"
        "int " << options["prefix"] << "lineno;\n"
        "YY_EXTERN_C int " << options["prefix"] << "lex(void)\n"
        "{\n"
        "  int res = YY_SCANNER." << options["lex"] << "();\n"
        "  " << options["prefix"] << "text = const_cast<char*>(YY_SCANNER.YYText());\n"
        "  " << options["prefix"] << "leng = static_cast<yy_size_t>(YY_SCANNER.YYLeng());\n"
        "  " << options["prefix"] << "lineno = static_cast<int>(YY_SCANNER.lineno());\n"
        "  return res;\n"
        "}\n\n"
        "#define " << options["prefix"] << "text const_cast<char*>(YY_SCANNER.YYText())\n"
        "#define " << options["prefix"] << "leng static_cast<yy_size_t>(YY_SCANNER.YYLeng())\n"
        "#define " << options["prefix"] << "lineno static_cast<int>(YY_SCANNER.lineno())\n";
    else
      *out <<
        "YY_EXTERN_C int yylex(void)\n"
        "{\n"
        "  return YY_SCANNER." << options["lex"] << "();\n"
        "}\n";
  }
  write_banner("SECTION 2: rules");
  if (options["matcher"].empty() && !options["fast"].empty())
  {
    for (Start start = 0; start < conditions.size(); ++start)
      *out << "extern void reflex_code_" << conditions[start] << "(reflex::Matcher&);\n";
    *out << std::endl;
  }
  else if (options["matcher"].empty() && !options["full"].empty())
  {
    for (Start start = 0; start < conditions.size(); ++start)
      *out << "extern const reflex::Pattern::Opcode reflex_code_" << conditions[start] << "[];\n";
    *out << std::endl;
  }
  *out << "int ";
  if (!options["namespace"].empty())
    *out << options["namespace"] << "::";
  if (!options["yyclass"].empty())
    *out << options["yyclass"];
  else if (!options["class"].empty())
    *out << options["class"];
  else
    *out << options["lexer"];
  if (!options["bison_bridge"].empty() || !options["bison_locations"].empty())
    *out << "::" << options["lex"] << "(YYSTYPE& yylval)\n{\n";
  else
    *out << "::" << options["lex"] << "(void)\n{\n";
  for (Start start = 0; start < conditions.size(); ++start)
  {
    if (options["matcher"].empty())
    {
      if (!options["full"].empty() || !options["fast"].empty())
      {
        *out << "  static const reflex::Pattern PATTERN_" << conditions[start] << "(reflex_code_" << conditions[start] << ");\n";
      }
      else
      {
        *out << "  static const reflex::Pattern PATTERN_" << conditions[start] << "(\"";
        write_regex(patterns[start]);
        *out << "\", \"m";
        if (!options["case_insensitive"].empty())
          *out << ";i";
        if (!options["dotall"].empty())
          *out << ";s";
        if (!options["freespace"].empty())
          *out << ";x";
        *out << "\");\n";
      }
    }
    else if (options["matcher"] == "boost" || options["matcher"] == "boost-perl")
    {
      *out << "  static const boost::regex PATTERN_" << conditions[start] << "(\"";
      if (!options["dotall"].empty())
        *out << "(?s)";
      write_regex(patterns[start]);
      *out << "\", static_cast<boost::regex_constants::syntax_option_type>(";
      *out << "boost::regex_constants::normal";
      if (!options["case_insensitive"].empty())
        *out << " | boost::regex_constants::icase";
      if (!options["fast"].empty())
        *out << " | boost::regex_constants::optimize";
      *out << "));\n";
    }
    else if (!options["pattern"].empty())
    {
      *out << "  static const " << options["pattern"] << " PATTERN_" << conditions[start] << "(\"(?m";
      if (!options["case_insensitive"].empty())
        *out << "i";
      if (!options["dotall"].empty())
        *out << "s";
      *out << ")";
      write_regex(patterns[start]);
      *out << "\");\n";
    }
    else
    {
      *out << "  static const char *PATTERN_" << conditions[start] << " = \"(?m";
      if (!options["case_insensitive"].empty())
        *out << "i";
      if (!options["dotall"].empty())
        *out << "s";
      if (!options["freespace"].empty())
        *out << "x";
      *out << ")";
      write_regex(patterns[start]);
      *out << "\";";
    }
  }
  *out <<
    "  if (!has_matcher())\n"
    "  {\n";
  if (!options["tabs"].empty())
    *out <<
      "    matcher(new Matcher(PATTERN_" << conditions[0] << ", in(), this, \"T=" << options["tabs"] << "\"));\n";
  else
    *out <<
      "    matcher(new Matcher(PATTERN_" << conditions[0] << ", in(), this));\n";
  if (!options["interactive"].empty() || !options["always_interactive"].empty())
    *out <<
      "    matcher().interactive();\n";
  else if (!options["batch"].empty())
    *out <<
      "    matcher().buffer();\n";
  *out <<
    "    start(" << conditions[0] << ");\n";
  if (!options["flex"].empty())
    *out <<
      "    YY_USER_INIT\n";
  *out <<
    "  }\n";
  if (conditions.size() == 1)
  {
    write_code(section_2[0]);
  }
  else if (!section_2.empty())
  {
    *out << "  switch (start())" << std::endl << "  {" << std::endl;
    for (CodesMap::const_iterator i = section_2.begin(); i != section_2.end(); ++i)
    {
      *out << "    case " << conditions[i->first] << ":" << std::endl;
      write_code(i->second);
      *out << "    break;" << std::endl;
    }
    *out << "  }" << std::endl;
  }
  *out <<
    "  while (true)\n"
    "  {\n";
  if (conditions.size() > 1)
    *out <<
      "    switch (start())\n"
      "    {\n";
  for (Start start = 0; start < conditions.size(); ++start)
  {
    if (conditions.size() > 1)
      *out <<
        "      case " << conditions[start] << ":\n"
        "        matcher().pattern(PATTERN_" << conditions[start] << ");\n";
    *out <<
      "        switch (matcher().scan())\n"
      "        {\n"
      "          case 0:\n"
      "            if (matcher().at_end())\n"
      "            {\n";
    bool has_eof = false;
    for (Rules::const_iterator rule = rules[start].begin(); rule != rules[start].end(); ++rule)
    {
      if (rule->regex == "<<EOF>>")
      {
        if (!options["debug"].empty())
          *out <<
            "              if (debug()) std::cerr << \"--EOF rule at line " << rule->code.lineno << " (start condition \" << start() << \")\\n\";\n";
        write_code(rule->code);
        has_eof = true;
        break;
      }
    }
    if (!has_eof && !options["debug"].empty())
      *out <<
        "              if (debug()) std::cerr << \"--EOF (start condition \" << start() << \")\\n\";\n";
    if (!has_eof)
      *out <<
        "              return 0;\n";
    *out <<
      "            }\n"
      "            else\n"
      "            {\n";
    if (!options["debug"].empty())
      *out <<
        "              if (debug()) std::cerr << \"--accepting default rule\\n\";\n";
    if (!options["nodefault"].empty())
    {
      if (!options["flex"].empty())
        *out <<
          "              LexerError(\"scanner jammed\");\n"
          "              return 0;\n";
      else if (!options["debug"].empty())
        *out <<
          "              if (debug()) std::cerr << \"--suppressing (\\\"\" << (char)matcher().input() << \"\\\")\\n\";\n";
      else
        *out <<
          "              matcher().input();\n";
    }
    else if (!options["flex"].empty())
    {
      *out <<
        "              output(matcher().input());\n";
    }
    else
    {
      *out <<
        "              out().put(matcher().input());\n";
    }
    *out <<
      "            }\n";
    if (!options["flex"].empty())
      *out <<
        "            YY_BREAK\n";
    else
      *out <<
        "            break;\n";
    size_t accept = 1;
    for (Rules::const_iterator rule = rules[start].begin(); rule != rules[start].end(); ++rule)
    {
      if (rule->regex != "<<EOF>>")
      {
        *out <<
          "          case " << accept << ": // rule at line " << rule->code.lineno << ": " << rule->regex << std::endl;
        if (rule->code.line != "|")
        {
          if (!options["debug"].empty())
            *out <<
              "            if (debug()) std::cerr << \"--accepting rule at line " << rule->code.lineno << " (\\\"\" << matcher().text() << \"\\\")\\n\";\n";
          if (!options["flex"].empty())
            *out <<
              "            YY_USER_ACTION\n";
          write_code(rule->code);
          if (!options["flex"].empty())
            *out <<
              "            YY_BREAK\n";
          else
            *out <<
              "            break;\n";
        }
        ++accept;
      }
    }
    *out <<
      "        }\n";
    if (conditions.size() > 1)
      *out <<
        "        break;\n";
  }
  if (conditions.size() > 1)
    *out <<
      "    }\n";
  *out <<
    "  }\n"
    "}" << std::endl;
}

void Reflex::write_main(void)
{
  if (!out->good())
    return;
  if (!options["main"].empty())
  {
    write_banner("SECTION 3: main");
    *out << "int main()\n{\n  return ";
    if (options["bison"].empty())
    {
      if (!options["namespace"].empty())
        *out << options["namespace"] << "::";
      if (!options["yyclass"].empty())
        *out << options["yyclass"];
      else if (!options["class"].empty())
        *out << options["class"];
      else
        *out << options["lexer"];
      *out << "()." << options["lex"] << "()";
    }
    else
    {
      *out << "yylex()";
    }
    *out << ";\n}\n";
  }
}

void Reflex::write_stats(void)
{
  if (!options["verbose"].empty())
  {
    std::cout << "reflex " REFLEX_VERSION " usage statistics:\n" << "  options used:\n";
    for (Map::const_iterator option = options.begin(); option != options.end(); ++option)
      if (!option->second.empty())
        std::cout << "    " << option->first << "=" << option->second << std::endl;
    if (!options["verbose"].empty())
      std::cout << "  inclusive (%s) and exclusive (%x) start conditions:\n";
  }
  if (!options["matcher"].empty())
  {
    if (!options["verbose"].empty())
    {
      for (Start start = 0; start < conditions.size(); ++start)
      {
        std::cout << "    ";
        if (inclusive.find(start) != inclusive.end())
          std::cout << "%s ";
        else
          std::cout << "%x ";
        std::cout << conditions[start] << ": " << rules[start].size() << " rules\n";
      }
    }
  }
  else
  {
    for (Start start = 0; start < conditions.size(); ++start)
    {
      std::string option = "m;r";
      if (!options["case_insensitive"].empty())
        option.append(";i");
      if (!options["dotall"].empty())
        option.append(";s");
      if (!options["freespace"].empty())
        option.append(";x");
      option.append(";n=").append(conditions[start]);
      if (options["graphs_file"] == "true")
        option.append(";f=reflex.").append(conditions[start]).append(".gv");
      else if (!options["graphs_file"].empty())
        option.append(";f=").append(start > 0 ? "+" : "").append(file_ext(options["graphs_file"], "gv"));
      if (!options["fast"].empty())
        option.append(";o");
      if (options["tables_file"] == "true")
        option.append(";f=reflex.").append(conditions[start]).append(".cpp");
      else if (!options["tables_file"].empty())
        option.append(";f=").append(start > 0 ? "+" : "").append(file_ext(options["tables_file"], "cpp"));
      if ((!options["full"].empty() || !options["fast"].empty()) && options["tables_file"].empty() && options["stdout"].empty())
        option.append(";f=+").append(options["outfile"]);
      try
      {
        reflex::Pattern pattern(patterns[start], option);
        reflex::Pattern::Index accept = 1;
        for (size_t rule = 0; rule < rules[start].size(); ++rule)
          if (rules[start][rule].regex != "<<EOF>>")
            if (!pattern.reachable(accept++))
              warning("rule cannot be matched", "", rules[start][rule].code.lineno);
        reflex::Pattern::Index n = 0;
        if (!patterns[start].empty())
          n = pattern.size();
        if (!options["verbose"].empty())
        {
          std::cout << "    ";
          if (inclusive.find(start) != inclusive.end())
            std::cout << "%s ";
          else
            std::cout << "%x ";
          std::cout << conditions[start] << ": " << n;
          if (n < rules[start].size())
            std::cout << "+1(EOF)";
          std::cout << " rules, " << pattern.nodes() << " nodes, " << pattern.edges() << " edges, " << pattern.words() << " code words\n";
        }
      }
      catch (reflex::Pattern::Error& e)
      {
        e.display();
        abort("in reading ", infile.c_str());
      }
    }
  }
}

void Reflex::write_regex(const std::string& regex)
{
  for (std::string::const_iterator i = regex.begin(); i != regex.end(); ++i)
  {
    if (*i == '\\')
      *out << "\\\\";
    else if (*i == '"')
      *out << "\\\"";
    else
      *out << *i;
  }
}
