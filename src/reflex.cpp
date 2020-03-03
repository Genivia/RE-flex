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
@copyright (c) 2015-2019, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#include "reflex.h"

/// Work around the Boost.Regex partial_match bug by forcing the generated scanner to buffer all input
#define WITH_BOOST_PARTIAL_MATCH_BUG

/// Safer fopen_s()
#if (!defined(__WIN32__) && !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64) && !defined(__BORLANDC__)) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
inline int fopen_s(FILE **file, const char *name, const char *mode) { return (*file = ::fopen(name, mode)) ? 0 : errno; }
#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Static data                                                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// @brief Table with command-line reflex options and lex specification %%options.
///
/// The table consists of option names with hyphens replaced by underscores.
static const char *options_table[] = {
  "array",
  "always_interactive",
  "batch",
  "bison",
  "bison_bridge",
  "bison_cc",
  "bison_cc_namespace",
  "bison_cc_parser",
  "bison_complete",
  "bison_locations",
  "case_insensitive",
  "class",
  "debug",
  "default",
  "dotall",
  "exception",
  "extra_type",
  "fast",
  "find",
  "flex",
  "freespace",
  "full",
  "graphs_file",
  "header_file",
  "include",
  "indent",
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
  "nocase_insensitive",
  "nodebug",
  "nodefault",
  "nodotall",
  "nofreespace",
  "noindent",
  "noinput",
  "noline",
  "nomain",
  "nopointer",
  "nostack",
  "nostdinit",
  "nounicode",
  "nounistd",
  "nounput",
  "nowarn",
  "noyylineno",
  "noyymore",
  "noyywrap",
  "outfile",
  "pattern",
  "pointer",
  "perf_report",
  "posix",
  "prefix",
  "reentrant",
  "regexp_file",
  "stack",
  "stdinit",
  "stdout",
  "tables_file",
  "tabs",
  "token_eof",
  "token_type",
  "unicode",
  "unput",
  "verbose",
  "warn",
  "yyclass",
  "yylineno",
  "yymore",
  "yywrap",
  "YYLTYPE",
  "YYSTYPE",
  "7bit",
  "8bit",
  NULL // end of table
};

/// @brief Table with regex library properties.
///
/// This table is extensible and new regex libraries may be added.  Each regex library is described by:
///
/// - a unique name that is used for specifying the `matcher=NAME` option
/// - the header file to be included
/// - the pattern type or class used by the matcher class
/// - the matcher class
/// - the regex library signature
///
/// A regex library signature is a string of the form `"decls:escapes?+."`, see reflex::convert.
///
/// The optional `"decls:"` part specifies which modifiers and other special `(?...)` constructs are supported:
/// - non-capturing group `(?:...)` is supported
/// - one or all of "imsx" specify which (?ismx) modifiers are supported:
/// - 'i' specifies that `(?i...)` case-insensitive matching is supported
/// - 'm' specifies that `(?m...)` multiline mode is supported for the ^ and $ anchors
/// - 's' specifies that `(?s...)` dotall mode is supported
/// - 'x' specifies that `(?x...)` freespace mode is supported
/// - `#` specifies that `(?#...)` comments are supported
/// - `=` specifies that `(?=...)` lookahead is supported
/// - `<` specifies that `(?<...)` lookbehind is supported
/// - `!` specifies that `(?!=...)` and `(?!<...)` are supported
/// - `^` specifies that `(?^...)` negative (reflex) patterns are supported
///
/// The `"escapes"` characters specify which standard escapes are supported:
/// - `a` for `\a` (BEL U+0007)
/// - `b` for `\b` (BS U+0008) in brackets `[\b]` only AND the `\b` word boundary
/// - `c` for `\cX` control character specified by `X` modulo 32
/// - `d` for `\d` ASCII digit `[0-9]`
/// - `e` for `\e` ESC U+001B
/// - `f` for `\f` FF U+000C
/// - `h` for `\h` ASCII blank `[ \t]` (SP U+0020 or TAB U+0009)
/// - `i` for `\i` reflex indent anchor
/// - `j` for `\j` reflex dedent anchor
/// - `j` for `\k` reflex undent anchor
/// - `l` for `\l` ASCII lower case letter `[a-z]`
/// - `n` for `\n` LF U+000A
/// - `p` for `\p{C}` Unicode character classes, also implies Unicode \x{X}, \l, \u, \d, \s, \w
/// - `r` for `\r` CR U+000D
/// - `s` for `\s` space (SP, TAB, LF, VT, FF, or CR)
/// - `t` for `\t` TAB U+0009
/// - `u` for `\u` ASCII upper case letter `[A-Z]` (when not followed by `{XXXX}`)
/// - `v` for `\v` VT U+000B
/// - `w` for `\w` ASCII word-like character `[0-9A-Z_a-z]`
/// - `x` for `\xXX` 8-bit character encoding in hexadecimal
/// - `y` for `\y` word boundary
/// - `z` for `\z` end of input anchor
/// - ``` for `\`` begin of input anchor
/// - `'` for `\'` end of input anchor
/// - `<` for `\<` left word boundary
/// - `>` for `\>` right word boundary
/// - `A` for `\A` begin of input anchor
/// - `B` for `\B` non-word boundary
/// - `D` for `\D` ASCII non-digit `[^0-9]`
/// - `H` for `\H` ASCII non-blank `[^ \t]`
/// - `L` for `\L` ASCII non-lower case letter `[^a-z]`
/// - `N` for `\N` not a newline
/// - `P` for `\P{C}` Unicode inverse character classes, see 'p'
/// - `Q` for `\Q...\E` quotations
/// - `R` for `\R` Unicode line break
/// - `S` for `\S` ASCII non-space (no SP, TAB, LF, VT, FF, or CR)
/// - `U` for `\U` ASCII non-upper case letter `[^A-Z]`
/// - `W` for `\W` ASCII non-word-like character `[^0-9A-Z_a-z]`
/// - `X` for `\X` any Unicode character
/// - `Z` for `\Z` end of input anchor, before the final line break
/// - `0` for `\0nnn` 8-bit character encoding in octal requires a leading `0`
/// - '1' to '9' for backreferences (not applicable to lexer specifications)
///
/// Note that 'p' is a special case to support Unicode-based matchers that
/// natively support UTF8 patterns and Unicode classes \p{C}, \P{C}, \w, \W,
/// \d, \D, \l, \L, \u, \U, \N, and \x{X}.  Basically, 'p' prevents conversion
/// of Unicode patterns to UTF8.  This special case does not support {NAME}
/// expansions in bracket lists such as [a-z||{upper}] and {lower}{+}{upper}
/// used in lexer specifications.
///
/// The optional `"?+"` specify lazy and possessive support:
/// - `?` lazy quantifiers for repeats are supported
/// - `+` possessive quantifiers for repeats are supported
///
/// The optional `"."` (dot) specifies that dot matches any character except newline.
/// A dot is implied by the presence of the 's' modifier, and can be omitted in that case.
static const Reflex::Library library_table[] = {
  {
    "reflex",
    "reflex/matcher.h",
    "reflex::Pattern",
    "reflex::Matcher",
    "imsx#=^:abcdefhijklnrstuvwxzABDHLNQSUW<>?+.",
  },
  {
    "boost",
    "reflex/boostmatcher.h",
    "boost::regex",
    "reflex::BoostPosixMatcher",
    "imsx#<=!:abcdefghlnrstuvwxzABDHLQSUWZ0<>.",
  },
  {
    "boost_perl",
    "reflex/boostmatcher.h",
    "boost::regex",
    "reflex::BoostMatcher",
    "imsx#<=!:abcdefghlnrstuvwxzABDHLQSUWZ0<>?+.",
  },
  {
    "pcre2_perl",
    "reflex/pcre2matcher.h",
    "std::string",
    "reflex::PCRE2Matcher",
    "imsx!#<=:abcdefghlnrstuvwxzABDGHKLNQRSUWXZ0?+.",
  },
  {
    "std_ecma", // this is an experimental option, not recommended!!
    "reflex/stdmatcher.h",
    "char *",
    "/* EXPERIMENTAL OPTION, NOT RECOMMENDED */ reflex::StdEcmaMatcher",
    "!=:bcdfnrstvwxBDSW?"
  },
  { NULL, NULL, NULL, NULL, NULL } // end of table
};

#ifdef OS_WIN
static const char *newline = "\r\n";
#else
static const char *newline = "\n";
#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Helper functions                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// Convert to lower case
inline int lower(int c)
  /// @returns lower case char
{
  return std::isalpha(c) ? (c | 0x20) : c;
}

/// Add file extension if not present, modifies the string argument and returns a copy
static std::string file_ext(std::string& name, const char *ext)
  /// @returns copy of file `name` string with extension `ext`
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

/// Main program instantiates Reflex class and runs `Reflex::main(argc, argv)`
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

/// Main program
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

