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
@file      letter_case.l
@brief     RE/Flex specification to convert Unicode UnicodeData.txt to C++
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2025, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

%{

#include <iostream>
#include <map>
#include <cwctype>
#include <clocale>

typedef std::map<std::string,int> Code;
typedef std::map<int,std::string> Letter;

Code code;
Letter letter;

%}

%o matcher=reflex
%o nodefault

hex			[[:xdigit:]]+
lab			[-A-Z ]+
sml			{lab}"SMALL"{lab}
cap			{lab}"CAPITAL"{lab}

%%

^{hex}+;{sml};Ll;	|
^{hex}+;{cap};Lu;	int x;
			char *s;
			matcher().less(size() - 4); // strip trailing ;Ll; and ;Lu;
			x = strtol(text(), &s, 16); // U+xxxx hex code
			++s;
			code[s] = x;
			letter[x] = s;

.|\n			// skip remainder

%%

int main()
{
  Lexer().lex();

  std::cout <<
    "// Converted from http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt by letter_case.l\n"
    "#include <reflex/unicode.h>\n"
    "#include <cstdlib>\n"
    "\n"
    "static int cmpint2(const void *p, const void *q)\n"
    "{\n"
    "  int a = *static_cast<const int*>(p);\n"
    "  int b = *static_cast<const int*>(q);\n"
    "  return a < b ? -1 : a > b ? 1 : 0;\n"
    "}\n"
    "\n"
    "int reflex::Unicode::Tables::tolower(int c)\n"
    "{\n"
    "  static const int ul[] = {\n";

  size_t n = 0;

  for (Letter::const_iterator i = letter.begin(); i != letter.end(); ++i)
  {
    Code::const_iterator j = code.end();
    std::string s = i->second;
    size_t k = s.find("CAPITAL");
    if (k != std::string::npos)
    {
      j = code.find(s.replace(k, 7, "SMALL"));
      if (j != code.end())
      {
        std::cout << "    " << i->first << ", " << j->second << ",\n";
        ++n;
      }
    }
  }

  std::cout <<
    "  };\n"
    "  const int key[2] = { c, 0 };\n"
    "  const int *val = static_cast<const int*>(std::bsearch(key, ul, " << n << ", sizeof(key), &cmpint2));\n"
    "  return val != NULL ? val[1] : c;\n"
    "}\n"
    "\n"
    "int reflex::Unicode::Tables::toupper(int c)\n"
    "{\n"
    "  static const int lu[] = {\n";

  n = 0;

  for (Letter::const_iterator i = letter.begin(); i != letter.end(); ++i)
  {
    Code::const_iterator j = code.end();
    std::string s = i->second;
    size_t k = s.find("SMALL");
    if (k != std::string::npos)
    {
      j = code.find(s.replace(k, 5, "CAPITAL"));
      if (j != code.end())
      {
        std::cout << "    " << i->first << ", " << j->second << ",\n";
        ++n;
      }
    }
  }

  std::cout <<
    "  };\n"
    "  const int key[2] = { c, 0 };\n"
    "  const int *val = static_cast<const int*>(std::bsearch(key, lu, " << n << ", sizeof(key), &cmpint2));\n"
    "  return val != NULL ? val[1] : c;\n"
    "}" << std::endl;
}
