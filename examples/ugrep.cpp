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
@file      ugrep.cpp
@brief     Unicode-aware grep utility
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2019, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt

Features:

  - Patterns are ERE POSIX syntax compliant, extended with RE/flex pattern syntax.
  - Unicode support for \p{} character categories, bracket list classes, etc.
  - File encoding support for UTF-8/16/32, EBCDIC, and many other code pages.

Examples:

  # find all capitalized Unicode words in places.txt
  ugrep '\p{Upper}\p{Lower}*' places.txt

  # find all capitalized Unicode words in places.txt and color highlight them
  ugrep --color=auto '\p{Upper}\p{Lower}*' places.txt

  # find the names Gödel (or Goedel), Escher, and Bach in GEB.txt and wiki.txt
  ugrep 'G(ö|oe)del|Escher|Bach' GEB.txt wiki.txt

  # count the number of occurrences of the names Gödel (or Goedel), Escher, and Bach in GEB.txt and wiki.txt
  ugrep -c 'G(ö|oe)del|Escher|Bach' GEB.txt wiki.txt

  # check if some.txt file contains any non-ASCII (i.e. Unicode) characters
  ugrep -q '[^[:ascii:]]' some.txt && echo "some.txt contains Unicode"

  # find word-anchored 'lorem' in UTF-16 formatted file utf16lorem.txt that contains a UTF-16 BOM
  ugrep -o '\<[Ll]orem\>' utf16lorem.txt

  # find word-anchored 'lorem' in UTF-16 formatted file utf16lorem.txt that does not contain a UTF-16 BOM
  ugrep -o --file-format=UTF-16 '\<[Ll]orem\>' utf16lorem.txt

Compile:

  c++ -std=c++11 -o ugrep ugrep.cpp -lreflex

*/

#include <reflex/matcher.h>

// ugrep version
#define VERSION "1.0.0"

// check if we are on a windows OS
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__BORLANDC__)
# define OS_WIN
#endif

#ifdef OS_WIN
#define isatty(fildes) ((fildes) == 1)
#else
#include <unistd.h>
#endif

// ugrep exit codes
#define EXIT_OK    0 // One or more lines were selected
#define EXIT_FAIL  1 // No lines were selected
#define EXIT_ERROR 2 // An error occurred

// GREP_COLOR environment variable
const char *grep_color = NULL;

// ugrep command-line options
bool flag_filename           = false;
bool flag_no_filename        = false;
bool flag_line_buffered      = false;
bool flag_count              = false;
bool flag_only_matching      = false;
bool flag_quiet              = false;
const char *flag_color       = NULL;
const char *flag_file_format = NULL;

// function protos
bool ugrep(reflex::Pattern& pattern, FILE *file, reflex::Input::file_encoding_type encoding, const char *infile = NULL);
void help(const char *message = NULL, const char *arg = NULL);
void version();

// table of file formats for ugrep option --file-format
const struct { const char *format; reflex::Input::file_encoding_type encoding; } format_table[] = {
  { "binary",     reflex::Input::file_encoding::plain   },
  { "ISO-8859-1", reflex::Input::file_encoding::latin   },
  { "ASCII",      reflex::Input::file_encoding::utf8    },
  { "EBCDIC",     reflex::Input::file_encoding::ebcdic  },
  { "UTF-8",      reflex::Input::file_encoding::utf8    },
  { "UTF-16",     reflex::Input::file_encoding::utf16be },
  { "UTF-16BE",   reflex::Input::file_encoding::utf16be },
  { "UTF-16LE",   reflex::Input::file_encoding::utf16le },
  { "UTF-32",     reflex::Input::file_encoding::utf32be },
  { "UTF-32BE",   reflex::Input::file_encoding::utf32be },
  { "UTF-32LE",   reflex::Input::file_encoding::utf32le },
  { "CP-437",     reflex::Input::file_encoding::cp437   },
  { "CP-850",     reflex::Input::file_encoding::cp850   },
  { "CP-855",     reflex::Input::file_encoding::cp850   },
  { "CP-1250",    reflex::Input::file_encoding::cp1250  },
  { "CP-1251",    reflex::Input::file_encoding::cp1251  },
  { "CP-1252",    reflex::Input::file_encoding::cp1252  },
  { "CP-1253",    reflex::Input::file_encoding::cp1253  },
  { "CP-1254",    reflex::Input::file_encoding::cp1254  },
  { "CP-1255",    reflex::Input::file_encoding::cp1255  },
  { "CP-1256",    reflex::Input::file_encoding::cp1256  },
  { "CP-1257",    reflex::Input::file_encoding::cp1257  },
  { "CP-1258",    reflex::Input::file_encoding::cp1258  },
  { NULL, 0 }
};