/// Reflex initialization
void Reflex::init(int argc, char **argv)
{
#ifdef OS_WIN
  color_term = false;
#else
  const char *term = getenv("TERM");
  color_term = term && (strstr(term, "ansi") || strstr(term, "xterm") || strstr(term, "color"));
#endif
  for (const char *const *i = options_table; *i != NULL; ++i)
    options[*i] = "";
  for (const Library *j = library_table; j->name != NULL; ++j)
    libraries[j->name] = *j;
  library = &libraries["reflex"];
  conditions.push_back("INITIAL");
  inclusive.insert(0);
  out = &std::cout;
  lineno = 0;

  for (int i = 1; i < argc; ++i)
  {
    const char *arg = argv[i];
    if (*arg == '-'
     || (arg[0] == '\xE2' && arg[1] == '\x88' && arg[2] == '\x92') // UTF-8 Unicode minus sign U+2212
#ifdef OS_WIN
     || *arg == '/'
#endif
     )
    {
      bool is_grouped = true;
      if (arg[1] == '\x88' && arg[2] == '\x92') // UTF-8 Unicode minus sign U+2212
        arg += 2;
      while (is_grouped && *++arg)
      {
        switch (*arg)
        {
          case '\xE2':
             if (arg[1] != '\x88' || arg[2] != '\x92') // UTF-8 Unicode minus sign U+2212
               break;
             arg += 2;
             // fall through
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
              StringMap::iterator i = options.find(name);
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
            options["debug"] = "true";
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
          case 'i':
            options["case_insensitive"] = "true";
            break;
          case 'I':
            options["interactive"] = "true";
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
            else if (++i < argc && *argv[i] != '-')
              options["matcher"] = argv[i];
            else
              help("missing NAME for option -m NAME");
            is_grouped = false;
            break;
          case 'n':
            break;
          case 'o':
            ++arg;
            if (*arg)
              options["outfile"] = &arg[*arg == '='];
            else if (++i < argc && *argv[i] != '-')
              options["outfile"] = argv[i];
            else
              help("missing FILE for option -o FILE");
            is_grouped = false;
            break;
          case 'p':
            options["perf_report"] = "true";
            break;
          case 'P':
            ++arg;
            if (*arg)
              options["prefix"] = &arg[*arg == '='];
            else if (++i < argc && *argv[i] != '-')
              options["prefix"] = argv[i];
            else
              help("missing NAME for option -P NAME");
            is_grouped = false;
            break;
          case 'R':
            options["reentrant"] = "true";
            break;
          case 's':
            options["nodefault"] = "true";
            break;
          case 'S':
            options["find"] = "true";
            break;
          case 't':
            options["stdout"] = "true";
            break;
          case 'T':
            ++arg;
            if (*arg)
              options["tabs"] = &arg[*arg == '='];
            else if (++i < argc && *argv[i] != '-')
              options["tabs"] = argv[i];
            else
              help("missing N for option -T N");
            is_grouped = false;
            break;
          case 'u':
            options["unicode"] = "true";
            break;
          case 'v':
            options["verbose"] = "true";
            break;
          case 'V':
            version();
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
        help("one input FILE argument can be specified, also found ", argv[i]);
      infile = argv[i];
    }
  }

  set_library();
}

/// Display version information and exit
void Reflex::version()
{
  std::cout << "reflex " REFLEX_VERSION " " PLATFORM << "\n"
    "License BSD-3-Clause: <https://opensource.org/licenses/BSD-3-Clause>\n"
    "Written by Robert van Engelen and others: <https://github.com/Genivia/RE-flex>" << std::endl;
  exit(EXIT_SUCCESS);
}

/// Display help information with an optional diagnostic message and exit
void Reflex::help(const char *message, const char *arg)
{
  if (message)
    std::cout
      << "reflex: "
      << message
      << (arg != NULL ? arg : "")
      << std::endl;
  std::cout << "Usage: reflex [OPTIONS] [FILE]\n\
\n\
    Scanner:\n\
        -+, --flex\n\
                generate Flex-compatible C++ scanner\n\
        -a, --dotall\n\
                dot in patterns match newline\n\
        -B, --batch\n\
                generate scanner for batch input by buffering the entire input\n\
        -f, --full\n\
                generate full scanner with FSM opcode tables\n\
        -F, --fast\n\
                generate fast scanner with FSM code\n\
        -i, --case-insensitive\n\
                ignore case in patterns\n\
        -I, --interactive, --always-interactive\n\
                generate interactive scanner\n\
        -m NAME, --matcher=NAME\n\
                match with ";
  for (LibraryMap::const_iterator i = libraries.begin(); i != libraries.end(); ++i)
    std::cout << i->first << ", ";
  std::cout << "...\n\
        --pattern=NAME\n\
                use custom pattern class NAME for custom matcher option -m\n\
        --include=FILE\n\
                include header FILE.h for custom matcher option -m\n\
        -S, --find\n\
                generate search engine to find matches, ignores unmatched input\n\
        -T N, --tabs=N\n\
                set default tab size to N (1,2,4,8) for indent/dedent matching\n\
        -u, --unicode\n\
                match Unicode . (dot), \\p, \\s, \\w, etc and group UTF-8 bytes\n\
        -x, --freespace\n\
                ignore space in patterns\n\
\n\
    Generated files:\n\
        -o FILE, --outfile=FILE\n\
                specify output FILE instead of lex.yy.cpp\n\
        -t, --stdout\n\
                write scanner on stdout instead of lex.yy.cpp\n\
        --graphs-file[=FILE]\n\
                write the scanner's DFA in Graphviz format to FILE.gv\n\
        --header-file[=FILE]\n\
                write a C++ header FILE.h in addition to the scanner\n\
        --regexp-file[=FILE]\n\
                write the scanner's regular expression patterns to FILE.txt\n\
        --tables-file[=FILE]\n\
                write the scanner's FSM opcode tables or FSM code to FILE.cpp\n\
\n\
    Generated code:\n\
        --namespace=NAME\n\
                use C++ namespace NAME for the generated scanner class, with\n\
                multiple namespaces specified as NAME1.NAME2.NAME3 ...\n\
        --lexer=NAME\n\
                use lexer class NAME instead of Lexer or yyFlexLexer\n\
        --lex=NAME\n\
                use lex function NAME instead of lex or yylex\n\
        --class=NAME\n\
                declare a user-defined scanner class NAME\n\
        --yyclass=NAME\n\
                generate Flex-compatible scanner with user-defined class NAME\n\
        --main\n\
                generate main() to invoke lex() or yylex() once\n\
        -L, --noline\n\
                suppress #line directives in scanner\n\
        -P NAME, --prefix=NAME\n\
                use NAME as prefix of the FlexLexer class name and its members\n\
        --nostdinit\n\
                initialize input to std::cin instead of stdin\n\
        --bison\n\
                generate global yylex() scanner for bison\n\
        --bison-bridge\n\
                generate reentrant yylex() scanner for bison pure parser\n\
        --bison-cc\n\
                generate bison C++ interface code for bison lalr1.cc skeleton\n\
        --bison-cc-namespace=NAME\n\
                use namespace NAME with bison lalr1.cc skeleton\n\
        --bison-cc-parser=NAME\n\
                use parser class NAME with bison lalr1.cc skeleton\n\
        --bison-complete\n\
                use bison complete-symbols feature, implies bison-cc\n\
        --bison-locations\n\
                include bison yylloc support\n\
        -R, --reentrant\n\
                generate Flex-compatible yylex() reentrant scanner functions\n\
                NOTE: adds functions only, reflex scanners are always reentrant\n\
        --noyywrap\n\
                do not call global yywrap() on EOF, requires option --flex\n\
        --exception=VALUE\n\
                use exception VALUE to throw in the default rule of the scanner\n\
        --token-type=NAME\n\
                use NAME as the return type of lex() and yylex() instead of int\n\
\n\
    Debugging:\n\
        -d, --debug\n\
                enable debug mode in scanner\n\
        -p, --perf-report\n\
                scanner reports detailed performance statistics to stderr\n\
        -s, --nodefault\n\
                disable the default rule in scanner that echoes unmatched text\n\
        -v, --verbose\n\
                report summary of scanner statistics to stdout\n\
        -w, --nowarn\n\
                do not generate warnings\n\
\n\
    Miscellaneous:\n\
        -c, -n\n\
                do-nothing POSIX options\n\
        -?, -h, --help\n\
                produce this help message and exit\n\
        -V, --version\n\
                report reflex version and exit\n\
\n\
    Lex/Flex-like options that are enabled by default or have no effect:\n\
        --c++                  default\n\
        --lex-compat           n/a\n\
        --never-interactive    default\n\
        --nounistd             n/a\n\
        --posix                n/a\n\
        --stack                n/a\n\
        --warn                 default\n\
        --yylineno             default\n\
        --yymore               default\n\
        --7bit                 n/a\n\
        --8bit                 default\n\
" << std::endl;
  exit(message ? EXIT_FAILURE : EXIT_SUCCESS);
}

/// Set/reset regex library matcher
void Reflex::set_library()
{
  if (!definitions.empty())
    warning("%option matcher should be specified before the start of regular definitions");
  if (options["matcher"] == "reflex")
  {
    options["matcher"].clear();
  }
  else if (!options["matcher"].empty())
  {
    std::string& name = options["matcher"];
    std::string name_ext = name;
    size_t pos;
    while ((pos = name.find('-')) != std::string::npos)
      name[pos] = '_';
    LibraryMap::iterator i = libraries.find(name);
    if (i != libraries.end())
    {
      library = &i->second;
    }
    else
    {
      library = &libraries[name];
      library->name = name.c_str();
      if (options["include"].empty())
        options["include"] = name_ext;
      file_ext(options["include"], "h");
      library->file = options["include"].c_str();
      if (options["pattern"].empty())
        library->pattern = "char *";
      else
        library->pattern = options["pattern"].c_str();
      library->matcher = name.c_str();
      library->signature = "m:";
      warning("using custom matcher ", library->name);
    }
  }
}

/// Parse lex specification input
void Reflex::parse()
{
  FILE *file = stdin;
  if (!infile.empty())
  {
    fopen_s(&file, infile.c_str(), "r");
    if (file == NULL)
      abort("cannot open file ", infile.c_str());
  }
  in = file;
  parse_section_1();
  parse_section_2();
  parse_section_3();
  if (file != stdin)
    fclose(file);
}

/// Parse the specified %%include file
void Reflex::include(const std::string& filename)
{
  FILE *file = NULL;
  fopen_s(&file, filename.c_str(), "r");
  if (file == NULL)
    abort("cannot include file ", filename.c_str());
  std::string save_infile(infile);
  infile = filename;
  reflex::BufferedInput save_in(in);
  in = file;
  std::string save_line(line);
  size_t save_lineno = lineno;
  size_t save_linelen = linelen;
  lineno = 0;
  parse_section_1();
  fclose(file);
  infile = save_infile;
  in = save_in;
  line = save_line;
  lineno = save_lineno;
  linelen = save_linelen;
}

/// Fetch next line from the input, return true if ok
bool Reflex::get_line()
{
  if (in.eof())
    return false;
  if (!in.good())
    abort("error in reading");
  ++lineno;
  line.clear();
  int c;
  while ((c = in.get()) != EOF && c != '\n')
  {
    if (c != '\r')
      line.push_back(c);
  }
  linelen = line.length();
  while (linelen > 0 && std::isspace(line.at(linelen - 1)))
    --linelen;
  line.resize(linelen);
  if (in.eof() && line.empty())
    return false;
  return true;
}

/// Advance pos over white space and comments, return true if ok
bool Reflex::skip_comment(size_t& pos)
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
        if (!get_line())
          return false;
        pos = 0;
      }
      pos += 2;
      if (pos >= linelen)
      {
        if (!get_line())
          return false;
        pos = 0;
      }
      continue;
    }
    if (pos < linelen)
      return true;
    if (!get_line())
      return false;
    pos = 0;
  }
}

/// Match case-insensitive string s while ignoring the rest of the line, return true if OK
bool Reflex::is(const char *s)
{
  for (size_t pos = 0; pos < linelen && *s != '\0' && lower(line.at(pos)) == *s; ++pos, ++s)
    continue;
  return *s == '\0';
}

/// Advance pos to match case-insensitive initial part of the string s followed by white space, return true if OK
bool Reflex::as(size_t& pos, const char *s)
{
  if (pos >= linelen || *s == '\0' || lower(line.at(pos)) != *s++)
    return false;
  while (++pos < linelen && *s != '\0' && lower(line.at(pos)) == *s++)
    continue;
  return ws(pos);
}

