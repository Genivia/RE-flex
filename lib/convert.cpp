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
@file      convert.cpp
@brief     RE/flex regex converter
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2017, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#include <reflex/convert.h>
#include <reflex/posix.h>
#include <reflex/ranges.h>
#include <reflex/unicode.h>
#include <reflex/utf8.h>
#include <cctype>
#include <cstdlib>
#include <cstring>

namespace reflex {

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Regex converter constants                                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// regex meta chars
static const char regex_meta[] = "#$()*+.?[\\]^{|}";

/// regex chars that can be escaped
static const char regex_escapes[] = "!\"#%&-/:;@~";

/// regex anchors and boundaries
static const char regex_anchors[] = "AZzBby`'<>";

/// \a (BEL), \b (BS), \t (TAB), \n (LF), \v (VT), \f (FF), \r (CR)
static const char regex_abtnvfr[] = "abtnvfr";

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Regex converter checks for modifiers and escapes                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

inline bool enable_modifier(int c, const char *pattern, size_t pos, std::map<int,size_t>& mod, size_t lev) throw (regex_error)
{
  std::map<int,size_t>::iterator i = mod.find(c);
  if (i == mod.end())
  {
    if (c != 'i' && c != 'm' && c != 's' && c != 'x')
      throw regex_error(regex_error::invalid_modifier, pattern, pos);
    mod[c] = lev;
    return false;
  }
  if (i->second == 0)
    return true;
  i->second = lev;
  return false;
}

inline bool is_modified(const std::map<int,size_t>& mod, int c)
{
  std::map<int,size_t>::const_iterator i = mod.find(c);
  return i != mod.end() && i->second > 0;
}

inline bool supports_modifier(const std::map<int,size_t>& mod, int c)
{
  std::map<int,size_t>::const_iterator i = mod.find(c);
  return i != mod.end() && i->second == 0;
}

inline bool supports_escape(const char *escapes, int escape)
{
  return escapes != NULL && std::strchr(escapes, escape) != NULL;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Regex converter helper functions                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

inline char lower(int c)
{
  return c | 0x20;
}

inline char upper(int c)
{
  return c & ~0x20;
}

inline int hex_or_octal_escape(const char *escapes)
{
  if (supports_escape(escapes, 'x'))
    return 'x';
  if (supports_escape(escapes, '0'))
    return '0';
  return '\0';
}

static void posix(const char *s, std::string& r, int esc, const char *par)
{
  const int *wc = Posix::range(s + (s[0] == '^'));
  if (wc == NULL)
    return;
  if (s[0] == '^')
  {
    if (wc[0] > 0x00)
      r = utf8(0x00, wc[0] - 1, esc, par);
    int last = wc[1] + 1;
    wc += 2;
    for (; wc[1] != 0; wc += 2)
    {
      r.append("|").append(utf8(last, wc[0] - 1, esc, par));
      last = wc[1] + 1;
    }
    if (last < 0x7F)
      r.append("|").append(utf8(last, 0x7f, esc, par));
  }
  else
  {
    r = utf8(wc[0], wc[1], esc, par);
    wc += 2;
    for (; wc[1] != 0; wc += 2)
      r.append("|").append(utf8(wc[0], wc[1], esc, par));
  }
}

static void unicode(const char *s, std::string& r, int esc, const char *par)
{
  const int *wc = Unicode::range(s + (s[0] == '^'));
  if (wc == NULL)
    return;
  if (s[0] == '^')
  {
    if (wc[0] > 0x00)
      r = utf8(0x00, wc[0] - 1, esc, par);
    int last = wc[1] + 1;
    wc += 2;
    for (; wc[1] != 0; wc += 2)
    {
      r.append("|").append(utf8(last, wc[0] - 1, esc, par));
      last = wc[1] + 1;
    }
    if (last < 0x10FFFF)
      r.append("|").append(utf8(last, 0x10FFFF, esc, par));
  }
  else
  {
    r = utf8(wc[0], wc[1], esc, par);
    wc += 2;
    for (; wc[1] != 0; wc += 2)
      r.append("|").append(utf8(wc[0], wc[1], esc, par));
  }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Regex converter escaped character conversions                             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

static void convert_escape_char(const char *pattern, size_t& loc, size_t& pos, convert_flag_type flags, const char *escapes, const std::map<int,size_t>& mod, const char *par, std::string& regex) throw (regex_error)
{
  int c = pattern[pos];
  if (std::strchr(regex_escapes, c) != NULL)
  {
    // translate \x to x
    regex.append(&pattern[loc], pos - loc - 1);
    loc = pos;
  }
  else if (std::strchr(regex_meta, c) == NULL)
  {
    char buf[3] = { '^', lower(c), '\0' };
    const char *name = buf + (std::islower(c) != 0);
    std::string translated;
    int esc = hex_or_octal_escape(escapes);
    if (!(flags & convert_flag::unicode))
    {
      posix(name, translated, esc, par);
      if (!translated.empty() && supports_escape(escapes, c))
      {
        // assume regex lib supports this POSIX character class
        translated.clear();
      }
      else if (translated.empty())
      {
        unicode(name, translated, esc, par);
      }
    }
    else
    {
      unicode(name, translated, esc, par);
    }
    if (!translated.empty())
    {
      if (translated.find('|') != std::string::npos)
        regex.append(&pattern[loc], pos - loc - 1).append(par).append(translated).append(")");
      else
        regex.append(&pattern[loc], pos - loc - 1).append(translated);
      loc = pos + 1;
    }
    else if (!supports_escape(escapes, c))
    {
      if (c == 'Z')
      {
        if (!supports_escape(escapes, 'z') || !supports_modifier(mod, '='))
          throw regex_error(regex_error::invalid_anchor, pattern, pos);
        // translate \Z to (?=[\n\r]*\z)
        regex.append(&pattern[loc], pos - loc - 1).append("(?=(\\r?\\n)?\\z)");
        loc = pos + 1;
      }
      else if (c == '`')
      {
        if (!supports_escape(escapes, 'A'))
          throw regex_error(regex_error::invalid_anchor, pattern, pos);
        // translate \` to \A
        regex.append(&pattern[loc], pos - loc - 1).append("\\A");
        loc = pos + 1;
      }
      else if (c == '\'')
      {
        if (!supports_escape(escapes, 'z'))
          throw regex_error(regex_error::invalid_anchor, pattern, pos);
        // translate \' to \z
        regex.append(&pattern[loc], pos - loc - 1).append("\\z");
        loc = pos + 1;
      }
      else if (c == 'b')
      {
        if (!supports_escape(escapes, 'y'))
          throw regex_error(regex_error::invalid_anchor, pattern, pos);
        // translate \b to \y
        regex.append(&pattern[loc], pos - loc - 1).append("\\y");
        loc = pos + 1;
      }
      else if (c == 'y')
      {
        if (!supports_escape(escapes, 'b'))
          throw regex_error(regex_error::invalid_anchor, pattern, pos);
        // translate \y to \b
        regex.append(&pattern[loc], pos - loc - 1).append("\\b");
        loc = pos + 1;
      }
      else if (c == '<')
      {
        if (!supports_escape(escapes, 'b') || !supports_escape(escapes, 'w') || !supports_modifier(mod, '='))
          throw regex_error(regex_error::invalid_anchor, pattern, pos);
        // translate \< to \b(?=\w)
        regex.append(&pattern[loc], pos - loc - 1).append("\\b(?=\\w)");
        loc = pos + 1;
      }
      else if (c == '>')
      {
        if (!supports_escape(escapes, 'b') || !supports_escape(escapes, 'w') || !supports_modifier(mod, '<'))
          throw regex_error(regex_error::invalid_anchor, pattern, pos);
        // translate \> to \b(?<=\w)
        regex.append(&pattern[loc], pos - loc - 1).append("\\b(?<=\\w)");
        loc = pos + 1;
      }
      else
      {
        if (std::strchr(regex_anchors, c))
          throw regex_error(regex_error::invalid_anchor, pattern, pos);
        const char *s = std::strchr(regex_abtnvfr, c);
        if (s == NULL)
          throw regex_error(regex_error::invalid_escape, pattern, pos);
        int wc = static_cast<int>(s - regex_abtnvfr + '\a');
        regex.append(&pattern[loc], pos - loc - 1).append(utf8(wc, wc, esc, par));
        loc = pos + 1;
      }
    }
  }
}

static int convert_hex(const char *pattern, size_t len, size_t& pos, convert_flag_type flags)
{
  char hex[9];
  hex[0] = '\0';
  size_t k = pos;
  int c = pattern[k++];
  if (k < len && pattern[k] == '{')
  {
    char *s = hex;
    while (++k < len && s < hex + sizeof(hex) - 1 && (c = pattern[k]) != '}')
      *s++ = c;
    *s = '\0';
  }
  else if (c == 'x' || (c == 'u' && (flags & convert_flag::u4)))
  {
    char *s = hex;
    size_t n = pos + 3;
    if (c == 'u')
      n += 2;
    while (k < n && k < len && std::isxdigit(c = pattern[k++]))
      *s++ = c;
    *s = '\0';
    --k;
  }
  if (hex[0] != '\0')
  {
    pos = k;
    return static_cast<int>(std::strtoul(hex, NULL, 16));
  }
  return -1;
}

static void convert_escape(const char *pattern, size_t len, size_t& loc, size_t& pos, convert_flag_type flags, const char *escapes, const std::map<int,size_t>& mod, const char *par, std::string& regex) throw (regex_error)
{
  int c = pattern[pos];
  if (c == '\n' || c == '\r')
  {
    // remove line continuation from \ \n (\ \r\n) to next line, skipping indent
    regex.append(&pattern[loc], pos - loc - 1);
    if (++pos < len && pattern[pos] == '\n')
      ++pos;
    while (pos < len && ((c = pattern[pos]) == ' ' || c == '\t'))
      ++pos;
    loc = pos;
  }
  else if (c == 'c')
  {
    ++pos;
    if (pos >= len)
      throw regex_error(regex_error::invalid_escape, pattern, pos - 1);
    c = pattern[pos];
    if (c < 0x21 || c >= 0x7F)
      throw regex_error(regex_error::invalid_escape, pattern, pos);
    c &= 0x1F;
    if (!supports_escape(escapes, 'c'))
    {
      // translate \cX to \xXX
      int esc = hex_or_octal_escape(escapes);
      regex.append(&pattern[loc], pos - loc - 2).append(utf8(c, c, esc, par));
      loc = pos + 1;
    }
  }
  else if (c == 'e')
  {
    if (!supports_escape(escapes, 'e'))
    {
      // translate \e to \x1b
      regex.append(&pattern[loc], pos - loc - 1).append("\\x1b");
      loc = pos + 1;
    }
  }
  else if (c >= '0' && c <= '7')
  {
    size_t k = pos;
    size_t n = k + 3 + (pattern[k] == '0');
    int wc = 0;
    while (k < n && (c = pattern[k]) >= '0' && c <= '7')
    {
      wc = 8 * wc + c - '0';
      ++k;
    }
    if (std::isalpha(wc) && is_modified(mod, 'i'))
    {
      // anycase: translate A to [Aa]
      regex.append(&pattern[loc], pos - loc - 1).append("[");
      regex.push_back(wc);
      regex.push_back(wc ^ 0x20);
      regex.append("]");
    }
    else
    {
      int esc = hex_or_octal_escape(escapes);
      regex.append(&pattern[loc], pos - loc - 1).append(utf8(wc, wc, esc, par));
    }
    pos = k - 1;
    loc = pos + 1;
  }
  else if (c == 'u' || c == 'x')
  {
    size_t k = pos;
    int wc = convert_hex(pattern, len, k, flags);
    if (wc >= 0)
    {
      // int wc = static_cast<int>(std::strtoul(hex, NULL, 16));
      if (wc < 0x80)
      {
        // translate \u{X}, \u00XX and \x{X} to \xXX
        if (std::isalpha(wc) && is_modified(mod, 'i'))
        {
          // anycase: translate A to [Aa]
          regex.append(&pattern[loc], pos - loc - 1).append("[");
          regex.push_back(wc);
          regex.push_back(wc ^ 0x20);
          regex.append("]");
        }
        else
        {
	  int esc = hex_or_octal_escape(escapes);
          regex.append(&pattern[loc], pos - loc - 1).append(utf8(wc, wc, esc, par));
        }
      }
      else
      {
        if (c == 'u' && wc >= 0xD800 && wc < 0xE000)
        {
          // translate surrogate pair \uDXXX\uDYYY
          if (k + 2 >= len || pattern[k + 1] != '\\' || pattern[k + 2] != 'u')
            throw regex_error(regex_error::invalid_escape, pattern, k);
          k += 2;
          int c2 = convert_hex(pattern, len, k, flags);
          if (c2 < 0 || (c2 & 0xFC00) != 0xDC00)
            throw regex_error(regex_error::invalid_escape, pattern, k - 2);
          wc = 0x010000 - 0xDC00 + ((wc - 0xD800) << 10) + c2;
        }
        // translate \u{X}, \uXXXX, \uDXXX\uDYYY, and \x{X} to UTF-8 pattern
        char buf[8];
        buf[utf8(wc, buf)] = '\0';
        regex.append(&pattern[loc], pos - loc - 1).append(par).append(buf).append(")");
      }
      pos = k;
      loc = pos + 1;
    }
    else
    {
      convert_escape_char(pattern, loc, pos, flags, escapes, mod, par, regex);
    }
  }
  else if (c == 'p' || c == 'P')
  {
    size_t k = ++pos;
    if (pos >= len)
      throw regex_error(regex_error::invalid_class, pattern, pos);
    // get name X of \pX, \PX, \p{X}, and \P{X}
    std::string name;
    if (pattern[pos] == '{')
    {
      size_t j = pos + 1;
      if (c == 'P')
        name.push_back('^');
      if (j + 2 < len && pattern[j] == 'I' && (pattern[j + 1] == 'n' || pattern[j + 1] == 's'))
        j += 2;
      k = j;
      while (k < len && pattern[k] != '}')
        ++k;
      name.append(pattern, j, k - j);
    }
    else
    {
      if (c == 'P')
        name.push_back('^');
      name.push_back(pattern[pos]);
    }
    std::string translated;
    int esc = hex_or_octal_escape(escapes);
    if (!(flags & convert_flag::unicode))
    {
      posix(name.c_str(), translated, esc, par);
      if (!translated.empty() && supports_escape(escapes, c))
      {
        // assume regex lib supports this POSIX character class
        translated.clear();
      }
      else if (translated.empty())
      {
        unicode(name.c_str(), translated, esc, par);
      }
    }
    else
    {
      unicode(name.c_str(), translated, esc, par);
    }
    if (!translated.empty())
    {
      if (translated.find('|') != std::string::npos)
        regex.append(&pattern[loc], pos - loc - 2).append(par).append(translated).append(")");
      else
        regex.append(&pattern[loc], pos - loc - 2).append(translated);
      loc = k + 1;
    }
    pos = k;
  }
  else
  {
    convert_escape_char(pattern, loc, pos, flags, escapes, mod, par, regex);
  }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Regex converter bracket list character class conversions                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

static void insert_escape_class(const char *pattern, size_t& pos, convert_flag_type flags, ORanges<int>& ranges) throw (regex_error)
{
  int c = pattern[pos];
  char name[2] = { lower(c), '\0' };
  const int *translated = NULL;
  if (!(flags & convert_flag::unicode))
    translated = Posix::range(name);
  if (translated == NULL)
    translated = Unicode::range(name);
  if (translated == NULL)
    throw regex_error(regex_error::invalid_escape, pattern, pos);
  if (std::islower(c))
  {
    for (const int *wc = translated; wc[1] != 0; wc += 2)
      ranges.insert(wc[0], wc[1]);
  }
  else
  {
    int last = 0x00;
    for (const int *wc = translated; wc[1] != 0; wc += 2)
    {
      if (wc[0] > 0x00)
        ranges.insert(last, wc[0] - 1);
      last = wc[1] + 1;
    }
    if ((flags & convert_flag::unicode) && last < 0x10FFFF)
      ranges.insert(last, 0x10FFFF);
    else if (last < 0x7F)
      ranges.insert(last, 0x7F);
  }
}

static int insert_escape(const char *pattern, size_t len, size_t& pos, convert_flag_type flags, ORanges<int>& ranges) throw (regex_error)
{
  int c = pattern[pos];
  if (c == 'c')
  {
    ++pos;
    if (pos >= len)
      throw regex_error(regex_error::invalid_escape, pattern, pos - 1);
    c = pattern[pos];
    if (c < 0x21 || c >= 0x7F)
      throw regex_error(regex_error::invalid_escape, pattern, pos);
    c &= 0x1F;
  }
  else if (c == 'e')
  {
    c = 0x1B;
  }
  else if (c >= '0' && c <= '7')
  {
    size_t k = pos + 3 + (pattern[pos] == '0');
    int wc = c - '0';
    while (++pos < k && (c = pattern[pos]) >= '0' && c <= '7')
      wc = 8 * wc + c - '0';
    c = wc;
    --pos;
  }
  else if (c == 'u' || c == 'x')
  {
    size_t k = pos;
    c = convert_hex(pattern, len, k, flags);
    if (c >= 0)
    {
      pos = k;
    }
    else
    {
      insert_escape_class(pattern, pos, flags, ranges);
      return -1;
    }
  }
  else if (c == 'p' || c == 'P')
  {
    size_t k = ++pos;
    if (k >= len)
      throw regex_error(regex_error::invalid_class, pattern, k);
    std::string name;
    if (pattern[k] == '{')
    {
      size_t j = k + 1;
      if (j + 2 < len && pattern[j] == 'I' && (pattern[j + 1] == 'n' || pattern[j + 1] == 's'))
        j += 2;
      k = j;
      while (k < len && pattern[k] != '}')
        ++k;
      name.assign(&pattern[j], k - j);
    }
    else
    {
      name.push_back(pattern[k]);
    }
    const int *translated = NULL;
    const char *s = name.c_str();
    if (s[0] == '^')
      ++s;
    if (!(flags & convert_flag::unicode))
      translated = Posix::range(s);
    if (translated == NULL)
      translated = Unicode::range(s);
    if (translated == NULL)
      throw regex_error(regex_error::invalid_class, pattern, pos);
    if (c == 'P' || name.at(0) == '^')
    {
      int last = 0x00;
      for (const int *wc = translated; wc[1] != 0; wc += 2)
      {
        if (wc[0] > 0x00)
          ranges.insert(last, wc[0] - 1);
        last = wc[1] + 1;
      }
      if ((flags & convert_flag::unicode) && last < 0x10FFFF)
        ranges.insert(last, 0x10FFFF);
      else if (last < 0x7F)
        ranges.insert(last, 0x7F);
    }
    else
    {
      for (const int *wc = translated; wc[1] != 0; wc += 2)
        ranges.insert(wc[0], wc[1]);
    }
    pos = k;
    return -1;
  }
  else if (std::isalpha(c))
  {
    const char *s = std::strchr(regex_abtnvfr, c);
    if (s == NULL)
    {
      insert_escape_class(pattern, pos, flags, ranges);
      return -1;
    }
    c = static_cast<int>(s - regex_abtnvfr + '\a');
  }
  ranges.insert(c);
  return c;
}

static void insert_posix_class(const char *pattern, size_t len, size_t& pos, ORanges<int>& ranges) throw (regex_error)
{
  pos += 2;
  char buf[8];
  char *name = buf;
  while (pos + 1 < len && name < buf + sizeof(buf) - 1 && (pattern[pos] != ':' || pattern[pos + 1] != ']'))
    *name++ = pattern[pos++];
  *name = '\0';
  name = buf + (*buf == '^');
  if (name[1] != '\0')
  {
    name[0] = upper(name[0]);
    if (name[0] == 'X' && name[1] == 'd')
      name = const_cast<char*>("XDigit");
    else if (name[0] == 'A' && name[1] == 's')
      name = const_cast<char*>("ASCII");
  }
  const int *translated = Posix::range(name);
  if (translated == NULL)
    throw regex_error(regex_error::invalid_class, pattern, pos);
  if (*buf == '^')
  {
    int last = 0x00;
    for (const int *wc = translated; wc[1] != 0; wc += 2)
    {
      if (wc[0] > 0x00)
        ranges.insert(last, wc[0] - 1);
      last = wc[1] + 1;
    }
    if (last < 0x7F)
      ranges.insert(last, 0x7F);
  }
  else
  {
    for (const int *wc = translated; wc[1] != 0; wc += 2)
      ranges.insert(wc[0], wc[1]);
  }
  ++pos;
}

static void insert_list(const char *pattern, size_t len, size_t& pos, convert_flag_type flags, ORanges<int>& ranges) throw (regex_error);

static void merge_list(const char *pattern, size_t len, size_t& pos, convert_flag_type flags, ORanges<int>& ranges) throw (regex_error)
{
  if (pattern[pos] == '^')
  {
    ++pos;
    ORanges<int> merge;
    insert_list(pattern, len, pos, flags, merge);
    ORanges<int>::const_reverse_iterator i = merge.rbegin();
    if ((flags & convert_flag::unicode) || (i != merge.rend() && i->second > 0x80))
    {
      ORanges<int> inverse(0x00, 0x10FFFF);
      inverse -= merge;
      ranges |= inverse;
    }
    else
    {
      ORanges<int> inverse(0x00, 0x7F);
      inverse -= merge;
      ranges |= inverse;
    }
  }
  else
  {
    insert_list(pattern, len, pos, flags, ranges);
  }
}

static void intersect_list(const char *pattern, size_t len, size_t& pos, convert_flag_type flags, ORanges<int>& ranges) throw (regex_error)
{
  ORanges<int> intersect;
  if (pattern[pos] == '^')
  {
    ++pos;
    insert_list(pattern, len, pos, flags, intersect);
    ORanges<int>::const_reverse_iterator i = intersect.rbegin();
    if ((flags & convert_flag::unicode) || (i != intersect.rend() && i->second > 0x80))
    {
      ORanges<int> inverse(0x00, 0x10FFFF);
      inverse -= intersect;
      ranges &= inverse;
    }
    else
    {
      ORanges<int> inverse(0x00, 0x7F);
      inverse -= intersect;
      ranges &= inverse;
    }
  }
  else
  {
    insert_list(pattern, len, pos, flags, intersect);
    ranges &= intersect;
  }
}

static void subtract_list(const char *pattern, size_t len, size_t& pos, convert_flag_type flags, ORanges<int>& ranges) throw (regex_error)
{
  ORanges<int> subtract;
  if (pattern[pos] == '^')
  {
    ++pos;
    insert_list(pattern, len, pos, flags, subtract);
    ORanges<int>::const_reverse_iterator i = subtract.rbegin();
    if ((flags & convert_flag::unicode) || (i != subtract.rend() && i->second > 0x80))
    {
      ORanges<int> inverse(0x00, 0x10FFFF);
      inverse -= subtract;
      ranges -= inverse;
    }
    else
    {
      ORanges<int> inverse(0x00, 0x7F);
      inverse -= subtract;
      ranges -= inverse;
    }
  }
  else
  {
    insert_list(pattern, len, pos, flags, subtract);
    ranges -= subtract;
  }
}

static void insert_list(const char *pattern, size_t len, size_t& pos, convert_flag_type flags, ORanges<int>& ranges) throw (regex_error)
{
  size_t loc = pos;
  bool range = false;
  int pc = -2;
  while (pos + 1 < len)
  {
    int c = pattern[pos];
    if (c == '\\')
    {
      ++pos;
      c = insert_escape(pattern, len, pos, flags, ranges);
      if (range)
      {
        if (c == -1 || pc > c)
          throw regex_error(regex_error::invalid_class_range, pattern, pos);
        ranges.insert(pc, c);
        range = false;
      }
      pc = c;
    }
    else if (c == '[' && pattern[pos + 1] == ':')
    {
      // POSIX character class (ASCII only)
      if (range)
        throw regex_error(regex_error::invalid_class_range, pattern, pos);
      insert_posix_class(pattern, len, pos, ranges);
      pc = -1;
    }
    else if (c == '|' && pattern[pos + 1] == '|' && pos + 3 < len && pattern[pos + 2] == '[')
    {
      // character class union [abc||[def]]
      if (range)
        throw regex_error(regex_error::invalid_class_range, pattern, pos);
      pos += 3;
      merge_list(pattern, len, pos, flags, ranges);
      pc = -1;
    }
    else if (c == '&' && pattern[pos + 1] == '&' && pos + 3 < len && pattern[pos + 2] == '[')
    {
      // character class intersection [a-z&&[^aeiou]]
      if (range)
        throw regex_error(regex_error::invalid_class_range, pattern, pos);
      pos += 3;
      intersect_list(pattern, len, pos, flags, ranges);
      pc = -1;
    }
    else if (c == '-' && pattern[pos + 1] == '-' && pos + 3 < len && pattern[pos + 2] == '[')
    {
      // character class subtraction [a-z--[aeiou]]
      if (range)
        throw regex_error(regex_error::invalid_class_range, pattern, pos);
      pos += 3;
      subtract_list(pattern, len, pos, flags, ranges);
      pc = -1;
    }
    else if (c == '-' && !range && pc != -2)
    {
      if (pc == -1)
        throw regex_error(regex_error::invalid_class_range, pattern, pos);
      range = true;
    }
    else
    {
      if ((c & 0xC0) == 0xC0)
      {
        // unicode: UTF-8 sequence
        const char *r;
        c = utf8(&pattern[pos], &r);
        pos += r - &pattern[pos] - 1;
      }
      if (range)
      {
        if (c == -1 || pc > c)
          throw regex_error(regex_error::invalid_class_range, pattern, pos);
        ranges.insert(pc, c);
        range = false;
      }
      else
      {
        ranges.insert(c);
      }
      pc = c;
    }
    ++pos;
    if (pos >= len)
      break;
    if (pattern[pos] == ']')
    {
      if (range)
        ranges.insert('-');
      if ((flags & convert_flag::lex))
      {
        while (pos + 5 < len && pattern[pos + 1] == '{' && (pattern[pos + 2] == '+' || pattern[pos + 2] == '-') && pattern[pos + 3] == '}' && pattern[pos + 4] == '[')
        {
          // lex: [abc]{+}[def] character class addition or [a-z]{-}[aeiou] subtraction
          c = pattern[pos + 2];
          pos += 5;
          if (c == '+')
            merge_list(pattern, len, pos, flags & ~convert_flag::lex, ranges);
          else
            subtract_list(pattern, len, pos, flags & ~convert_flag::lex, ranges);
        }
      }
      break;
    }
  }
  if (pos >= len || pattern[pos] != ']')
    throw regex_error(regex_error::mismatched_brackets, pattern, loc);
  if (ranges.empty())
    throw regex_error(regex_error::empty_class, pattern, loc);
}

static std::string convert_unicode_ranges(const ORanges<int>& ranges, const char *escapes, const char *par)
{
  std::string regex;
  const char *sep = par;
  int esc = hex_or_octal_escape(escapes);
  for (ORanges<int>::const_iterator i = ranges.begin(); i != ranges.end(); ++i)
  {
    regex.append(sep).append(utf8(i->first, i->second - 1, esc, par));
    sep = "|";
  }
  return regex.append(")");
}

static std::string convert_posix_ranges(const ORanges<int>& ranges, const char *escapes, const char *par)
{
  std::string regex;
  std::string range;
  int esc = hex_or_octal_escape(escapes);
  for (ORanges<int>::const_iterator i = ranges.begin(); i != ranges.end(); ++i)
  {
    range = utf8(i->first, i->second - 1, esc, par);
    if (range[0] == '[')
      regex.append(range.c_str() + 1, range.size() - 2);
    else
      regex.append(range);
  }
  return regex.append("]");
}

static void convert_anycase_ranges(ORanges<int>& ranges)
{
  ORanges<int> letters;
  letters.insert('A', 'Z');
  letters.insert('a', 'z');
  letters &= ranges;
  for (ORanges<int>::const_iterator i = letters.begin(); i != letters.end(); ++i)
    ranges.insert(i->first ^ 0x20, (i->second - 1) ^ 0x20);
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Regex converter                                                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

std::string convert(const char *pattern, const char *signature, convert_flag_type flags, const std::map<std::string,std::string> *macros) throw (regex_error)
{
  std::string regex;
  bool anc = false;
  bool beg = true;
  size_t pos = 0;
  size_t loc = 0;
  size_t lev = 0;
  size_t lap = 0;
  size_t len = std::strlen(pattern);
  const char *par = "(";
  const char *esc = signature;
  std::map<int,size_t> mod;
  bool can = false;
  if (std::strchr(signature, ':') != NULL)
  {
    const char *s;
    for (s = signature; *s != ':'; s++)
      mod[*s] = 0;
    esc = ++s;
    par = "(?:";
    can = true;
  }
  if ((flags & convert_flag::anycase))
    enable_modifier('i', pattern, 0, mod, lev + 1);
  if ((flags & convert_flag::multiline))
    enable_modifier('m', pattern, 0, mod, lev + 1);
  if ((flags & convert_flag::dotall))
    enable_modifier('s', pattern, 0, mod, lev + 1);
  if ((flags & convert_flag::freespace))
    enable_modifier('x', pattern, 0, mod, lev + 1);
  if (len > 2 && pattern[0] == '(' && pattern[1] == '?')
  {
    // directive (?...) mode modifier
    std::string mods;
    size_t k = 2;
    while (k < len && std::isalpha(pattern[k]))
    {
      if (enable_modifier(pattern[k], pattern, k, mod, lev + 1))
        mods.push_back(pattern[k]);
      ++k;
    }
    if (k < len && pattern[k] == ')')
    {
      // preserve (?imsx) at start of regex
      if (can)
        regex.append("(?").append(mods).append(")");
      pos = k + 1;
      loc = pos;
    }
  }
  if ((flags & convert_flag::recap))
  {
    // recap: translate x|y to (x)|(y)
    regex.append(&pattern[loc], pos - loc).append("(");
    loc = pos;
  }
  while (pos < len)
  {
    int c = pattern[pos];
    switch (c)
    {
      case '\\':
        if (pos + 1 >= len)
          throw regex_error(regex_error::invalid_escape, pattern, pos);
	anc = false;
        c = pattern[++pos];
        if (c == 'Q')
        {
          if (!supports_escape(esc, 'Q'))
          {
            // \Q is not a supported escape, translate by grouping and escaping meta chars up to the closing \E
            regex.append(&pattern[loc], pos - loc - 1);
            size_t k = loc = ++pos;
            while (pos + 1 < len && (pattern[pos] != '\\' || pattern[pos + 1] != 'E'))
            {
              if (std::strchr(regex_meta, pattern[pos]) != NULL)
              {
                regex.append(&pattern[loc], pos - loc).push_back('\\');
                loc = pos;
              }
              ++pos;
            }
            if (pos >= len || pattern[pos] != '\\')
              throw regex_error(regex_error::mismatched_quotation, pattern, k);
            if (k < pos)
              beg = false;
            regex.append(&pattern[loc], pos - loc);
            loc = pos + 2;
          }
          else
          {
            // retain regex up to and including the closing \E
            size_t k = ++pos;
            while (pos + 1 < len && (pattern[pos] != '\\' || pattern[pos + 1] != 'E'))
              ++pos;
            if (pos >= len || pattern[pos] != '\\')
              throw regex_error(regex_error::mismatched_quotation, pattern, k);
            if (k < pos)
              beg = false;
          }
          ++pos;
        }
        else if (c == 'R')
        {
	  // translate \R to match Unicode line break U+000D U+000A | [U+000A - U+000D] | U+0085 | U+2028 | U+2029
	  regex.append(&pattern[loc], pos - loc - 1).append(par).append("\\r\\n|[\\x0a-\\x0d]|\\xc2\\x85|\\xe2\\x80[\\xa8\\xa9]").append(")");
	  loc = pos + 1;
          beg = false;
        }
        else if (c == 'X')
        {
	  // translate \X to match any extended UTF-8 (compact form)
	  regex.append(&pattern[loc], pos - loc - 1).append(par).append("[\\x00-\\xbf]|[\\xc0-\\xff][\\x80-\\xbf]*").append(")");
	  loc = pos + 1;
          beg = false;
        }
        else
        {
          convert_escape(pattern, len, loc, pos, flags, esc, mod, par, regex);
	  anc = (std::strchr(regex_anchors, c) != NULL);
	  beg = false;
        }
        break;
      case '/':
        if ((flags & convert_flag::lex))
        {
          if (beg)
            throw regex_error(regex_error::empty_expression, pattern, pos);
          // lex: translate lookahead (trailing context) / to (?=
          if (!supports_modifier(mod, '='))
            throw regex_error(regex_error::invalid_modifier, pattern, pos);
          regex.append(&pattern[loc], pos - loc).append("(?=");
          lap = lev + 1;
          loc = pos + 1;
        }
	anc = false;
        beg = true;
        break;
      case '(':
        ++lev;
        if (pos + 1 < len && pattern[pos + 1] == '?')
        {
          ++pos;
          if (pos + 1 < len)
          {
            ++pos;
            if (pattern[pos] == '#')
            {
              size_t k = pos++;
              while (pos < len && pattern[pos] != ')')
                ++pos;
              if (pos >= len || pattern[pos] != ')')
                throw regex_error(regex_error::mismatched_parens, pattern, k);
              if (!supports_modifier(mod, '#'))
              {
                // no # modifier: remove (?#...)
                regex.append(&pattern[loc], k - loc - 2);
                loc = pos + 1;
              }
              --lev;
            }
            else
            {
              std::string mods;
              size_t k = pos;
              while (k < len && std::isalpha(pattern[k]))
              {
                if (enable_modifier(pattern[k], pattern, k, mod, lev + 1))
                  mods.push_back(pattern[k]);
                ++k;
              }
              if (k >= len)
                throw regex_error(regex_error::mismatched_parens, pattern, pos);
              if (pattern[k] == ':' || pattern[k] == ')')
              {
                if (can)
                  regex.append(&pattern[loc], pos - loc).append(mods).push_back(pattern[k]);
                else if (pattern[k] == ')')
                  regex.append(&pattern[loc], pos - loc - 2);
                else
                  regex.append(&pattern[loc], pos - loc - 1);
                if (pattern[k] == ')')
                {
                  // (?imsx)...
                  for (std::map<int,size_t>::iterator i = mod.begin(); i != mod.end(); ++i)
                    if (i->second == lev + 1)
                      i->second = lev;
                  --lev;
                }
                pos = k;
                loc = pos + 1;
              }
              else if (supports_modifier(mod, pattern[pos]))
              {
                // (?=...), (?!...), (?<...) etc
                beg = true;
              }
              else
              {
                throw regex_error(regex_error::invalid_syntax, pattern, pos);
              }
            }
          }
        }
        else
        {
          beg = true;
          if ((flags & convert_flag::recap) || (flags & convert_flag::lex))
          {
            // recap: translate ( to (?:
            regex.append(&pattern[loc], pos - loc).append("(?:");
            loc = pos + 1;
          }
        }
        break;
      case ')':
        if (lev == 0)
          throw regex_error(regex_error::mismatched_parens, pattern, pos);
        if (beg)
          throw regex_error(regex_error::empty_expression, pattern, pos);
        if (lap == lev + 1)
        {
          // lex lookahead: translate ) to ))
          regex.append(&pattern[loc], pos - loc).append(")");
          loc = pos;
          lap = 0;
        }
        // terminate (?isx:...)
        for (std::map<int,size_t>::iterator i = mod.begin(); i != mod.end(); )
        {
          if (i->second == lev + 1)
            mod.erase(i++);
          else
            ++i;
        }
        --lev;
        break;
      case '|':
        if (beg)
          throw regex_error(regex_error::empty_expression, pattern, pos);
        if (lap == lev + 1)
        {
          // lex lookahead: translate | to )|
          regex.append(&pattern[loc], pos - loc).append(")");
          loc = pos;
          lap = 0;
        }
        else if ((flags & convert_flag::recap) && lev == 0)
        {
          // recap: translate x|y to (x)|(y)
          regex.append(&pattern[loc], pos - loc).append(")|(");
          loc = pos + 1;
        }
        beg = true;
        break;
      case '[':
        if (pos + 1 < len && pattern[pos + 1] == '^')
        {
          ORanges<int> ranges;
          regex.append(&pattern[loc], pos - loc);
          pos += 2;
          insert_list(pattern, len, pos, flags, ranges);
          if (is_modified(mod, 'i'))
            convert_anycase_ranges(ranges);
          ORanges<int>::const_reverse_iterator i = ranges.rbegin();
          if ((flags & convert_flag::unicode) || (i != ranges.rend() && i->second > 0x80))
          {
            // Unicode: translate [^ ] to new [ ] regex with inverted content
            ORanges<int> inverse(0x00, 0x10FFFF);
            inverse -= ranges;
            regex.append(convert_unicode_ranges(inverse, esc, par));
          }
          else
          {
            // ASCII: translate [^ ] to new [^ ] regex
            regex.append("[^").append(convert_posix_ranges(ranges, esc, par));
          }
          loc = pos + 1;
        }
        else
        {
          ORanges<int> ranges;
          regex.append(&pattern[loc], pos - loc);
          ++pos;
          insert_list(pattern, len, pos, flags, ranges);
          if (is_modified(mod, 'i'))
            convert_anycase_ranges(ranges);
          ORanges<int>::const_reverse_iterator i = ranges.rbegin();
          if (i != ranges.rend() && i->second > 0x80)
          {
            // Unicode: translate [ ] to new regex
            regex.append(convert_unicode_ranges(ranges, esc, par));
          }
          else
          {
            // ASCII: translate [ ] to new regex
            regex.append("[").append(convert_posix_ranges(ranges, esc, par));
          }
          loc = pos + 1;
        }
	anc = false;
        beg = false;
        break;
      case '"':
        if ((flags & convert_flag::lex))
        {
          // lex: translate "..."
          if (!supports_escape(esc, 'Q'))
          {
            // \Q is not a supported escape, translate "..." by grouping and escaping meta chars while removing \ from \"
            regex.append(&pattern[loc], pos - loc).append(par);
            size_t k = loc = ++pos;
            while (pos < len && pattern[pos] != '"')
            {
              if (pattern[pos] == '\\' && pos + 1 < len && pattern[pos + 1] == '"')
              {
                regex.append(&pattern[loc], pos - loc);
                loc = ++pos;
              }
              else if (std::strchr(regex_meta, pattern[pos]) != NULL)
              {
                regex.append(&pattern[loc], pos - loc).push_back('\\');
                loc = pos;
              }
              ++pos;
            }
            regex.append(&pattern[loc], pos - loc).append(")");
            if (k < pos)
              beg = false;
          }
          else
          {
            // translate "..." to \Q...\E while removing \ from \" and translating \E to \E\\E\Q
            regex.append(&pattern[loc], pos - loc).append(par).append("\\Q");
            size_t k = loc = ++pos;
            while (pos < len && pattern[pos] != '"')
            {
              if (pattern[pos] == '\\' && pos + 1 < len)
              {
                if (pattern[pos + 1] == '"')
                {
                  regex.append(&pattern[loc], pos - loc);
                  loc = ++pos;
                }
                else if (pattern[pos + 1] == 'E')
                {
                  regex.append(&pattern[loc], pos - loc).append("\\E\\\\E\\Q");
                  loc = ++pos + 1;
                }
              }
              ++pos;
            }
            regex.append(&pattern[loc], pos - loc).append("\\E").append(")");
            if (k < pos)
              beg = false;
          }
          if (pos >= len || pattern[pos] != '"')
            throw regex_error(regex_error::mismatched_quotation, pattern, loc);
          loc = pos + 1;
        }
	anc = false;
        break;
      case '{':
        if (macros != NULL && pos + 1 < len && (std::isalpha(pattern[pos + 1]) || pattern[pos + 1] == '_'))
        {
          // if macros are provided: lookup {name} and expand without converting
          regex.append(&pattern[loc], pos - loc);
          loc = pos++;
          size_t k = pos++;
          while (pos < len && (std::isalnum(pattern[pos]) || pattern[pos] == '_'))
            ++pos;
          std::string name;
          name.append(&pattern[k], pos - k);
          if (pos >= len || pattern[pos] != '}')
            throw regex_error(regex_error::undefined_name, pattern, pos);
          std::map<std::string,std::string>::const_iterator i = macros->find(name);
          if (i == macros->end())
            throw regex_error(regex_error::undefined_name, pattern, k);
          regex.append(par).append(i->second).append(")");
          loc = pos + 1;
	  anc = false;
          beg = false;
        }
        else
        {
	  if (anc)
	    throw regex_error(regex_error::invalid_syntax, pattern, pos);
          if (beg)
            throw regex_error(regex_error::empty_expression, pattern, pos);
          ++pos;
          if (pos >= len || !std::isdigit(pattern[pos]))
            throw regex_error(regex_error::invalid_repeat, pattern, pos);
          char *s;
          size_t n = static_cast<size_t>(std::strtoul(&pattern[pos], &s, 10));
          pos = s - pattern;
          if (pattern[pos] == ',')
          {
            ++pos;
            if (pattern[pos] != '}')
            {
              size_t m = static_cast<size_t>(std::strtoul(&pattern[pos], &s, 10));
              if (m < n)
                throw regex_error(regex_error::invalid_repeat, pattern, pos);
              pos = s - pattern;
            }
          }
          if (pattern[pos] != '}')
          {
            if (pos + 1 < len)
              throw regex_error(regex_error::invalid_repeat, pattern, pos);
            else
              throw regex_error(regex_error::mismatched_braces, pattern, pos);
          }
          if (pos + 1 < len && (pattern[pos + 1] == '?' || pattern[pos + 1] == '+') && !supports_escape(esc, pattern[pos + 1]))
            throw regex_error(regex_error::invalid_quantifier, pattern, pos + 1);
        }
        break;
      case '}':
        throw regex_error(regex_error::mismatched_braces, pattern, pos);
      case '#':
        if (is_modified(mod, 'x'))
        {
          if ((flags & convert_flag::lex))
          {
            // x modifier and lex: translate # to \#
            regex.append(&pattern[loc], pos - loc).append("\\#");
            loc = pos + 1;
            beg = false;
          }
          else
          {
            // x modifier: remove #...\n
            regex.append(&pattern[loc], pos - loc);
            while (pos + 1 < len && pattern[++pos] != '\n')
              continue;
            loc = pos + 1;
          }
        }
        else
        {
	  anc = false;
          beg = false;
        }
        break;
      case '.':
        if ((flags & convert_flag::unicode))
        {
          // unicode: translate . to match any UTF-8 so . matches anything (also beyond U+10FFFF)
          if (is_modified(mod, 's'))
            regex.append(&pattern[loc], pos - loc).append(par).append("[\\x00-\\xff][\\x80-\\xbf]*)");
          else
            regex.append(&pattern[loc], pos - loc).append(par).append(".[\\x80-\\xbf]*)");
          loc = pos + 1;
        }
        else if (is_modified(mod, 's'))
        {
          // dotall: translate . to [\x00-\xff]
          regex.append(&pattern[loc], pos - loc).append("[\\x00-\\xff]");
          loc = pos + 1;
        }
	anc = false;
        beg = false;
        break;
      case '*':
      case '+':
      case '?':
        if (anc)
          throw regex_error(regex_error::invalid_syntax, pattern, pos);
        if (beg)
          throw regex_error(regex_error::empty_expression, pattern, pos);
        if (pos + 1 < len && (pattern[pos + 1] == '?' || pattern[pos + 1] == '+') && !supports_escape(esc, pattern[pos + 1]))
          throw regex_error(regex_error::invalid_quantifier, pattern, pos + 1);
        break;
      case '\t':
      case '\n':
      case '\r':
      case ' ':
        if (is_modified(mod, 'x'))
        {
          regex.append(&pattern[loc], pos - loc);
          loc = pos + 1;
        }
        else
        {
	  anc = false;
          beg = false;
        }
        break;
      case '^':
        if (!beg || !supports_modifier(mod, 'm'))
          throw regex_error(regex_error::invalid_anchor, pattern, pos);
	anc = true;
        break;
      case '$':
        if (!supports_modifier(mod, 'm'))
          throw regex_error(regex_error::invalid_anchor, pattern, pos);
        if (beg && (flags & convert_flag::lex))
	  throw regex_error(regex_error::empty_expression, pattern, pos);
	anc = true;
        break;
      default:
        if (std::isalpha(pattern[pos]))
        {
          if (is_modified(mod, 'i'))
          {
            // anycase: translate A to [Aa]
            regex.append(&pattern[loc], pos - loc).append("[");
            regex.push_back(c);
            regex.push_back(c ^ 0x20);
            regex.append("]");
            loc = pos + 1;
          }
        }
        else if ((c & 0xC0) == 0xC0)
        {
          // unicode: group UTF-8 sequence
          regex.append(&pattern[loc], pos - loc).append(par).push_back(c);
          while (((c = pattern[++pos]) & 0xC0) == 0x80)
            regex.push_back(c);
          regex.append(")");
          loc = pos;
          --pos;
        }
	anc = false;
        beg = false;
	break;
    }
    ++pos;
  }
  if (lev > 0)
    throw regex_error(regex_error::mismatched_parens, pattern, pos);
  if (beg && (flags & convert_flag::lex))
    throw regex_error(regex_error::empty_expression, pattern, pos);
  regex.append(&pattern[loc], pos - loc);
  if (lap > 0)
    regex.append(")");
  if ((flags & convert_flag::recap))
    regex.append(")");
  return regex;
}

} // namespace reflex
