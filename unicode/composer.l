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
@file      composer.l
@brief     RE/Flex specification to convert Unicode UnicodeData.txt to C++
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2024, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

%{

#include <iostream>
#include <map>

typedef std::map<int,int> Map;
typedef std::map<int,Map> Compositions;

int x, y, z;
Compositions p;

%}

%o matcher=reflex
%o nodefault

%x COMPOSITION

hex			[[:xdigit:]]+

%%

^{hex}+;.*;L;		sscanf(text(), "%x", &x); start(COMPOSITION);
.|\n			// skip remainder

<COMPOSITION>{
{hex}+[ ]{hex}+		sscanf(text(), "%x %x", &y, &z); p[z][y] = x; start(INITIAL);
.|\n			start(INITIAL);
}

%%

int main()
{
  Lexer().lex();

  std::cout <<
    "// Converted from http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt by composer.l\n"
    "// With Knuth-Bendix completion applied\n"
    "#include <reflex/unicode.h>\n"
    "void reflex::Unicode::Tables::composer()\n{\n";

  size_t old_rules = 0;
  for (Compositions::const_iterator i = p.begin(); i != p.end(); ++i)
    old_rules += i->second.size();

  // apply Knuth-Bendix
  // y z v -> x v -> w
  // y v z -> u z -> w

  int pass = 0;

  while (true)
  {
    fprintf(stderr, "\n*** PASS %d ***\n\n", ++pass);

    Compositions q;
    for (Compositions::const_iterator i = p.begin(); i != p.end(); ++i)
    {
      int z = i->first;
      for (Map::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
      {
        int y = j->first;
        int x = j->second;
        for (Compositions::const_iterator k = p.begin(); k != p.end(); ++k)
        {
          int v = k->first;
          if (v != z)
          {
            Map::const_iterator l = k->second.find(x);
            if (l != k->second.end())
            {
              int w = l->second;
              fprintf(stderr, "%.4x %.4x %.4x -> %.4x %.4x -> %.4x", y, z, v, x, v, w);
              bool confluent = false;
              Compositions::const_iterator m = p.find(v);
              if (m != p.end())
              {
                Map::const_iterator n = m->second.find(y);
                if (n != m->second.end())
                {
                  int u = n->second;
                  Map::const_iterator o = i->second.find(u);
                  if (o != i->second.end())
                  {
                    fprintf(stderr, " is confluent %.4x %.4x %.4x -> %.4x %.4x -> %.4x\n", y, v, z, u, z, w);
                  }
                  else
                  {
                    fprintf(stderr, " adding rules %.4x %.4x %.4x -> %.4x %.4x -> %.4x\n", y, v, z, u, z, w);
                    q[v][y] = z;
                    q[z][u] = w;
                  }
                  confluent = true;
                }
              }
              if (!confluent)
                fprintf(stderr, " is not confluent\n");
            }
          }
        }
      }
    }

    if (q.empty())
      break;

    for (Compositions::const_iterator i = q.begin(); i != q.end(); ++i)
    {
      int z = i->first;
      for (Map::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
      {
        int y = j->first;
        int x = j->second;
        p[z][y] = x;
      }
    }
  }

  // write contractions y z -> x

  size_t rules = 0;

  for (Compositions::const_iterator i = p.begin(); i != p.end(); ++i)
  {
    int z = i->first;
    std::cout << "  static const int _" << z << "[] = {\n";
    for (Map::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
    {
      int y = j->first;
      int x = j->second;
      std::cout << "    " << y << ", " << x << ",\n";
    }
    rules += i->second.size();
    std::cout << "    0, 0\n  };" << std::endl;
    std::cout << "  compose[" << z << "] = _" << z << ";\n";
  }

  std::cout << "}" << std::endl;

  fprintf(stderr, "\nKnuth-Bendix completion took %d passes to add %zu new rules for %zu rules total\n\n", pass, rules - old_rules, rules);
}