/// Advance pos over whitespace, returns true if OK
bool Reflex::ws(size_t& pos)
{
  if (pos >= linelen || (pos > 0 && !std::isspace(line.at(pos))))
    return false;
  while (pos < linelen && std::isspace(line.at(pos)))
    ++pos;
  return true;
}

/// Advance pos over '=' and whitespace when present, return true if OK
bool Reflex::eq(size_t& pos)
{
  (void)ws(pos);
  if (pos + 1 >= linelen || line.at(pos) != '=')
    return false;
  ++pos;
  (void)ws(pos);
  return true;
}

/// Advance pos to end of line while skipping whitespace, return true if end of line
bool Reflex::nl(size_t& pos)
{
  while (pos < linelen && std::isspace(line.at(pos)))
    ++pos;
  return pos >= linelen;
}

/// Check if current line starts a block of code or a comment
bool Reflex::is_code()
{
  return linelen > 0 && ((std::isspace(line.at(0)) && options["freespace"].empty()) || is("%{") || is("//") || is("/*"));
}

/// Check if current line starts a block of %top code
bool Reflex::is_topcode()
{
  size_t pos = 0;
  return is("%top{") || (as(pos, "%top") && pos < linelen && line.at(pos) == '{');
}

/// Check if current line starts a block of %class code
bool Reflex::is_classcode()
{
  size_t pos = 0;
  return is("%class{") || (as(pos, "%class") && pos < linelen && line.at(pos) == '{');
}

/// Check if current line starts a block of %init code
bool Reflex::is_initcode()
{
  size_t pos = 0;
  return is("%init{") || (as(pos, "%init") && pos < linelen && line.at(pos) == '{');
}

/// Advance pos over name (letters, digits, ., -, _ or any non-ASCII character > U+007F), return name
std::string Reflex::get_name(size_t& pos)
{
  if (pos >= linelen || (!std::isalnum(line.at(pos)) && line.at(pos) != '_' && (line.at(pos) & 0x80) != 0x80))
    return "";
  size_t loc = pos++;
  while (pos < linelen)
  {
    if (!std::isalnum(line.at(pos)) && line.at(pos) != '_' && line.at(pos) != '-' && line.at(pos) != '.' && (line.at(pos) & 0x80) != 0x80)
      break;
    ++pos;
  }
  return line.substr(loc, pos - loc);
}

/// Advance pos over option name or namespace (letters, digits, ::, ., -, _ or any non-ASCII character > U+007F), return name
std::string Reflex::get_namespace(size_t& pos)
{
  size_t loc = pos++;
  while (pos < linelen)
  {
    if (line.at(pos) == ':' && pos + 1 < linelen && line.at(pos + 1) == ':') // parse ::
      ++pos;
    else if (!std::isalnum(line.at(pos)) && line.at(pos) != '_' && line.at(pos) != '-' && line.at(pos) != '.' && (line.at(pos) & 0x80) != 0x80)
      break;
    ++pos;
  }
  return line.substr(loc, pos - loc);
}

/// Advance pos over option name (letters, digits, +/hyphen/underscore), return name
std::string Reflex::get_option(size_t& pos)
{
  if (pos >= linelen || !std::isalnum(line.at(pos)))
    return "";
  size_t loc = pos++;
  while (pos < linelen)
  {
    if (line.at(pos) == '-' || line.at(pos) == '+') // normalize - and + to _
      line[pos] = '_';
    else if (!std::isalnum(line.at(pos)) && line.at(pos) != '_')
      break;
    ++pos;
  }
  return line.substr(loc, pos - loc);
}

/// Advance pos over start condition name (an ASCII C++ identifier or C++11 Unicode identifier), return name
std::string Reflex::get_start(size_t& pos)
{
  if (pos >= linelen || (!std::isalpha(line.at(pos)) && line.at(pos) != '_' && (line.at(pos) & 0x80) != 0x80))
    return "";
  size_t loc = pos++;
  while (pos < linelen)
  {
    if (line.at(pos) == '-') // normalize - to _
      line[pos] = '_';
    else if (!std::isalnum(line.at(pos)) && line.at(pos) != '_' && (line.at(pos) & 0x80) != 0x80)
      break;
    ++pos;
  }
  return line.substr(loc, pos - loc);
}

/// Advance pos over quoted string, return string
std::string Reflex::get_string(size_t& pos)
{
  if (pos >= linelen || line.at(pos) != '"')
    return "";
  std::string string;
  while (++pos < linelen && line.at(pos) != '"')
  {
    if (line.at(pos) == '\\' && (line.at(pos + 1) == '"' || line.at(pos + 1) == '\\'))
      ++pos;
    string.push_back(line.at(pos));
  }
  ++pos;
  return string;
}

/// Get regex string, converted to a format understood by the selected regex engine library
std::string Reflex::get_regex(size_t& pos)
{
  std::string regex;
  size_t at_lineno = lineno;
  (void)ws(pos); // skip indent, if any
  size_t loc = pos;
  bool fsp = !options["freespace"].empty();
  size_t nsp = pos;
  while (pos < linelen)
  {
    int c = line.at(pos);
    if (fsp)
    {
      if (nsp < pos && (
            (c == '{' && (pos + 1 == linelen || std::isspace(line.at(pos + 1)))) ||
            (c == '|' && pos + 1 == linelen) ||
            (c == '/' && pos + 1 < linelen && (line.at(pos + 1) == '/' || line.at(pos + 1) == '*'))))
      {
        pos = nsp;
        break;
      }
    }
    else if (std::isspace(c))
    {
      break;
    }
    ++pos;
    if (c == '"')
    {
      // eat "..."
      while (pos < linelen && line.at(pos) != '"')
        pos += 1 + (line.at(pos) == '\\');
      ++pos;
    }
    else if (c == '[')
    {
      // eat [...]
      if (pos < linelen && line.at(pos) == '^')
        ++pos;
      if (line.at(pos) != '\\')
        ++pos;
      while (pos < linelen && line.at(pos) != ']')
        pos += 1 + (line.at(pos) == '\\');
      ++pos;
    }
    else if (c == '(' && pos + 2 < linelen && line.at(pos + 1) == '?' && line.at(pos + 2) == '#')
    {
      // eat (?#...)
      pos += 2; 
      while (pos < linelen && line.at(pos) != ')')
        pos += 1 + (line.at(pos) == '\\');
      ++pos;
    }
    else if (c == '\\')
    {
      if (pos == linelen)
      {
        // line ends in \ and continues on the next line
        regex.append(line.substr(loc, pos - loc));
        if (!get_line())
          error("EOF encountered inside a pattern", NULL, at_lineno);
        if (is("%%"))
          error("%% section ending encountered inside a pattern", NULL, at_lineno);
        pos = 0;
        (void)ws(pos); // skip indent, if any
        loc = pos;
      }
      else if (line.at(pos) == 'Q')
      {
        // eat \Q...\E
        while (pos + 1 < linelen && (line.at(pos) != '\\' || line.at(pos + 1) != 'E'))
          ++pos;
        pos += 2;
      }
      else
      {
        ++pos;
      }
    }
    if (fsp && !std::isspace(c))
      nsp = pos;
  }
  regex.append(line.substr(loc, pos - loc));
  if (regex != "<<EOF>>")
  {
    reflex::convert_flag_type flags = reflex::convert_flag::lex;
    if (!options["case_insensitive"].empty())
      flags |= reflex::convert_flag::anycase;
    if (!options["dotall"].empty())
      flags |= reflex::convert_flag::dotall;
    if (!options["freespace"].empty())
      flags |= reflex::convert_flag::freespace;
    if (!options["unicode"].empty())
      flags |= reflex::convert_flag::unicode;
    try
    {
      regex = reflex::convert(regex, library->signature, flags, &definitions); 
    }
    catch (reflex::regex_error& e)
    {
      error("malformed regular expression or unsupported syntax\n", e.what(), at_lineno);
    }
  }
  return regex;
}