// ugrep main()
int main(int argc, char **argv)
{
  std::string regex;
  std::vector<const char*> infiles;

  bool color_term = false;

#ifndef OS_WIN
  // check weather we have a color terminal
  const char *term = getenv("TERM");
  color_term = term && (strstr(term, "ansi") || strstr(term, "xterm") || strstr(term, "color"));
  grep_color = getenv("GREP_COLOR");
#endif
 
  // parse ugrep command-line options and arguments
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

      // parse a ugrep command-line option
      while (is_grouped && *++arg)
      {
        switch (*arg)
        {
          case '-':
            ++arg;
            if (strcmp(arg, "color") == 0 || strcmp(arg, "colour") == 0)
              flag_color = "auto";
            else if (strncmp(arg, "color=", 6) == 0)
              flag_color = arg + 6;
            else if (strncmp(arg, "colour=", 7) == 0)
              flag_color = arg + 7;
            else if (strcmp(arg, "count") == 0)
              flag_count = true;
            else if (strncmp(arg, "file-format=", 12) == 0)
              flag_file_format = arg + 12;
            else if (strcmp(arg, "help") == 0)
              help();
            else if (strcmp(arg, "no-filename") == 0)
              flag_no_filename = true;
            else if (strcmp(arg, "only-matching") == 0)
              flag_only_matching = true;
            else if (strcmp(arg, "quiet") == 0 || strcmp(arg, "silent") == 0)
              flag_quiet = true;
            else if (strncmp(arg, "regexp=", 7) == 0)
              regex.append(arg + 7).push_back('|');
            else if (strcmp(arg, "version") == 0)
              version();
            else
              help("unknown option --", arg);
            is_grouped = false;
            break;

          case 'c':
            flag_count = true;
            break;

          case 'e':
            ++arg;
            if (*arg)
              regex.append(&arg[*arg == '=']).push_back('|');
            else if (++i < argc)
              regex.append(argv[i]).push_back('|');
            else
              help("missing pattern for option -e");
            is_grouped = false;
            break;

          case 'h':
            flag_no_filename = false;
            break;

          case 'q':
            flag_quiet = true;
            break;

          case 'o':
            flag_only_matching = true;
            break;

          default:
            help("unknown option -", arg);
        }
      }
    }
    else
    {
      // parse a ugrep command-line argument
      if (regex.empty())
        // no regex pattern specified yet, so assign it to the regex string
        regex.assign(arg).push_back('|');
      else
        // otherwise add the file argument to the list of files
        infiles.push_back(arg);
    }
  }

  // if no regex pattern was specified then exit
  if (regex.empty())
    help();

  // remove the ending '|' from the |-concatenated regexes in the regex string
  regex.pop_back();

  // enable line buffering if the input is a TTY and options -c -o -q are not specified
  if (isatty(1) && !flag_count && !flag_only_matching && !flag_quiet)
    flag_line_buffered = true;

  // display file name if more than one input file is specified and option --no-filename is not specified
  if (infiles.size() > 1 && !flag_no_filename)
    flag_filename = true;

  // (re)set grep_color depending on color_term and the ugrep command-line options
  if (!flag_color || strcmp(flag_color, "never") == 0)
  {
    grep_color = NULL;
  }
  else if (strcmp(flag_color, "always") == 0)
  {
    if (!grep_color)
      grep_color = "1";
  }
  else if (strcmp(flag_color, "auto") == 0)
  {
    if (!color_term)
      grep_color = NULL;
    else if (!grep_color)
      grep_color = "1";
  }
  else
  {
    help("unknown --color option");
  }

  // if any match was found in any of the input files then we set found==true
  bool found = false;

  try
  {
    reflex::Input::file_encoding_type encoding = reflex::Input::file_encoding::plain;

    // parse ugrep option --file-format=format
    if (flag_file_format)
    {
      int i;

      // scan the format_table[] for a matching format
      for (i = 0; format_table[i].format != NULL; ++i)
        if (strcmp(flag_file_format, format_table[i].format) == 0)
          break;

      if (format_table[i].format == NULL)
        help("Unknown file format specified");

      // encoding is the file format used by all input files, if no BOM is present
      encoding = format_table[i].encoding;
    }

    reflex::Pattern pattern(reflex::Matcher::convert(regex, reflex::convert_flag::unicode));

    if (infiles.empty())
    {
      // read standard input to find pattern matches
      found |= ugrep(pattern, stdin, encoding);
    }
    else
    {
      // read each file to find pattern matches
      for (auto infile : infiles)
      {
        FILE *file = fopen(infile, "r");

        if (file == NULL)
        {
          perror("Cannot open file for reading");
          exit(EXIT_ERROR);
        }

        found |= ugrep(pattern, file, encoding, infile);

        fclose(file);
      }
    }
  }
  catch (reflex::regex_error& error)
  {
    std::cerr << error.what();
    exit(EXIT_ERROR);
  }

  exit(found ? EXIT_OK : EXIT_FAIL);
}

