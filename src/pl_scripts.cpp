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
@file      pl_scripts.cpp
@brief     RE/Flex scanner generator replacement for Flex/Lex
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2016, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#include "pl_scripts.h"

void pl_scripts(Reflex::Map& scripts)
{
  scripts["IdentifierIgnorable"]    = "[\\x00-\\x08]|[\\x0e-\\x1b]|\\x7f|\\xc2[\\x80-\\x9f]";
  scripts["UnicodeIdentifierStart"] = scripts["L"] + "|"
                                    + scripts["Nl"];
  scripts["UnicodeIdentifierPart"]  = scripts["L"] + "|"
                                    + scripts["Nl"] + "|"
                                    + scripts["Pc"] + "|"
                                    + scripts["Mn"] + "|"
                                    + scripts["Mc"] + "|"
                                    + scripts["Nd"] + "|"
                                    + scripts["Cf"] + "|"
                                    + scripts["IdentifierIgnorable"];

  // Java identifier properties
  scripts["JavaIdentifierStart"]    = scripts["L"] + "|"
                                    + scripts["Nl"] + "|"
                                    + scripts["Sc"] + "|"
                                    + scripts["Pc"];
  scripts["JavaIdentifierPart"]     = scripts["L"] + "|"
                                    + scripts["Nl"] + "|"
                                    + scripts["Sc"] + "|"
                                    + scripts["Pc"] + "|"
                                    + scripts["Mn"] + "|"
                                    + scripts["Mc"] + "|"
                                    + scripts["Nd"] + "|"
                                    + scripts["Cf"] + "|"
                                    + scripts["IdentifierIgnorable"];

  // C# identifier properties
  scripts["CsIdentifierStart"]      = scripts["L"] + "|"
                                    + scripts["Nl"] + "|"
                                    + scripts["Pc"] + "|"
                                    + "@";
  scripts["CsIdentifierPart"]       = scripts["L"] + "|"
                                    + scripts["Nl"] + "|"
                                    + scripts["Pc"] + "|"
                                    + scripts["Mn"] + "|"
                                    + scripts["Mc"] + "|"
                                    + scripts["Nd"] + "|"
                                    + scripts["Cf"];

  // Python 3.x identifier properties
  scripts["PythonIdentifierStart"]  = scripts["L"] + "|"
                                    + scripts["Nl"] + "|"
                                    + "\\xe2\\x84\\x98|\\xe2\\x84\\xae|\\xe3\\x82[\\x9b-\\x9c]"; // Other_ID_Start
  scripts["PythonIdentifierPart"]   = scripts["L"] + "|"
                                    + scripts["Nl"] + "|"
                                    + scripts["Pc"] + "|"
                                    + scripts["Mn"] + "|"
                                    + scripts["Mc"] + "|"
                                    + scripts["Nd"] + "|"
                                    + "\\xc2\\xb7|\\xce\\x87|\\xe1\\x8d[\\xa9-\\xb1]|\\xe1\\xa7\\x9a"; // Other_ID_Continue
}