/// Get line(s) of code, %{ %}, %%top{ %}, %%class{ %}, and %%init{ %}
std::string Reflex::get_code(size_t& pos)
{
  std::string code;
  size_t at_lineno = lineno;
  size_t blk = 0, lev = 0;
  enum { CODE, STRING, CHAR, COMMENT } tok = CODE;
  if (pos == 0 && (is("%{") || is_topcode() || is_classcode() || is_initcode()))
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
      if (!get_line())
        error("EOF encountered inside an action, %} or %% expected", NULL, at_lineno);
      pos = 0;
      if (tok == CODE)
      {
        if ((blk > 0 || lev > 0) && is("%%"))
          error("%% section ending encountered inside an action", NULL, at_lineno);
        if (is("%{"))
        {
          code.append(newline);
          ++blk;
        }
        else if (is("%}"))
        {
          code.append(newline);
          if (blk > 0)
            --blk;
          if (blk == 0 && lev == 0)
          {
            if (!get_line())
              error("EOF encountered inside an action, %} or %% expected", NULL, at_lineno);
            return code;
          }
        }
        else
        {
          if (blk == 0 && lev == 0 && linelen > 0 && (!std::isspace(line.at(0)) || !options["freespace"].empty()))
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

/// Returns string with all \ replaced by \\ to stringify file paths
std::string Reflex::escape_bs(const std::string& s)
{
  std::string t = s;
  size_t i = 0;
  while ((i = t.find('\\', i)) != std::string::npos)
  {
    t.replace(i, 1, "\\\\");
    i += 2;
  }
  return t;
}

/// Add start conditions <start1,start2,...> or subtract them with <-start1,-start2,...>
bool Reflex::get_starts(size_t& pos, Starts& starts)
{
  pos = 0;
  if (linelen > 1 && line.at(0) == '<' && (std::isalpha(line.at(1)) || line.at(1) == '_' || line.at(1) == '*' || (line.at(1) & 0x80) == 0x80 || line.at(1) == '^') && line.find('>') != std::string::npos)
  {
    do
    {
      ++pos;
      if (pos >= linelen - 1)
        break;
      bool reverse = line.at(pos) == '^';
      if (reverse)
        ++pos;
      if (line.at(pos) == '*')
      {
        if (reverse)
        {
          starts.clear();
        }
        else
        {
          for (Start start = 0; start < conditions.size(); ++start)
            starts.insert(start);
        }
        ++pos;
      }
      else
      {
        std::string name = get_start(pos);
        if (name.empty())
          break;
        Start start;
        for (start = 0; start < conditions.size() && name != conditions.at(start); ++start)
          continue;
        if (start == conditions.size())
          error("undeclared start condition ", name.c_str());
        Starts::iterator i = starts.find(start);
        if (i != starts.end())
        {
          if (reverse)
            starts.erase(i);
          else
            warning("scope already includes start condition ", name.c_str());
        }
        else
        {
          if (reverse)
            warning("scope does not include start condition ", name.c_str());
          else
            starts.insert(start);
        }
      }
    } while (pos < linelen && line.at(pos) == ',');
    if (pos + 1 >= linelen || line.at(pos) != '>')
      error("bad start condition: ", line.c_str());
    ++pos;
    return true;
  }
  return false;
}

/// Abort with an error message
void Reflex::abort(const char *message, const char *arg)
{
  std::cerr <<
    SGR("\033[0m") << "reflex: " <<
    SGR("\033[1;31m") << "error: " << SGR("\033[0m") <<
    message <<
    SGR("\033[1m") << (arg != NULL ? arg : "") << SGR("\033[0m") <<
    std::endl;
  exit(EXIT_FAILURE);
}

/// Report an error and exit
void Reflex::error(const char *message, const char *arg, size_t at_lineno)
{
  std::cerr <<
    SGR("\033[0m") << (infile.empty() ? "(stdin)" : infile.c_str()) << ":" << (at_lineno ? at_lineno : lineno) << ": " <<
    SGR("\033[1;31m") << "error: " << SGR("\033[0m") <<
    message <<
    SGR("\033[1m") << (arg != NULL ? arg : "") << SGR("\033[0m") <<
    std::endl;
  exit(EXIT_FAILURE);
}

/// Report a warning
void Reflex::warning(const char *message, const char *arg, size_t at_lineno)
{
  if (options["nowarn"].empty())
    std::cerr <<
      SGR("\033[0m") << (infile.empty() ? "(stdin)" : infile.c_str()) << ":" << (at_lineno ? at_lineno : lineno) << ": " <<
      SGR("\033[1;35m") << "warning: " << SGR("\033[0m") <<
      message <<
      SGR("\033[1m") << (arg != NULL ? arg : "") << SGR("\033[0m") <<
      std::endl;
}

/// Parse section 1 of a lex specification
void Reflex::parse_section_1()
{
  if (!get_line())
    return;
  while (!is("%%"))
  {
    if (linelen == 0)
    {
      if (!get_line())
        return;
    }
    else
    {
      if (is_code())
      {
        size_t pos = 0;
        size_t this_lineno = lineno;
        std::string code = get_code(pos);
        section_1.push_back(Code(code, infile, this_lineno));
      }
      else if (is_topcode())
      {
        size_t pos = 0;
        size_t this_lineno = lineno;
        std::string code = get_code(pos);
        section_top.push_back(Code(code, infile, this_lineno));
      }
      else if (is_classcode())
      {
        size_t pos = 0;
        size_t this_lineno = lineno;
        std::string code = get_code(pos);
        section_class.push_back(Code(code, infile, this_lineno));
      }
      else if (is_initcode())
      {
        size_t pos = 0;
        size_t this_lineno = lineno;
        std::string code = get_code(pos);
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
              std::string filename;
              if (line.at(pos) == '"')
                filename = get_string(pos); // %include "NAME"
              else
                filename = get_name(pos); // %include NAME
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
                std::string name = get_start(pos);
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
                  std::string name = get_start(pos);
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
                  std::string name = get_option(pos);
                  if (name.empty())
                    error("bad %option name or value");
                  if (name != "c__") // %option c++ has no effect
                  {
                    StringMap::iterator i = options.find(name);
                    if (i == options.end())
                      error("unrecognized %option: ", name.c_str());
                    (void)ws(pos);
                    if (eq(pos) && pos < linelen)
                    {
                      std::string value;
                      if (line.at(pos) == '"')
                        value = get_string(pos); // %option OPTION = "NAME"
                      else
                        value = get_namespace(pos); // %option OPTION = NAME
                      (void)ws(pos);
                      if (!i->second.empty() && i->second.compare(value) != 0)
                        warning("redefining %option ", name.c_str());
                      i->second = value;
                    }
                    else
                    {
                      i->second = "true";
                      if (name.compare(0, 2, "no") == 0)
                      {
                        StringMap::iterator j = options.find(name.substr(2));
                        if (j != options.end() && j->second.compare("true") == 0)
                        {
                          warning("disabling an initially enabled %option ", name.c_str() + 2);
                          j->second.clear();
                        }
                      }
                      else
                      {
                        std::string noname("no");
                        StringMap::iterator j = options.find(noname.append(name));
                        if (j != options.end() && j->second.compare("true") == 0)
                        {
                          warning("enabling an initially disabled %option ", noname.c_str());
                          j->second.clear();
                        }
                      }
                    }
                    if (!option && !nl(pos))
                      error("trailing text after %option: ", name.c_str());
                    if (name.compare("matcher") == 0)
                      set_library();
                  }
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
          if ((name = get_name(pos)).empty() || !ws(pos) || (regex = get_regex(pos)).empty() || !nl(pos))
            error("bad line in section 1: ", line.c_str());
          if (definitions.find(name) != definitions.end())
            error("attempt to redefine ", name.c_str());
          definitions[name] = regex;
        }
        if (!get_line())
          return;
      }
    }
  }
}

/// Parse section 2 of a lex specification
void Reflex::parse_section_2()
{
  if (in.eof())
    error("missing %% section 2");
  bool init = true;
  std::stack<Starts> scopes;
  if (!get_line())
    return;
  while (!is("%%"))
  {
    if (linelen == 0)
    {
      if (!get_line())
        break;
    }
    else
    {
      size_t pos = 0;
      if (init && is_code())
      {
        std::string code = get_code(pos);
        if (scopes.empty())
        {
          for (Start start = 0; start < conditions.size(); ++start)
          {
            if (inclusive.find(start) != inclusive.end())
              section_2[start].push_back(Code(code, infile, lineno));
          }
        }
        else
        {
          for (Starts::const_iterator start = scopes.top().begin(); start != scopes.top().end(); ++start)
            section_2[*start].push_back(Code(code, infile, lineno));
        }
      }
      else if (is("}") && !scopes.empty())
      {
        scopes.pop();
        if (!get_line())
          break;
      }
      else
      {
        if (!skip_comment(pos) || is("%%"))
          break;
        Starts starts;
        if (!scopes.empty())
          starts = scopes.top();
        bool has_starts = get_starts(pos, starts);
        if (has_starts && pos < linelen && line.at(pos) == '{' && (pos + 1 == linelen || std::isspace(line.at(pos + 1))))
        {
          scopes.push(starts);
          if (!get_line())
            error("EOF encountered inside an action, %} or %% expected");
          init = true;
        }
        else
        {
          if ((has_starts && starts.empty()) || (!has_starts && !scopes.empty() && scopes.top().empty()))
            warning("rule cannot be matched because the scope of start conditions is empty");
          std::string regex = get_regex(pos);
          if (regex.empty())
            error("bad line in section 2: ", line.c_str());
          size_t rule_lineno = lineno;
          std::string code = get_code(pos);
          if (has_starts)
          {
            for (Starts::const_iterator start = starts.begin(); start != starts.end(); ++start)
              rules[*start].push_back(Rule(regex, Code(code, infile, rule_lineno)));
          }
          else if (scopes.empty() && regex == "<<EOF>>")
          {
            // only the first <<EOF>> code should be used
            if (code == "|")
              error("bad <<EOF>> action | in section 2: ", line.c_str());
            for (Start start = 0; start < conditions.size(); ++start)
              rules[start].push_back(Rule(regex, Code(code, infile, rule_lineno)));
          }
          else if (!scopes.empty())
          {
            for (Starts::const_iterator start = scopes.top().begin(); start != scopes.top().end(); ++start)
              rules[*start].push_back(Rule(regex, Code(code, infile, rule_lineno)));
          }
          else
          {
            for (Starts::const_iterator start = inclusive.begin(); start != inclusive.end(); ++start)
              rules[*start].push_back(Rule(regex, Code(code, infile, rule_lineno)));
          }
          init = false;
        }
      }
    }
  }
  if (!scopes.empty())
  {
    const char *name = conditions.at(*scopes.top().begin()).c_str();
    if (in.eof())
      error("EOF encountered inside scope ", name);
    else
      error("%% section ending encountered inside scope ", name);
  }
  patterns.resize(conditions.size());
  for (Start start = 0; start < conditions.size(); ++start)
  {
    std::string& pattern = patterns[start];
    pattern.assign("(?m");
    if (!options["case_insensitive"].empty())
      pattern.push_back('i');
    if (!options["dotall"].empty())
      pattern.push_back('s');
    if (!options["freespace"].empty())
      pattern.push_back('x');
    pattern.append(")%");
    try
    {
      pattern.assign(reflex::convert(pattern, library->signature, reflex::convert_flag::none));
    }
    catch (reflex::regex_error& e)
    {
      error("malformed regular expression\n", e.what());
    }
    pattern.resize(pattern.size() - 1); // remove dummy % from (?m...)%
    const char *sep = "";
    for (Rules::const_iterator rule = rules[start].begin(); rule != rules[start].end(); ++rule)
    {
      if (rule->regex != "<<EOF>>")
      {
        pattern.append(sep).append("(").append(rule->regex).append(")");
        sep = "|";
      }
    }
  }
}

/// Parse section 3 of a lex specification
void Reflex::parse_section_3()
{
  while (get_line())
    section_3.push_back(Code(line, infile, lineno));
}

/// Write lex.yy.cpp
void Reflex::write()
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
  if (!options["bison_complete"].empty())
    options["bison_cc"] = "true";
  if (!options["namespace"].empty())
    undot_namespace(options["namespace"]);
  if (!options["bison_cc_namespace"].empty())
    undot_namespace(options["bison_cc_namespace"]);
  else if (!options["bison_cc"].empty())
    options["bison_cc_namespace"] = "yy";
  if (!options["bison_cc"].empty() && options["bison_cc_parser"].empty())
    options["bison_cc_parser"] = "parser";
  if (!options["bison_cc"].empty() && options["YYLTYPE"].empty())
    options["YYLTYPE"] = options["bison_cc_namespace"] + "::location";
  if (!options["bison_cc"].empty() && options["YYSTYPE"].empty())
    options["YYSTYPE"] = options["bison_cc_namespace"] + "::" + options["bison_cc_parser"] + "::semantic_type";
  if (!options["bison_complete"].empty() && options["token_type"].empty())
    options["token_type"] = options["bison_cc_namespace"] + "::" + options["bison_cc_parser"] + "::symbol_type";
  if (!options["bison_complete"].empty() && options["token_eof"].empty())
    options["token_eof"] = options["token_type"] + (options["bison_locations"].empty() ? "(0)" : "(0, location())");
  std::string token_type = options["token_type"].empty() ? "int" : options["token_type"];
  std::string yyltype = options["YYLTYPE"].empty() ? "YYLTYPE" : options["YYLTYPE"];
  std::string yystype = options["YYSTYPE"].empty() ? "YYSTYPE" : options["YYSTYPE"];
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
      abort("cannot open file ", options["outfile"].c_str());
    out = &ofs;
  }
  *out << "// " << options["outfile"] << " generated by reflex " REFLEX_VERSION " from " << infile << std::endl;
  write_prelude();
  write_section_top();
  write_defines();
  write_class();
  write_section_1();
  write_lexer();
  write_main();
  write_section_3();
  if (!out->good())
    abort("error in writing");
  if (options["matcher"].empty() && (!options["full"].empty() || !options["fast"].empty()) && options["tables_file"].empty() && options["stdout"].empty())
    write_banner("TABLES");
  if (ofs.is_open())
    ofs.close();
  stats();
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
            abort("cannot open file ", filename.c_str());
          out = &ofs;
        }
      }
      write_regex(NULL, patterns[start]);
      *out << std::endl;
      if (!ofs.good())
        abort("error in writing");
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
      abort("cannot open file ", options["header_file"].c_str());
    out = &ofs;
    *out <<
      "// " << options["header_file"] << " generated by reflex " REFLEX_VERSION " from " << infile << std::endl << std::endl <<
      "#ifndef " << (options["prefix"] == "yy" ? "" : options["prefix"].c_str()) << "REFLEX_HEADER_H" << std::endl <<
      "#define " << (options["prefix"] == "yy" ? "" : options["prefix"].c_str()) << "REFLEX_HEADER_H" << std::endl <<
      "#define " << (options["prefix"] == "yy" ? "" : options["prefix"].c_str()) << "IN_HEADER 1" << std::endl;
    write_prelude();
    write_section_top();
    write_class();
    if (!options["bison_cc"].empty())
    {
      write_banner("BISON C++");
    }
    else if (!options["reentrant"].empty() || !options["bison_bridge"].empty())
    {
      if (!options["bison_locations"].empty())
        write_banner("BISON BRIDGE LOCATIONS");
      else if (!options["bison_bridge"].empty())
        write_banner("BISON BRIDGE");
      else
        write_banner("REENTRANT");
      *out << "typedef ";
      if (!options["namespace"].empty())
        write_namespace_scope();
      if (!options["yyclass"].empty())
        *out << options["yyclass"];
      else if (!options["class"].empty())
        *out << options["class"];
      else
        *out << options["lexer"];
      *out <<
        " yyscanner_t;\n"
        "typedef void *yyscan_t;\n"
        "\n"
        "#ifndef YY_EXTERN_C\n"
        "#define YY_EXTERN_C\n"
        "#endif\n"
        "\n";
      if (!options["bison_locations"].empty())
        *out << "YY_EXTERN_C " << token_type << " yylex(" << yystype << "*, " << yyltype << "*, yyscan_t);\n";
      else if (!options["bison_bridge"].empty())
        *out << "YY_EXTERN_C " << token_type << " yylex(" << yystype << "*, yyscan_t);\n";
      else
        *out << "YY_EXTERN_C " << token_type << " yylex(yyscan_t);\n";
      *out << "YY_EXTERN_C void yylex_init(yyscan_t*);\n";
      if (!options["flex"].empty())
        *out << "YY_EXTERN_C void yylex_init_extra(" << (options["extra_type"].empty() ? "YY_EXTRA_TYPE" : options["extra_type"].c_str()) << ", yyscan_t*);\n";
      *out << "YY_EXTERN_C void yylex_destroy(yyscan_t);\n";
    }
    else if (!options["bison_locations"].empty())
    {
      write_banner("BISON LOCATIONS");
      *out <<
        "extern ";
      if (!options["namespace"].empty())
        write_namespace_scope();
      if (!options["yyclass"].empty())
        *out << options["yyclass"];
      else if (!options["class"].empty())
        *out << options["class"];
      else
        *out << options["lexer"];
      *out <<
        " YY_SCANNER;\n"
        "\n"
        "#ifndef YY_EXTERN_C\n"
        "#define YY_EXTERN_C\n"
        "#endif\n"
        "\n"
        "YY_EXTERN_C " << token_type << " yylex(" << yystype << "*, " << yyltype << "*);\n";
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
        write_namespace_scope();
      if (!options["yyclass"].empty())
        *out << options["yyclass"];
      else if (!options["class"].empty())
        *out << options["class"];
      else
        *out << options["lexer"];
      *out <<
        " YY_SCANNER;\n"
        "\n"
        "#ifndef YY_EXTERN_C\n"
        "#define YY_EXTERN_C\n"
        "#endif\n"
        "\n";
      if (!options["flex"].empty())
        *out << "YY_EXTERN_C int " << options["prefix"] << "lex(void);\n";
      else
        *out << "YY_EXTERN_C " << token_type << " yylex(void);\n";
    }
    *out << std::endl << "#endif" << std::endl;
    if (!out->good())
      abort("error in writing");
    ofs.close();
  }
}

