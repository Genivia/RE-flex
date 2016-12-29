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
@copyright (c) 2015-2016, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_H
#define REFLEX_H

#include "pattern.h"
#include "utf8.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <stack>
#include <vector>

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__BORLANDC__)
# define OS_WIN
# define PLATFORM "WIN"
#else
# define PLATFORM "(unknown arch)"
#endif

#define REFLEX_VERSION "0.9.9"

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
    std::string line;
    std::string file;
    size_t      lineno;
  };

  typedef std::vector<Code> Codes;

  struct Rule {
    Rule(
        const std::string& regex,
        const Code&        code)
      :
        regex(regex),
        code(code)
    { }
    std::string regex;
    Code        code;
  };

  typedef std::vector<Rule>                 Rules;
  typedef std::vector<std::string>          Strings;
  typedef std::map<std::string,std::string> Map;
  typedef size_t                            Start;
  typedef std::set<Start>                   Starts;
  typedef std::map<Start,Codes>             CodesMap;
  typedef std::map<Start,Rules>             RulesMap;

 private:
  void          init(int argc, char **argv);
  void          parse(void);
  void          parse_section_1(void);
  void          parse_section_2(void);
  void          parse_section_3(void);
  void          include(const std::string& filename);
  void          write(void);
  void          write_banner(const char *title);
  void          write_prelude(void);
  void          write_class(void);
  void          write_section_top();
  void          write_section_class();
  void          write_section_init();
  void          write_section_1();
  void          write_section_3();
  void          write_code(const Codes& codes);
  void          write_code(const Code& code);
  void          write_lexer(void);
  void          write_main(void);
  void          write_stats(void);
  void          write_regex(const std::string& regex);
  bool          getline(void);
  bool          skipcomment(size_t& pos);
  bool          as(size_t& pos, const char *s);
  bool          ws(size_t& pos);
  bool          eq(size_t& pos);
  bool          nl(size_t& pos);
  bool          iscode(void);
  bool          istopcode(void);
  bool          isclasscode(void);
  bool          isinitcode(void);
  std::string   getname(size_t& pos);
  std::string   getstring(size_t& pos);
  std::string   getregex(size_t& pos);
  Starts        getstarts(size_t& pos);
  std::string   getcode(size_t& pos);
  void          error(const char *message, const char *arg = NULL, size_t at_lineno = 0);
  void          warning(const char *message, const char *arg = NULL, size_t at_lineno = 0);

 protected:
  Map           options;       ///< maps option name (from the options_table) to its option value
  Map           scripts;       ///< maps Unicode script names to patterns
  Strings       conditions;    ///< "INITIAL" start condition etc. defined with %x name
  Strings       patterns;      ///< regex patterns for each start condition
  Starts        inclusive;     ///< inclusive start conditions
  Map           definitions;   ///< map of {name} to regex
  RulesMap      rules;         ///< <Start_i>regex_j action for Start i Rule j
  Codes         section_top;   ///< %top{ user code %} in section 1 container
  Codes         section_class; ///< %class{ class code %} in section 1 container
  Codes         section_init;  ///< %init{ init code %} in section 1 container
  Codes         section_1;     ///< %{ user code %} in section 1 container
  CodesMap      section_2;     ///< lexer user code in section 2 container
  Codes         section_3;     ///< main user code in section 3 container
  std::string   infile;        ///< input file name
  std::istream *in;            ///< input stream with lex spec
  std::ostream *out;           ///< output stream
  std::string   line;          ///< current line read from input
  size_t        lineno;        ///< current line number at input
  size_t        linelen;       ///< current line length
};

#endif
