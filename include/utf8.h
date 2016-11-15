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
@file      utf8.h
@brief     RE/flex UCS to UTF-8 converters
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2016, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_UTF8_H
#define REFLEX_UTF8_H

#include <string>
#include <cstring>

namespace reflex {

/// Convert a UCS range [a,b] to a UTF-8 regex pattern.
/// @returns regex string to match the UCS range encoded in UTF-8.
std::string utf8(
    wchar_t     a,             ///< lower bound of UCS range
    wchar_t     b,             ///< upper bound of UCS range
    bool        strict = true, ///< returned regex is strict UTF-8 (true) or lean UTF-8 (false)
    const char *esc = NULL     ///< escape char(s), 0-3 chars limit, one backslash "\\" is the default
    );

/// Convert UCS to UTF-8.
/// @returns length (in bytes) of UTF-8 character sequence stored in s.
inline size_t utf8(
    wchar_t c, ///< UCS character
    char   *s) ///< points to the buffer to populate with UTF-8 (1 to 6 bytes)
{
  if (c < 0x80)
  {
    *s++ = static_cast<char>(c);
    return 1;
  }
  char *t = s;
  if (c < 0x0800)
  {
    *s++ = static_cast<char>(0xC0 | ((c >> 6) & 0x1F));
  }
  else
  {
    if (c < 0x010000)
    {
      *s++ = static_cast<char>(0xE0 | ((c >> 12) & 0x0F));
    }
    else
    {
      size_t w = c;
      if (c < 0x200000)
      {
        *s++ = static_cast<char>(0xF0 | ((w >> 18) & 0x07));
      }
      else
      {
        if (w < 0x04000000)
        {
          *s++ = static_cast<char>(0xF8 | ((w >> 24) & 0x03));
        }
        else
        {
          *s++ = static_cast<char>(0xFC | ((w >> 30) & 0x01));
          *s++ = static_cast<char>(0x80 | ((w >> 24) & 0x3F));
        }
        *s++ = static_cast<char>(0x80 | ((w >> 18) & 0x3F));
      }
      *s++ = static_cast<char>(0x80 | ((c >> 12) & 0x3F));
    }
    *s++ = static_cast<char>(0x80 | ((c >> 6) & 0x3F));
  }
  *s++ = static_cast<char>(0x80 | (c & 0x3F));
  return s - t;
}

} // namespace reflex

#endif