// Search file, display pattern matches, return true when pattern matched anywhere
bool ugrep(reflex::Pattern& pattern, FILE *file, reflex::Input::file_encoding_type encoding, const char *infile)
{
  bool found = false;

  std::string label, mark, unmark;

  if (flag_filename)
    label.assign(infile).append(":");

  if (grep_color)
  {
    mark.assign("\033[").append(grep_color).append("m");
    unmark.assign("\033[0m");
  }

  // create an input object to read the file (or stdin) using the given file format encoding
  reflex::Input input(file, encoding);

  if (flag_quiet)
  {
    // -q quite mode: report if a single pattern match was found in the input
    found = reflex::Matcher(pattern, input).find();
  }
  else if (flag_count)
  {
    size_t lineno = 0;
    size_t lines = 0;

    // -c count mode: count the number of lines that matched
    for (auto& match : reflex::Matcher(pattern, input).find)
    {
      if (lineno != match.lineno())
      {
        lineno = match.lineno();
        ++lines;
      }
      found = true;
    }

    std::cout << label << lines << std::endl;
  }
  else if (flag_line_buffered)
  {
    std::string line;

    // line-buffered mode: read input line-by-line and display lines that matched the pattern
    while (input)
    {
      int ch;

      // read the next line
      line.clear();
      while ((ch = input.get()) != EOF && ch != '\n')
        line.push_back(ch);

      // search the line for pattern matches
      for (auto& match : reflex::Matcher(pattern, line).find)
      {
        std::cout << label << line.substr(0, match.first()) << mark << match.text() << unmark << line.substr(match.last()) << std::endl;
        found = true;
      }
    }
  }
  else
  {
    // block-buffered mode: echo all pattern matches
    for (auto& match : reflex::Matcher(pattern, input).find)
    {
      std::cout << label << mark << match.text() << unmark << std::endl;
      found = true;
    }
  }

  return found;
}

// Display help information with an optional diagnostic message and exit
void help(const char *message, const char *arg)
{
  if (message)
    std::cout << "ugrep: " << message << (arg != NULL ? arg : "") << std::endl;
  std::cout << "Usage: ugrep [-cehoqV] [--line-buffered] pattern [file ...]\n\
\n\
     -c, --count\n\
             Only a count of selected lines is written to standard output.\n\
     --colour[=when], --color[=when]\n\
             Mark up the matching text with the expression stored in\n\
             GREP_COLOR environment variable.  The possible values of when can\n\
             be `never', `always' or `auto'.\n\
     -e pattern, --regexp=pattern\n\
             Specify a pattern used during the search of the input: an input\n\
             line is selected if it matches any of the specified patterns.\n\
             This option is most useful when multiple -e options are used to\n\
             specify multiple patterns, or when a pattern begins with a dash\n\
             (`-').\n\
     -h, --no-filename\n\
             Never print filename headers (i.e. filenames) with output lines.\n\
     -o, --only-matching\n\
             Prints only the matching part of the lines.\n\
     -q, --quiet, --silent\n\
             Quiet mode: suppress normal output.  ugrep will only search a file\n\
             until a match has been found, making searches potentially less\n\
             expensive.\n\
     --file-format=format\n\
             The input file format:";
  for (int i = 0; format_table[i].format != NULL; ++i)
    std::cout << (i % 8 ? " " : "\n             ") << format_table[i].format;
  std::cout << "\n\
     --line-buffered\n\
             Force output to be line buffered.  By default, output is line\n\
             buffered when standard output is a terminal and block buffered\n\
             otherwise.\n\
     -V, --version\n\
             Display version information and exit.\n\
\n\
     The ugrep utility exits with one of the following values:\n\
\n\
     0     One or more lines were selected.\n\
     1     No lines were selected.\n\
     >1    An error occurred.\n\
" << std::endl;
  exit(EXIT_ERROR);
}

// Display version info
void version()
{
  std::cout << "ugrep " VERSION << std::endl;
  exit(EXIT_OK);
}