/// Write a banner in lex.yy.cpp
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

/// Write the prelude to lex.yy.cpp
void Reflex::write_prelude()
{
  if (!out->good())
    return;
  write_banner("OPTIONS USED");
  for (StringMap::const_iterator option = options.begin(); option != options.end(); ++option)
  {
    if (!option->second.empty())
    {
      *out << "#define " << (options["prefix"] == "yy" ? "" : options["prefix"].c_str()) << "REFLEX_OPTION_";
      out->width(20);
      *out << std::left << option->first;
      // if option name ends in 'file' then #define the option's value as a string file name
      if (option->first.size() > 4 && option->first.compare(option->first.size() - 4, 4, "file") == 0)
        *out << "\"" << escape_bs(option->second) << "\"" << std::endl;
      else
        *out << option->second << std::endl;
    }
  }
  if (!options["debug"].empty())
    *out << "\n// --debug option enables ASSERT:\n#define ASSERT(c) assert(c)" << std::endl;
  if (!options["perf_report"].empty())
    *out << "\n// --perf-report option requires a timer:\n#include <reflex/timer.h>" << std::endl;
}

/// Write Flex-compatible #defines to lex.yy.cpp
void Reflex::write_defines()
{
  if (!out->good())
    return;
  if (!options["flex"].empty())
  {
    write_banner("FLEX-COMPATIBLE DEFINITIONS");
    size_t num_rules = 0;
    for (Start start = 0; start < conditions.size(); ++start)
      num_rules += rules[start].size();
    for (Start start = 0; start < conditions.size(); ++start)
      *out << "#define " << conditions[start] << " (" << start << ")" << std::endl;
    *out << "#define YY_NUM_RULES (" << num_rules << ")" << std::endl;
  }
}

