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
@file      reflex.h
@brief     RE/flex scanner generator replacement for Flex/Lex
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2019, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_H
#define REFLEX_H

#include <reflex/convert.h>
#include <reflex/error.h>
#include <reflex/input.h>
#include <reflex/pattern.h>
#include <reflex/utf8.h>
#include <cctype>
#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <set>
#include <stack>
#include <vector>

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__BORLANDC__)
# define OS_WIN
#endif

#if !defined(PLATFORM)
# if defined(OS_WIN)
#  define PLATFORM "WIN"
# else
#  define PLATFORM ""
# endif
#endif

// DO NOT ALTER THIS LINE: the makemake.sh script updates the version
#define REFLEX_VERSION "3.2.12"

/// RE/flex scanner generator class, a variation of the classic "lex" tool to generate scanners.
/**

More info TODO

The Lex specification file extension is `.l`, but also `.lex`, `.ll`, `.l++`,
`.lxx`, and `.lpp` are in use.
*/
class Reflex
{
 public:
  void main(int argc, char **argv);

  /// A library entry to describe regex library properties
  struct Library {
    const char *name;      ///< the unique ID name of the regex library
    const char *file;      ///< the header file to include
    const char *pattern;   ///< the name of the pattern class or C++ type used by the matcher class
    const char *matcher;   ///< the name of the matcher class
    const char *signature; ///< a regex library signature of the form "[decls:]escapes[?+]"
  };

  /// Line of code fragment in lex specifications
  struct Code {
    Code(
        const std::string& line,
        const std::string& file,
        size_t             lineno)
      :
        line(line),
        file(file),
        lineno(lineno)
    { }
    std::string line;   ///< line of code
    std::string file;   ///< source filename
    size_t      lineno; ///< source line number
  };

  /// A regex pattern and action pair that forms a rule
  struct Rule {
    Rule(
        const std::string& pattern,
        const std::string& regex,
        const Code&        code)
      :
        pattern(pattern),
        regex(regex),
        code(code)
    { }
    std::string pattern; ///< the pattern
    std::string regex;   ///< the pattern-converted regex for the selected regex engine
    Code        code;    ///< the action code corresponding to the pattern
  };

  typedef std::map<std::string,Library>     LibraryMap; ///< Dictionary of regex libraries
  typedef std::vector<Code>                 Codes;      ///< Collection of ordered lines of code
  typedef std::vector<Rule>                 Rules;      ///< Collection of ordered rules
  typedef std::vector<std::string>          Strings;    ///< Collection of ordered strings
  typedef std::map<std::string,std::string> StringMap;  ///< Dictionary (std::string)
  typedef std::map<std::string,const char*> Dictionary; ///< Dictionary (const char*)
  typedef size_t                            Start;      ///< Start condition state type
  typedef std::set<Start>                   Starts;     ///< Set of start conditions
  typedef std::map<Start,Codes>             CodesMap;   ///< Map of start conditions to lines of code
  typedef std::map<Start,Rules>             RulesMap;   ///< Map of start conditions to rules

 private:
  void        init(int argc, char **argv);
  void        version();
  void        help(const char *message = NULL, const char *arg = NULL);
  void        set_library();
  void        parse();
  void        parse_section_1();
  void        parse_section_2();
  void        parse_section_3();
  void        include(const std::string& filename);
  void        write();
  void        write_banner(const char *title);
  void        write_prelude();
  void        write_defines();
  void        write_class();
  void        write_section_top();
  void        write_section_class();
  void        write_section_init();
  void        write_section_begin();
  void        write_perf_report();
  void        write_section_1();
  void        write_section_3();
  void        write_code(const Codes& codes);
  void        write_code(const Code& code);
  void        write_lexer();
  void        write_main();
  void        write_regex(const std::string *condition, const std::string& regex);    
  void        write_namespace_open();
  void        write_namespace_close();
  void        write_namespace_scope();
  void        undot_namespace(std::string& s);
  void        stats();
  bool        get_line();
  bool        skip_comment(size_t& pos);
  bool        is(const char *s);
  bool        ins(const char *s);
  bool        br(size_t pos, const char *s = NULL);
  bool        as(size_t& pos, const char *s);
  bool        ws(size_t& pos);
  bool        eq(size_t& pos);
  bool        nl(size_t& pos);
  bool        is_code();
  bool        is_top_code();
  bool        is_class_code();
  bool        is_init_code();
  bool        is_begin_code();
  std::string get_name(size_t& pos);
  std::string get_option(size_t& pos);
  std::string get_start(size_t& pos);
  std::string get_string(size_t& pos);
  bool        get_pattern(size_t& pos, std::string& pattern, std::string& regex);
  std::string get_namespace(size_t& pos);
  std::string get_code(size_t& pos);
  std::string escape_bs(const std::string& s);
  std::string upper_name(const std::string& s);
  std::string param_args(const std::string& s);
  bool        get_starts(size_t& pos, Starts& starts);
  void        abort(const char *message, const char *arg = NULL);
  void        error(const char *message, const char *arg = NULL, size_t at_lineno = 0);
  void        warning(const char *message, const char *arg = NULL, size_t at_lineno = 0);
  const char *SGR(const char *code) { return color_term ? code : ""; }

 protected:
  StringMap             options;       ///< maps option name (from the options_table) to its option value
  LibraryMap            libraries;     ///< maps regex library name ("reflex", "boost", etc) to library info
  Library              *library;       ///< the regex library selected
  Strings               conditions;    ///< "INITIAL" start condition etc. defined with %x name
  Strings               patterns;      ///< regex patterns for each start condition
  Starts                inclusive;     ///< inclusive start conditions
  StringMap             definitions;   ///< map of {name} to regex
  RulesMap              rules;         ///< <Start_i>regex_j action for Start i Rule j
  Codes                 section_top;   ///< %top{ user code %} in section 1 container
  Codes                 section_class; ///< %class{ class code %} in section 1 container
  Codes                 section_init;  ///< %init{ init code %} in section 1 container
  Codes                 section_begin; ///< %begin{ begin scanning code %} in section 1 container
  Codes                 section_1;     ///< %{ user code %} in section 1 container
  CodesMap              section_2;     ///< lexer user code in section 2 container
  Codes                 section_3;     ///< main user code in section 3 container
  std::string           infile;        ///< input file name
  reflex::BufferedInput in;            ///< input lex spec
  std::ostream         *out;           ///< output stream
  std::string           line;          ///< current line read from input
  size_t                lineno;        ///< current line number at input
  size_t                linelen;       ///< current line length
  bool                  color_term;    ///< terminal supports colors
};

#endif
