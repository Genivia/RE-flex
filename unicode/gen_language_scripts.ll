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
@file      language_scripts.l
@brief     RE/Flex specification to convert Unicode Scripts.txt to a C++ map
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

int lo, hi;
Scripts p;

%}

%o matcher=reflex
%o nodefault
%x SCRIPT

hex			[[:xdigit:]]+

%%

^#.*\n			// skip comments
^\h*\n			// skip empty lines
^{hex}			sscanf(text(), "%x", &lo); hi = lo; start(SCRIPT);
^{hex}".."{hex}		sscanf(text(), "%x..%x", &lo, &hi); start(SCRIPT);

<SCRIPT>[^;#\n]+	// skip any except ;, #, and \n
<SCRIPT>"; "\w+		p[text()+2].insert(lo, hi);
<SCRIPT>"# "..		p[text()+2].insert(lo, hi);
<SCRIPT>\n		start(INITIAL);

%%

int main()
{
  Lexer().lex();

  (     p["C"] = p["Cc"]) |= p["Cf"]; // |= p["Cn"]; omitted: do not include unassigned Cn
  ((    p["L"] = p["L&"]) |= p["Lm"]) |= p["Lo"];
  ((    p["M"] = p["Mn"]) |= p["Mc"]) |= p["Me"];
  ((    p["N"] = p["Nd"]) |= p["Nl"]) |= p["No"];
  ((((((p["P"] = p["Pd"]) |= p["Ps"]) |= p["Pe"]) |= p["Pi"]) |= p["Pf"]) |= p["Pc"]) |= p["Po"];
  (((   p["S"] = p["Sm"]) |= p["Sc"]) |= p["Sk"]) |= p["So"];
  ((    p["Z"] = p["Zs"]) |= p["Zl"]) |= p["Zp"];

  // Add ASCII and Unicode character range U+00 to U+10FFFF minus the UTF-16 surrogate halves
   p["ASCII"]             = Chars(0x00, 0x7F);
  (p["Unicode"]           = Chars(0x00, 0x10FFFF)) -= Chars(0xD800, 0xDFFF);
  (p["Non_ASCII_Unicode"] = p["Unicode"])          -= p["ASCII"];

  // Add Unicode Space, includes blank characters HT (9), LF (10), VT (11), FF (12), CR (13)
  (p["Space"] = p["Zs"]) |= Chars(0x09, 0x0D);

  // Add Unicode Word
  ((p["Word"]  = p["L"]) |= p["Nd"]) |= p["Pc"];

  // Add Unicode identifier properties
  (((    p["IdentifierIgnorable"]    = Chars(0x00, 0x08)) |= Chars(0x0E, 0x1B)) |= Chars(0x7F)) |= Chars(0x80, 0x9F);
  (      p["UnicodeIdentifierStart"] = p["L"]) |= p["Nl"];
  (((((((p["UnicodeIdentifierPart"]  = p["L"]) |= p["Nl"]) |= p["Pc"]) |= p["Mn"]) |= p["Mc"]) |= p["Nd"]) |= p["Cf"]) |= p["IdentifierIgnorable"];

  // Add Java identifier properties
  (((     p["JavaIdentifierStart"] = p["L"]) |= p["Nl"]) |= p["Sc"]) |= p["Pc"];
  ((((((((p["JavaIdentifierPart"]  = p["L"]) |= p["Nl"]) |= p["Sc"]) |= p["Pc"]) |= p["Mn"]) |= p["Mc"]) |= p["Nd"]) |= p["Cf"]) |= p["IdentifierIgnorable"];

  // Add C# identifier properties
  (((   p["CsIdentifierStart"] = p["L"]) |= p["Nl"]) |= p["Pc"]) |= Chars('@');
  ((((((p["CsIdentifierPart"]  = p["L"]) |= p["Nl"]) |= p["Pc"]) |= p["Mn"]) |= p["Mc"]) |= p["Nd"]) |= p["Cf"];

  // Add Python 3.x identifier properties, includes Other_ID_Start and Other_ID_Continue
  (((((    p["PythonIdentifierStart"] = p["L"]) |= p["Nl"]) |= Chars(0x1885, 0x1886)) |= Chars(0x2118)) |= Chars(0x212E)) |= Chars(0x309B, 0x309C);
  (((((((((p["PythonIdentifierPart"]  = p["L"]) |= p["Nl"]) |= p["Pc"]) |= p["Mn"]) |= p["Mc"]) |= p["Nd"]) |= Chars(0xB7)) |= Chars(0x0387)) |= Chars(0x1369, 0x1371)) |= Chars(0x19DA);

  std::cout <<
    "// Converted from http://www.unicode.org/Public/UCD/latest/ucd/Scripts.txt by language_scripts.l\n"
    "#include <reflex/unicode.h>\n"
    "void reflex::Unicode::Tables::language_scripts(void)\n{\n";

  // Write range[] code
  for (Scripts::const_iterator i = p.begin(); i != p.end(); ++i)
  {
    const std::string& name = i->first.compare("L&") == 0 ? "L_" : i->first;
    std::cout << "  static const int " << name << "[] = {\n";
    for (Chars::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
      std::cout << "    " << j->first << ", " << j->second-1 << ",\n";
    std::cout << "    0, 0\n  };" << std::endl;
    std::cout << "  range[\"" << i->first << "\"] = " << name << ";\n";
  }

  std::cout << "}" << std::endl;
}