/// Write the lexer class to lex.yy.cpp
void Reflex::write_class()
{
  if (!out->good())
    return;
  write_banner("REGEX MATCHER");
  if (!options["noindent"].empty())
    *out << "#define WITH_NO_INDENT\n";
  *out << "#include <" << library->file << ">" << std::endl;
  const char *matcher = library->matcher;
  std::string lex = options["lex"];
  std::string token_type = options["token_type"].empty() ? "int" : options["token_type"];
  std::string yyltype = options["YYLTYPE"].empty() ? "YYLTYPE" : options["YYLTYPE"];
  std::string yystype = options["YYSTYPE"].empty() ? "YYSTYPE" : options["YYSTYPE"];
  std::string base;
  if (!options["flex"].empty())
  {
    write_banner("FLEX-COMPATIBLE ABSTRACT LEXER CLASS");
    *out << "#include <reflex/flexlexer.h>" << std::endl;
    if (!options["bison"].empty() && options["reentrant"].empty() && options["bison_bridge"].empty() && options["bison_locations"].empty())
      *out <<
        "#undef yytext\n"
        "#undef yyleng\n"
        "#undef yylineno\n";
    if (!options["namespace"].empty())
    {
      *out << std::endl;
      write_namespace_open();
      *out << std::endl;
    }
    *out << "typedef reflex::FlexLexer" << "<" << matcher << "> FlexLexer;" << std::endl;
    if (!options["namespace"].empty())
    {
      *out << std::endl;
      write_namespace_close();
    }
    base = "FlexLexer";
  }
  else
  {
    write_banner("ABSTRACT LEXER CLASS");
    *out << "#include <reflex/abslexer.h>" << std::endl;
    base.append("reflex::AbstractLexer<").append(matcher).append(">");
  }
  write_banner("LEXER CLASS");
  std::string lexer = options["lexer"];
  if (!options["namespace"].empty())
  {
    write_namespace_open();
    *out << std::endl;
  }
  *out <<
    "class " << lexer << " : public " << base << " {\n";
  write_section_class();
  if (!options["flex"].empty())
  {
    *out <<
      " public:\n"
      "  " << lexer << "(\n"
      "      const reflex::Input& input = reflex::Input(),\n"
      "      std::ostream        *os    = NULL)\n"
      "    :\n"
      "      " << base << "(input, os)\n";
    write_section_init();
    if (!options["bison_complete"].empty())
    {
      if (!options["bison_locations"].empty())
        *out <<
          "  virtual " << yyltype << " location(void) const\n"
          "  {\n"
          "    " << yyltype << " yylloc;\n"
          "    yylloc.begin.line = static_cast<unsigned int>(matcher().lineno());\n"
          "    yylloc.begin.column = static_cast<unsigned int>(matcher().columno());\n"
          "    yylloc.end.line = static_cast<unsigned int>(matcher().lineno_end());\n"
          "    yylloc.end.column = static_cast<unsigned int>(matcher().columno_end());\n"
          "    return yylloc;\n"
          "  }\n";
      *out <<
        "  virtual " << token_type << " " << lex << "(void)";
    }
    else if (!options["bison_cc"].empty())
    {
      if (!options["bison_locations"].empty())
        *out <<
          "  virtual void yylloc_update(" << yyltype << "& yylloc)\n"
          "  {\n"
          "    yylloc.begin.line = static_cast<unsigned int>(matcher().lineno());\n"
          "    yylloc.begin.column = static_cast<unsigned int>(matcher().columno());\n"
          "    yylloc.end.line = static_cast<unsigned int>(matcher().lineno_end());\n"
          "    yylloc.end.column = static_cast<unsigned int>(matcher().columno_end());\n"
          "  }\n"
          "  virtual " << token_type << " yylex(void)\n"
          "  {\n"
          "    LexerError(\"" << lexer << "::yylex invoked but %option bison-cc is used\");\n"
          "    yyterminate();\n"
          "  }\n"
          "  virtual " << token_type << " " << lex << "(" << yystype << " *lvalp, " << yyltype << " *llocp)\n"
          "  {\n"
          "    return " << lex << "(*lvalp, *llocp);\n"
          "  }\n"
          "  virtual " << token_type << " " << lex << "(" << yystype << "& yylval, " << yyltype << "& yylloc)";
      else
        *out <<
          "  virtual " << token_type << " yylex(void)\n"
          "  {\n"
          "    LexerError(\"" << lexer << "::yylex invoked but %option bison-cc is used\");\n"
          "    yyterminate();\n"
          "  }\n"
          "  virtual " << token_type << " " << lex << "(" << yystype << " *lvalp)\n"
          "  {\n"
          "    return yylex(*lvalp);\n"
          "  }\n"
          "  virtual " << token_type << " " << lex << "(" << yystype << "& yylval)";
    }
    else if (!options["bison_locations"].empty())
    {
      *out <<
        "  virtual void yylloc_update(" << yyltype << "& yylloc)\n"
        "  {\n"
        "    yylloc.first_line = static_cast<unsigned int>(matcher().lineno());\n"
        "    yylloc.first_column = static_cast<unsigned int>(matcher().columno());\n"
        "    yylloc.last_line = static_cast<unsigned int>(matcher().lineno_end());\n"
        "    yylloc.last_column = static_cast<unsigned int>(matcher().columno_end());\n"
        "  }\n"
        "  virtual " << token_type << " yylex(void)\n"
        "  {\n"
        "    LexerError(\"" << lexer << "::yylex invoked but %option bison-bridge and/or bison-locations is used\");\n"
        "    yyterminate();\n"
        "  }\n"
        "  virtual " << token_type << " " << lex << "(" << yystype << "& yylval, " << yyltype << "& yylloc)";
    }
    else if (!options["bison_bridge"].empty())
    {
      *out <<
        "  virtual " << token_type << " yylex(void)\n"
        "  {\n"
        "    LexerError(\"" << lexer << "::yylex invoked but %option bison-bridge and/or bison-locations is used\");\n"
        "    yyterminate();\n"
        "  }\n"
        "  virtual " << token_type << " " << lex << "(" << yystype << "& yylval)";
    }
    else
    {
      if (lex != "yylex")
        *out <<
          "  virtual " << token_type << " yylex(void)\n"
          "  {\n"
          "    LexerError(\"" << lexer << "::yylex invoked but %option lex=" << lex << " is used\");\n"
          "    yyterminate();\n"
          "  }\n";
      *out <<
        "  virtual " << token_type << " " << lex << "()";
    }
    if (options["yyclass"].empty())
      *out << ";\n";
    else
      *out <<
        "\n"
        "  {\n"
        "    LexerError(\"" << lexer << "::" << lex << " invoked but %option yyclass=" << options["yyclass"] << " is used\");\n"
        "    yyterminate();\n"
        "  }"
        << std::endl;
  }
  else
  {
    *out <<
      " public:\n"
      "  typedef " << base << " AbstractBaseLexer;\n"
      "  " << lexer << "(\n"
      "      const reflex::Input& input = reflex::Input(),\n" <<
      "      std::ostream&        os    = std::cout)\n"
      "    :\n"
      "      AbstractBaseLexer(input, os)\n";
    write_section_init();
    for (Start start = 0; start < conditions.size(); ++start)
      *out <<
        "  static const int " << conditions[start] << " = " << start << ";\n";
    if (!options["bison_complete"].empty())
    {
      if (!options["bison_locations"].empty())
        *out <<
          "  virtual " << yyltype << " location(void) const\n"
          "  {\n"
          "    " << yyltype << " yylloc;\n"
          "    yylloc.begin.line = static_cast<unsigned int>(matcher().lineno());\n"
          "    yylloc.begin.column = static_cast<unsigned int>(matcher().columno());\n"
          "    yylloc.end.line = static_cast<unsigned int>(matcher().lineno_end());\n"
          "    yylloc.end.column = static_cast<unsigned int>(matcher().columno_end());\n"
          "    return yylloc;\n"
          "  }\n";
      *out <<
        "  virtual " << token_type << " " << lex << "(void)";
    }
    else if (!options["bison_cc"].empty())
    {
      if (!options["bison_locations"].empty())
        *out <<
          "  virtual void yylloc_update(" << yyltype << "& yylloc)\n"
          "  {\n"
          "    yylloc.begin.line = static_cast<unsigned int>(matcher().lineno());\n"
          "    yylloc.begin.column = static_cast<unsigned int>(matcher().columno());\n"
          "    yylloc.end.line = static_cast<unsigned int>(matcher().lineno_end());\n"
          "    yylloc.end.column = static_cast<unsigned int>(matcher().columno_end());\n"
          "  }\n"
          "  virtual " << token_type << " " << lex << "(" << yystype << " *lvalp, " << yyltype << " *llocp)\n"
          "  {\n"
          "    return " << lex << "(*lvalp, *llocp);\n"
          "  }\n"
          "  virtual " << token_type << " " << lex << "(" << yystype << "& yylval, " << yyltype << "& yylloc)";
      else
        *out <<
          "  virtual " << token_type << " " << lex << "(" << yystype << " *lvalp)\n"
          "  {\n"
          "    return " << lex << "(*lvalp);\n"
          "  }\n"
          "  virtual " << token_type << " " << lex << "(" << yystype << "& yylval)";
    }
    else if (!options["bison_locations"].empty())
    {
      *out <<
        "  virtual void yylloc_update(" << yyltype << "& yylloc)\n"
        "  {\n"
        "    yylloc.first_line = static_cast<unsigned int>(matcher().lineno());\n"
        "    yylloc.first_column = static_cast<unsigned int>(matcher().columno());\n"
        "    yylloc.last_line = static_cast<unsigned int>(matcher().lineno_end());\n"
        "    yylloc.last_column = static_cast<unsigned int>(matcher().columno_end());\n"
        "  }\n"
        "  virtual " << token_type << " " << lex << "(" << yystype << "& yylval, " << yyltype << "& yylloc)";
    }
    else if (!options["bison_bridge"].empty())
    {
      *out <<
        "  virtual " << token_type << " " << lex << "(" << yystype << "& yylval)";
    }
    else
    {
      *out <<
        "  virtual " << token_type << " " << lex << "()";
    }
    if (options["class"].empty())
      *out << ";\n";
    else
      *out << " = 0;\n";
  }
  if (options["bison_cc"].empty() && options["bison_bridge"].empty() && options["bison_locations"].empty())
    *out <<
      "  " << token_type << " " << lex << "(\n"
      "      const reflex::Input& input,\n"
      "      std::ostream        *os = NULL)\n"
      "  {\n"
      "    in(input);\n"
      "    if (os)\n"
      "      out(*os);\n"
      "    return " << lex << "();\n"
      "  }\n";
  write_perf_report();
  *out <<
    "};" << std::endl;
  if (!options["namespace"].empty())
  {
    *out << std::endl;
    write_namespace_close();
  }
}

/// Write %%top{ %} code to lex.yy.cpp
void Reflex::write_section_top()
{
  if (!section_top.empty())
  {
    write_banner("SECTION 1: %top{ user code %}");
    write_code(section_top);
  }
}

/// Write %%class{ %} code to lex.yy.cpp
void Reflex::write_section_class()
{
  if (!section_class.empty())
    write_code(section_class);
}

/// Write %%init{ %} code to lex.yy.cpp
void Reflex::write_section_init()
{
  *out << "  {\n";
  if (!section_init.empty())
    write_code(section_init);
  if (!options["debug"].empty())
    *out << "    set_debug(" << options["debug"] << ");\n";
  if (!options["perf_report"].empty())
    *out << "    set_perf_report();\n";
  *out << "  }" << std::endl;
}

/// Write perf_report code to lex.yy.cpp
void Reflex::write_perf_report()
{
  if (!options["perf_report"].empty())
  {
    *out <<
      "  void perf_report()\n"
      "  {\n"
      "    if (perf_report_time_pointer != NULL)\n"
      "      *perf_report_time_pointer += reflex::timer_elapsed(perf_report_timer);\n"
      "    std::cerr << \"reflex " REFLEX_VERSION " " << escape_bs(infile) << " performance report:\\n\";\n";
    for (Start start = 0; start < conditions.size(); ++start)
    {
      *out <<
        "    std::cerr << \"  " << conditions[start] << " rules matched:\\n\"";
      size_t report = 0;
      for (Rules::const_iterator rule = rules[start].begin(); rule != rules[start].end(); ++rule)
      {
        if (rule->regex != "<<EOF>>" && rule->code.line != "|")
        {
          *out <<
            "\n      \"    rule at line " << rule->code.lineno << " accepted \" << perf_report_" << conditions[start] << "_rule[" << report << "] << \" times matching \" << perf_report_" << conditions[start] << "_size[" << report << "] << \" bytes total in \" << perf_report_" << conditions[start] << "_time[" << report << "] << \" ms\\n\"";
          ++report;
        }
      }
      if (options["nodefault"].empty())
        *out <<
          "\n      \"    default rule accepted \" << perf_report_" << conditions[start] << "_default << \" times\\n\"";
      *out <<
        ";\n";
    }
    *out <<
      "    std::cerr << \"  WARNING: execution time measurements are relative:\\n  - includes caller's execution time between matches when " << options["lex"] << "() returns\\n  - perf-report instrumentation adds overhead that increases execution times\\n\" << std::endl;\n"
      "    set_perf_report();\n"
      "  }\n";
    *out <<
      "  void set_perf_report()\n"
      "  {\n";
    for (Start start = 0; start < conditions.size(); ++start)
    {
      size_t report = 0;
      for (Rules::const_iterator rule = rules[start].begin(); rule != rules[start].end(); ++rule)
      {
        if (rule->regex != "<<EOF>>" && rule->code.line != "|")
        {
          *out <<
            "    perf_report_" << conditions[start] << "_rule[" << report << "] = 0;\n"
            "    perf_report_" << conditions[start] << "_size[" << report << "] = 0;\n"
            "    perf_report_" << conditions[start] << "_time[" << report << "] = 0;\n";
          ++report;
        }
      }
      if (options["nodefault"].empty())
        *out <<
          "    perf_report_" << conditions[start] << "_default = 0;\n";
    }
    *out <<
      "    perf_report_time_pointer = NULL;\n"
      "    reflex::timer_start(perf_report_timer);\n"
      "  }\n"
      " protected:\n";
    for (Start start = 0; start < conditions.size(); ++start)
    {
      size_t report = 0;
      for (Rules::const_iterator rule = rules[start].begin(); rule != rules[start].end(); ++rule)
        if (rule->regex != "<<EOF>>" && rule->code.line != "|")
          ++report;
      *out <<
        "  size_t perf_report_" << conditions[start] << "_rule[" << report << "];\n"
        "  size_t perf_report_" << conditions[start] << "_size[" << report << "];\n"
        "  float  perf_report_" << conditions[start] << "_time[" << report << "];\n";
      if (options["nodefault"].empty())
        *out <<
          "  size_t perf_report_" << conditions[start] << "_default;\n";
    }
    *out <<
      "  float *perf_report_time_pointer;\n"
      "  reflex::timer_type perf_report_timer;\n";
  }
}

