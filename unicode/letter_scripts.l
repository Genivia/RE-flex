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
@file      letter_scripts.l
@brief     RE/Flex specification to convert Unicode UnicodeData.txt to a C++ map
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2016, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

%{

#include <reflex/ranges.h>
#include <iostream>
#include <map>

typedef reflex::ORanges<int>        Chars;   // open-ended ranges of characters
typedef std::map<std::string,Chars> Scripts; // maps Unicode category/script to open-ended ranges

int x;
Scripts p;

%}

%o matcher=reflex
%o nodefault

hex			[[:xdigit:]]+

%%

^{hex};[^;\n]+;Ll;.+\n	sscanf(text(), "%x", &x); p["Ll"].insert(x);
^{hex};[^;\n]+;Lu;.+\n	sscanf(text(), "%x", &x); p["Lu"].insert(x);
^{hex};[^;\n]+;Lt;.+\n	sscanf(text(), "%x", &x); p["Lt"].insert(x);
.*\n			// skip remainder

%%

int main()
{
  Lexer().lex();

  std::cout <<
    "// Converted from http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt by letter_scripts.l\n"
    "#include <reflex/unicode.h>\n"
    "void reflex::Unicode::Tables::letter_scripts(void)\n{\n";

  // Write range[] code
  for (Scripts::const_iterator i = p.begin(); i != p.end(); ++i)
  {
    const std::string& name = i->first;
    std::cout << "  static const int " << name << "[] = {\n";
    for (Chars::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
    {
      std::cout << "    " << j->first << ", " << j->second-1 << ",\n";
    }
    std::cout << "    0, 0\n  };" << std::endl;
    std::cout << "  range[\"" << i->first << "\"] = " << name << ";\n";
  }

  std::cout << "}" << std::endl;
}