/// Write section 1 user-defined code to lex.yy.cpp
void Reflex::write_section_1()
{
  if (!section_1.empty())
  {
    write_banner("SECTION 1: %{ user code %}");
    write_code(section_1);
  }
}

/// Write section 3 user-defined code to lex.yy.cpp
void Reflex::write_section_3()
{
  if (!section_3.empty())
  {
    write_banner("SECTION 3: user code");
    write_code(section_3);
  }
}

/// Write lines of code to lex.yy.cpp annotated with #line source info
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
        *out << " \"" << escape_bs(code->file) << "\"";
      *out << std::endl;
    }
    *out << code->line << std::endl;
    this_lineno = code->lineno + 1;
  }
}

/// Write a line(s) of code to lex.yy.cpp annotated with #line source info
void Reflex::write_code(const Code& code)
{
  if (options["noline"].empty())
  {
    *out << "#line " << code.lineno;
    if (!infile.empty())
      *out << " \"" << escape_bs(infile) << "\"";
    *out << std::endl;
  }
  if (!code.line.empty())
    *out << code.line << std::endl;
}

/// Write lexer code and lex() method code
void Reflex::write_lexer()
{
  if (!out->good())
    return;
  std::string lex = options["lex"];
  std::string token_type = options["token_type"].empty() ? "int" : options["token_type"];
  std::string token_eof = options["token_eof"].empty() ? token_type + "()" : options["token_eof"];
  std::string yyltype = options["YYLTYPE"].empty() ? "YYLTYPE" : options["YYLTYPE"];
  std::string yystype = options["YYSTYPE"].empty() ? "YYSTYPE" : options["YYSTYPE"];
  if (!options["bison_cc"].empty() || !options["bison_complete"].empty())
  {
    write_banner("BISON C++");
  }
  else if (!options["reentrant"].empty() || !options["bison_bridge"].empty())
  {
    if (!options["bison_locations"].empty())
      write_banner("BISON BRIDGE LOCATIONS");
    if (!options["bison_bridge"].empty())
      write_banner("BISON BRIDGE");
    else
      write_banner("REENTRANT");
    *out << "typedef ";
    if (!options["namespace"].empty())
      write_namespace_scope();
    if (!options["yyclass"].empty())
      *out << options["yyclass"];
    else if (!options["class"].empty())
      *out << options["class"];
    else
      *out << options["lexer"];
    *out <<
      " yyscanner_t;\n"
      "\n"
      "typedef void *yyscan_t;\n"
      "\n"
      "#ifndef YY_EXTERN_C\n"
      "#define YY_EXTERN_C\n"
      "#endif\n"
      "\n";
    if (!options["bison_locations"].empty())
      *out <<
        "YY_EXTERN_C " << token_type << " yylex(" << yystype << " *lvalp, " << yyltype << " *llocp, yyscan_t scanner)\n"
        "{\n"
        "  return static_cast<yyscanner_t*>(scanner)->" << lex << "(*lvalp, *llocp);\n"
        "}\n"
        "\n";
    else if (!options["bison_bridge"].empty())
      *out <<
        "YY_EXTERN_C " << token_type << " yylex(" << yystype << " *lvalp, yyscan_t scanner)\n"
        "{\n"
        "  return static_cast<yyscanner_t*>(scanner)->" << lex << "(*lvalp);\n"
        "}\n"
        "\n";
    else
      *out <<
        "YY_EXTERN_C " << token_type << " yylex(yyscan_t scanner)\n"
        "{\n"
        "  return static_cast<yyscanner_t*>(scanner)->" << lex << "();\n"
        "}\n"
        "\n";
    *out <<
      "YY_EXTERN_C void yylex_init(yyscan_t *scanner)\n"
      "{\n"
      "  *scanner = static_cast<yyscan_t>(new yyscanner_t);\n"
      "}\n"
      "\n";
    if (!options["flex"].empty())
      *out <<
        "YY_EXTERN_C void yylex_init_extra(" << (options["extra_type"].empty() ? "YY_EXTRA_TYPE" : options["extra_type"].c_str()) << " extra, yyscan_t *scanner)\n"
        "{\n"
        "  *scanner = static_cast<yyscan_t>(new yyscanner_t);\n"
        "  yyset_extra(extra, *scanner);\n"
        "}\n"
        "\n";
    *out <<
      "YY_EXTERN_C void yylex_destroy(yyscan_t scanner)\n"
      "{\n"
      "  delete static_cast<yyscanner_t*>(scanner);\n"
      "}\n";
  }
  else if (!options["bison_locations"].empty())
  {
    write_banner("BISON LOCATIONS");
    if (!options["namespace"].empty())
      write_namespace_scope();
    if (!options["yyclass"].empty())
      *out << options["yyclass"];
    else if (!options["class"].empty())
      *out << options["class"];
    else
      *out << options["lexer"];
    *out <<
      " YY_SCANNER;\n"
      "\n"
      "#ifndef YY_EXTERN_C\n"
      "#define YY_EXTERN_C\n"
      "#endif\n"
      "\n"
      "YY_EXTERN_C " << token_type << " yylex(" << yystype << " *lvalp, " << yyltype << " *llocp)\n"
      "{\n"
      "  return YY_SCANNER." << lex << "(*lvalp, *llocp);\n"
      "}\n"
      "\n";
  }
  else if (!options["bison"].empty())
  {
    write_banner("BISON");
    if (!options["namespace"].empty())
      write_namespace_scope();
    if (!options["yyclass"].empty())
      *out << options["yyclass"];
    else if (!options["class"].empty())
      *out << options["class"];
    else
      *out << options["lexer"];
    *out <<
      " YY_SCANNER;\n"
      "\n"
      "#ifndef YY_EXTERN_C\n"
      "#define YY_EXTERN_C\n"
      "#endif\n"
      "\n";
    if (!options["flex"].empty())
      *out <<
        "char *" << options["prefix"] << "text;\n"
        "yy_size_t " << options["prefix"] << "leng;\n"
        "int " << options["prefix"] << "lineno;\n"
        "\n"
        "YY_EXTERN_C " << token_type << " " << options["prefix"] << "lex(void)\n"
        "{\n"
        "  " << token_type << " ret = YY_SCANNER." << lex << "();\n"
        "  " << options["prefix"] << "text = const_cast<char*>(YY_SCANNER.YYText());\n"
        "  " << options["prefix"] << "leng = static_cast<yy_size_t>(YY_SCANNER.YYLeng());\n"
        "  " << options["prefix"] << "lineno = static_cast<int>(YY_SCANNER.lineno());\n"
        "  return ret;\n"
        "}\n"
        "\n"
        "#define " << options["prefix"] << "text const_cast<char*>(YY_SCANNER.YYText())\n"
        "#define " << options["prefix"] << "leng static_cast<yy_size_t>(YY_SCANNER.YYLeng())\n"
        "#define " << options["prefix"] << "lineno static_cast<int>(YY_SCANNER.lineno())\n";
    else
      *out <<
        "YY_EXTERN_C " << token_type << " yylex(void)\n"
        "{\n"
        "  return YY_SCANNER." << lex << "();\n"
        "}\n";
  }
  write_banner("SECTION 2: rules");
  if (options["matcher"].empty() && !options["fast"].empty())
  {
    for (Start start = 0; start < conditions.size(); ++start)
    {
      if (!options["namespace"].empty())
        write_namespace_open();
      *out << "extern void reflex_code_" << conditions[start] << "(reflex::Matcher&);\n";
      if (!options["find"].empty())
        *out << "extern const reflex::Pattern::Pred reflex_pred_" << conditions[start] << "[];\n";
      if (!options["namespace"].empty())
        write_namespace_close();
    }
    *out << std::endl;
  }
  else if (options["matcher"].empty() && !options["full"].empty())
  {
    for (Start start = 0; start < conditions.size(); ++start)
    {
      if (!options["namespace"].empty())
        write_namespace_open();
      *out << "extern const reflex::Pattern::Opcode reflex_code_" << conditions[start] << "[];\n";
      if (!options["find"].empty())
        *out << "extern const reflex::Pattern::Pred reflex_pred_" << conditions[start] << "[];\n";
      if (!options["namespace"].empty())
        write_namespace_close();
    }
    *out << std::endl;
  }
  *out << token_type << " ";
  if (!options["namespace"].empty())
    write_namespace_scope();
  if (!options["yyclass"].empty())
    *out << options["yyclass"];
  else if (!options["class"].empty())
    *out << options["class"];
  else
    *out << options["lexer"];
  if (!options["bison_complete"].empty())
    *out << "::" << lex << "()\n{\n";
  else if (!options["bison_locations"].empty())
    *out << "::" << lex << "(" << yystype << "& yylval, " << yyltype << "& yylloc)\n{\n";
  else if (!options["bison_cc"].empty() || !options["bison_bridge"].empty())
    *out << "::" << lex << "(" << yystype << "& yylval)\n{\n";
  else
    *out << "::" << lex << "()\n{\n";
  for (Start start = 0; start < conditions.size(); ++start)
  {
    if (options["matcher"].empty())
    {
      if (!options["full"].empty() || !options["fast"].empty())
      {
        *out << "  static const reflex::Pattern PATTERN_" << conditions[start] << "(reflex_code_" << conditions[start];
        if (!options["find"].empty())
          *out << ", reflex_pred_" << conditions[start];
        *out << ");\n";
      }
      else
      {
        write_regex(&conditions[start], patterns[start]);
        *out << "  static const reflex::Pattern PATTERN_" << conditions[start] << "(REGEX_" << conditions[start] << ");\n";
      }
    }
    else
    {
      write_regex(&conditions[start], patterns[start]);
      *out << "  static const " << library->pattern << " PATTERN_" << conditions[start] << "(REGEX_" << conditions[start] << ");\n";
    }
  }
  *out <<
    "  if (!has_matcher())\n"
    "  {\n";
  if (!options["tabs"].empty())
    *out <<
      "    matcher(new Matcher(PATTERN_" << conditions[0] << ", " << (options["nostdinit"].empty() ? "stdinit()" : "nostdinit()") << ", this, \"T=" << options["tabs"] << "\"));\n";
  else
    *out <<
      "    matcher(new Matcher(PATTERN_" << conditions[0] << ", " << (options["nostdinit"].empty() ? "stdinit()" : "nostdinit()") << ", this));\n";
#ifdef WITH_BOOST_PARTIAL_MATCH_BUG
  if (options["matcher"] == "boost" || options["matcher"] == "boost-perl")
    *out <<
      "    if (!matcher().buffer()) // work around Boost.Regex match_partial bug\n"
      "      return " << token_type << "(); // could not buffer: terminate\n";
  else
#endif
  if (!options["interactive"].empty() || !options["always_interactive"].empty())
    *out <<
      "    matcher().interactive();\n";
  else if (!options["batch"].empty())
    *out <<
      "    matcher().buffer();\n";
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
  if (!options["perf_report"].empty())
    *out <<
      "    if (perf_report_time_pointer != NULL)\n"
      "      *perf_report_time_pointer += reflex::timer_elapsed(perf_report_timer);\n";
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
    if (!options["find"].empty())
    {
      if (!options["bison_locations"].empty() && options["bison_complete"].empty())
        *out <<
          "        matcher().find();\n"
          "        yylloc_update(yylloc);\n"
          "        switch (matcher().accept())\n";
      else
        *out <<
          "        switch (matcher().find())\n";
      *out <<
        "        {\n"
        "          case 0:\n"
        "            return " << token_eof << ";\n";
    }
    else
    {
      if (!options["bison_locations"].empty() && options["bison_complete"].empty())
        *out <<
          "        matcher().scan();\n"
          "        yylloc_update(yylloc);\n"
          "        switch (matcher().accept())\n";
      else
        *out <<
          "        switch (matcher().scan())\n";
      *out <<
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
              "              if (debug()) std::cerr << \"--" <<
              SGR("\\033[1;35m") << "EOF rule at line " << rule->code.lineno << SGR("\\033[0m") <<
              " (start condition \" << start() << \")\\n\";\n";
          write_code(rule->code);
          has_eof = true;
          break;
        }
      }
      if (!has_eof && !options["debug"].empty())
        *out <<
          "              if (debug()) std::cerr << \"--" <<
          SGR("\\033[1;35m") << "EOF" << SGR("\\033[0m") <<
          " (start condition \" << start() << \")\\n\";\n";
      if (!options["perf_report"].empty())
        *out << "              perf_report();\n";
      if (!has_eof)
      {
        if (!options["flex"].empty())
          *out <<
            "              yyterminate();\n";
        else
          *out <<
            "              return " << token_eof << ";\n";
      }
      *out <<
        "            }\n"
        "            else\n"
        "            {\n";
      if (!options["debug"].empty())
        *out <<
          "              if (debug()) std::cerr << \"--" <<
          SGR("\\033[1;31m") << "accepting default rule" << SGR("\\033[0m") <<
          "\\n\";\n";
      if (!options["nodefault"].empty())
      {
        if (!options["flex"].empty())
          *out <<
            "              LexerError(\"scanner jammed\");\n"
            "              yyterminate();\n";
        else if (!options["debug"].empty())
          *out <<
            "              char ch = matcher().input();\n"
            "              if (debug()) std::cerr << \"--" <<
            SGR("\\033[1;31m") << "suppressing default rule for" << SGR("\\033[0m") <<
            " (\\\"\" << ch << \"\\\")\\n\";\n";
        else if (!options["exception"].empty())
          *out <<
            "              throw " << options["exception"] << ";\n";
        else
          *out <<
            "              lexer_error(\"scanner jammed\");\n"
            "              return " << token_type << "();\n";
      }
      else
      {
        if (!options["perf_report"].empty())
          *out <<
            "              ++perf_report_" << conditions[start] << "_default;\n";
        if (!options["exception"].empty())
          *out <<
            "              throw " << options["exception"] << ";\n";
        else if (!options["flex"].empty())
          *out <<
            "              output(matcher().input());\n";
        else
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
    }
    size_t accept = 1;
    size_t report = 0;
    bool has_code = true;
    for (Rules::const_iterator rule = rules[start].begin(); rule != rules[start].end(); ++rule)
    {
      bool eof_rule = rule->regex == "<<EOF>>";
      if (!eof_rule || !has_code)
      {
        if (!eof_rule)
          *out <<
            "          case " << accept << ": // rule at line " << rule->code.lineno << ": " << rule->regex << std::endl;
        has_code = rule->code.line != "|";
        if (has_code)
        {
          if (!eof_rule)
          {
            if (!options["perf_report"].empty())
            {
              *out <<
                "            ++perf_report_" << conditions[start] << "_rule[" << report << "];\n"
                "            perf_report_" << conditions[start] << "_size[" << report << "] += size();\n"
                "            perf_report_time_pointer = &perf_report_" << conditions[start] << "_time[" << report << "];\n";
              ++report;
            }
          }
          if (!options["debug"].empty())
            *out <<
              "            if (debug()) std::cerr << \"--" <<
              SGR("\\033[1;35m") << "accepting rule at line " << rule->code.lineno << SGR("\\033[0m") <<
              " (\\\"" << SGR("\\033[1m") << "\" << matcher().text() << \"" << SGR("\\033[0m") << "\\\")\\n\";\n";
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
  {
    *out <<
      "      default:\n"
      "        start(0);\n"
      "    }\n";
  }
  *out <<
    "  }\n"
    "}" << std::endl;
}

/// Write main() to lex.yy.cpp
void Reflex::write_main()
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
        write_namespace_scope();
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

/// Write regex string to lex.yy.cpp by escaping \ and ", prevent trigraphs, very long strings are represented by character arrays
void Reflex::write_regex(const std::string *condition, const std::string& regex)
{
  // output a string if start condition == NULL (--regexp-file option) or when the string is not too long
  if (!condition ||
#ifdef OS_WIN
      regex.size() <= 16384
#else
      regex.size() <= 65536
#endif
     )
  {
    if (condition)
      *out << "  static const char *REGEX_" << *condition << " = ";
    *out << "\"";
    int c = '\0';
    for (std::string::const_iterator i = regex.begin(); i != regex.end(); ++i)
    {
      if (*i == '\\' || *i == '"' || (*i == '?' && c == '?'))
        *out << "\\";
      *out << *i;
      c = *i;
    }
    *out << "\"";
  }
  else
  {
    if (condition)
      *out << "  static const char REGEX_" << *condition << "[" << regex.size() + 1 << "] = ";
    *out << "{ ";
    for (std::string::const_iterator i = regex.begin(); i != regex.end(); ++i)
    {
      if (*i == '\\')
        *out << "'\\\\',";
      else if (*i == '\'')
        *out << "'\\'',";
      else if (std::isprint(*i))
        *out << "'" << *i << "', ";
      else
        *out << static_cast<unsigned int>(*i) << ", ";
    }
    *out << "0 }";
  }
  if (condition)
    *out << ";\n";
}

/// Write namespace openings NAME {
void Reflex::write_namespace_open()
{
  const std::string& s = options["namespace"];
  size_t i = 0, j;
  while ((j = s.find("::", i)) != std::string::npos)
  {
    *out << "namespace " << s.substr(i, j-i) << " {" << std::endl;
    i = j + 2;
  }
  *out << "namespace " << s.substr(i) << " {" << std::endl;
}

/// Write namespace closing scope } // NAME
void Reflex::write_namespace_close()
{
  const std::string& s = options["namespace"];
  size_t i = 0, j;
  while ((j = s.find("::", i)) != std::string::npos)
  {
    *out << "} // namespace " << s.substr(i, j-i) << std::endl;
    i = j + 2;
  }
  *out << "} // namespace " << s.substr(i) << std::endl;
}

/// Write namespace scope NAME ::
void Reflex::write_namespace_scope()
{
  *out << options["namespace"] << "::";
}

/// Replace all . by :: in namespace name
void Reflex::undot_namespace(std::string& s)
{
  size_t i = 0;
  while ((i = s.find('.', i)) != std::string::npos)
  {
    s.replace(i, 1, "::");
    i += 2;
  }
}

/// Display usage report
void Reflex::stats()
{
  if (!options["verbose"].empty())
  {
    std::cout << "reflex " REFLEX_VERSION " " << infile << " usage report:\n" << "  options used:\n";
    for (StringMap::const_iterator option = options.begin(); option != options.end(); ++option)
      if (!option->second.empty())
        std::cout << "    " << option->first << "=" << option->second << std::endl;
    if (!options["verbose"].empty())
      std::cout << "  inclusive (%s) and exclusive (%x) start conditions (with construction time):\n";
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
      std::cout << std::endl;
    }
  }
  else
  {
    for (Start start = 0; start < conditions.size(); ++start)
    {
      std::string option = "r";
      option.append(";n=").append(conditions[start]);
      if (!options["namespace"].empty())
        option.append(";z=").append(options["namespace"]);
      if (options["graphs_file"] == "true")
        option.append(";f=reflex.").append(conditions[start]).append(".gv");
      else if (!options["graphs_file"].empty())
        option.append(";f=").append(start > 0 ? "+" : "").append(file_ext(options["graphs_file"], "gv"));
      if (!options["fast"].empty())
        option.append(";o");
      if (!options["find"].empty())
        option.append(";p");
      if (options["tables_file"] == "true")
        option.append(";f=reflex.").append(conditions[start]).append(".cpp");
      else if (!options["tables_file"].empty())
        option.append(";f=").append(start > 0 ? "+" : "").append(file_ext(options["tables_file"], "cpp"));
      if ((!options["full"].empty() || !options["fast"].empty()) && options["tables_file"].empty() && options["stdout"].empty())
        option.append(";f=+").append(escape_bs(options["outfile"]));      
      try
      {
        reflex::Pattern pattern(patterns[start], option);
        reflex::Pattern::Index accept = 1;
        for (size_t rule = 0; rule < rules[start].size(); ++rule)
          if (rules[start][rule].regex != "<<EOF>>")
            if (!pattern.reachable(accept++))
              warning("rule cannot be matched because a previous rule subsumes it, perhaps try to move this rule up?", "", rules[start][rule].code.lineno);
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
          std::cout << conditions[start] << ":\n"
            << std::setw(10) << n << " rules (" << pattern.parse_time() << " ms)";
          if (n < rules[start].size())
            std::cout << " + <<EOF>> rule";
          std::cout
            << "\n"
            << std::setw(10) << pattern.nodes() << " nodes (" << pattern.nodes_time() << " ms)\n"
            << std::setw(10) << pattern.edges() << " edges (" << pattern.edges_time() << " ms)\n"
            << std::setw(10) << pattern.words() << " words (" << pattern.words_time() << " ms)\n";
        }
      }
      catch (reflex::regex_error& e)
      {
        abort("malformed regular expression\n", e.what());
      }
    }
    if (!options["verbose"].empty())
      std::cout << std::endl;
  }
}

