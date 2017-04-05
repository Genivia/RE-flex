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
@file      input.cpp
@brief     RE/flex input character sequence class
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2017, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#include <reflex/input.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__BORLANDC__)
# define off_t __int64
# define ftello _ftelli64
# define fseeko _fseeki64
#else
# include <unistd.h> // fstat()
#endif

namespace reflex {

// EBCDIC to ISO-8859-1 (ASCII+)
static unsigned char e2a[256] = {
    0,  1,  2,  3,156,  9,134,127,151,141,142, 11, 12, 13, 14, 15,
   16, 17, 18, 19,157,133,  8,135, 24, 25,146,143, 28, 29, 30, 31,
  128,129,130,131,132, 10, 23, 27,136,137,138,139,140,  5,  6,  7,
  144,145, 22,147,148,149,150,  4,152,153,154,155, 20, 21,158, 26,
   32,160,161,162,163,164,165,166,167,168, 91, 46, 60, 40, 43, 33,
   38,169,170,171,172,173,174,175,176,177, 93, 36, 42, 41, 59, 94,
   45, 47,178,179,180,181,182,183,184,185,124, 44, 37, 95, 62, 63,
  186,187,188,189,190,191,192,193,194, 96, 58, 35, 64, 39, 61, 34,
  195, 97, 98, 99,100,101,102,103,104,105,196,197,198,199,200,201,
  202,106,107,108,109,110,111,112,113,114,203,204,205,206,207,208,
  209,126,115,116,117,118,119,120,121,122,210,211,212,213,214,215,
  216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,
  123, 65, 66, 67, 68, 69, 70, 71, 72, 73,232,233,234,235,236,237,
  125, 74, 75, 76, 77, 78, 79, 80, 81, 82,238,239,240,241,242,243,
   92,159, 83, 84, 85, 86, 87, 88, 89, 90,244,245,246,247,248,249,
   48, 49, 50, 51, 52, 53, 54, 55, 56, 57,250,251,252,253,254,255
};

void Input::file_init(void)
{
  // attempt to determine the file size with fstat()
#if !defined(HAVE_CONFIG_H) || defined(HAVE_FSTAT)
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__BORLANDC__)
  struct _stat st;
  if (_fstat(_fileno(file_), &st) == 0 && st.st_size <= 4294967295LL)
#else
  struct stat st;
  if (::fstat(::fileno(file_), &st) == 0 && st.st_size <= 4294967295LL)
#endif
    size_ = static_cast<size_t>(st.st_size);
#endif
  // assume plain (ASCII, binary or UTF-8 without BOM) content by default
  utfx_ = file_encoding::plain;
  // check first UTF BOM byte
  if (::fread(utf8_, 1, 1, file_) == 1)
  {
    utf8_[1] = '\0';
    uidx_ = 0;
    if (utf8_[0] == '\0' || utf8_[0] == '\xef' || utf8_[0] == '\xfe' || utf8_[0] == '\xff')
    {
      // check second UTF BOM byte
      if (::fread(utf8_ + 1, 1, 1, file_) == 1)
      {
        utf8_[2] = '\0';
        if (utf8_[0] == '\0' && utf8_[1] == '\0')  // UTF-32 big endian BOM 0000XXXX?
        {
          if (::fread(utf8_ + 2, 1, 2, file_) == 2)
          {
            utf8_[4] = '\0';
            if (utf8_[2] == '\xfe' && utf8_[3] == '\xff') // UTF-32 big endian BOM 0000FEFF?
            {
              size_ = 0;
              uidx_ = sizeof(utf8_);
              utfx_ = file_encoding::utf32be;
            }
          }
        }
        else if (utf8_[0] == '\xfe' && utf8_[1] == '\xff') // UTF-16 big endian BOM FEFF?
        {
          size_ = 0;
          uidx_ = sizeof(utf8_);
          utfx_ = file_encoding::utf16be;
        }
        else if (utf8_[0] == '\xff' && utf8_[1] == '\xfe') // UTF-16 or UTF-32 little endian BOM FFFEXXXX?
        {
          if (::fread(utf8_ + 2, 1, 2, file_) == 2)
          {
            utf8_[4] = '\0';
            if (utf8_[2] == '\0' && utf8_[3] == '\0') // UTF-32 little endian BOM FFFE0000?
            {
              size_ = 0;
              uidx_ = sizeof(utf8_);
              utfx_ = file_encoding::utf32le;
            }
            else
            {
              size_ = 0;
              utf8_[utf8(utf8_[2] | utf8_[3] << 8, utf8_)] = '\0';
              uidx_ = 0;
              utfx_ = file_encoding::utf16le;
            }
          }
        }
        else if (utf8_[0] == '\xef' && utf8_[1] == '\xbb') // UTF-8 BOM EFBBXX?
        {
          if (::fread(utf8_ + 2, 1, 1, file_) == 1)
          {
            utf8_[3] = '\0';
            if (utf8_[2] == '\xbf') // UTF-8 BOM EFBBBF?
            {
              size_ -= 3;
              uidx_ = sizeof(utf8_);
              utfx_ = file_encoding::utf8;
            }
          }
        }
      }
    }
  }
}

size_t Input::file_get(char *s, size_t n)
{
  char *t = s;
  if (uidx_ < sizeof(utf8_))
  {
    unsigned short k = 0;
    while (k < n && utf8_[uidx_ + k] != '\0')
      *t++ = utf8_[uidx_ + k++];
    n -= k;
    if (n == 0)
    {
      uidx_ += k;
      return k;
    }
    uidx_ = sizeof(utf8_);
  }
  unsigned char buf[4];
  switch (utfx_)
  {
    case file_encoding::latin:
      while (n > 0 && ::fread(t, 1, 1, file_) == 1)
      {
        int c = static_cast<unsigned char>(*t);
        if (c < 0x80)
        {
          t++;
          --n;
        }
        else
        {
          utf8(c, utf8_);
          *t++ = utf8_[0];
          --n;
          if (n > 0)
          {
            *t++ = utf8_[1];
            --n;
          }
          else
          {
            uidx_ = 1;
            utf8_[2] = '\0';
          }
        }
      }
      return t - s;
    case file_encoding::ebcdic:
      while (n > 0 && ::fread(t, 1, 1, file_) == 1)
      {
        int c = e2a[static_cast<unsigned char>(*t)];
        if (c < 0x80)
        {
          *t++ = static_cast<char>(c);
          --n;
        }
        else
        {
          utf8(c, utf8_);
          *t++ = utf8_[0];
          --n;
          if (n > 0)
          {
            *t++ = utf8_[1];
            --n;
          }
          else
          {
            uidx_ = 1;
            utf8_[2] = '\0';
          }
        }
      }
      return t - s;
    case file_encoding::utf16be:
      while (n > 0 && ::fread(buf, 2, 1, file_) == 1)
      {
        int c = buf[0] << 8 | buf[1];
        if (c < 0x80)
        {
          *t++ = static_cast<char>(c);
          --n;
        }
        else
        {
          if (c >= 0xD800 && c < 0xE000)
          {
            // UTF-16 surrogate pair
            if (c < 0xDC00 && ::fread(buf + 2, 2, 1, file_) == 1 && (buf[2] & 0xFC) == 0xDC)
              c = 0x010000 - 0xDC00 + ((c - 0xD800) << 10) + (buf[2] << 8 | buf[3]);
            else
              c = REFLEX_NONCHAR;
          }
          size_t l = utf8(c, utf8_);
          if (n < l)
          {
            std::memcpy(t, utf8_, n);
            utf8_[l] = '\0';
            uidx_ = static_cast<unsigned short>(n);
            t += n;
            n = 0;
          }
          else
          {
            std::memcpy(t, utf8_, l);
            t += l;
            n -= l;
          }
        }
      }
      return t - s;
    case file_encoding::utf16le:
      while (n > 0 && ::fread(buf, 2, 1, file_) == 1)
      {
        int c = buf[0] | buf[1] << 8;
        if (c < 0x80)
        {
          *t++ = static_cast<char>(c);
          --n;
        }
        else
        {
          if (c >= 0xD800 && c < 0xE000)
          {
            // UTF-16 surrogate pair
            if (c < 0xDC00 && ::fread(buf + 2, 2, 1, file_) == 1 && (buf[3] & 0xFC) == 0xDC)
              c = 0x010000 - 0xDC00 + ((c - 0xD800) << 10) + (buf[2] | buf[3] << 8);
            else
              c = REFLEX_NONCHAR;
          }
          size_t l = utf8(c, utf8_);
          if (n < l)
          {
            std::memcpy(t, utf8_, n);
            utf8_[l] = '\0';
            uidx_ = static_cast<unsigned short>(n);
            t += n;
            n = 0;
          }
          else
          {
            std::memcpy(t, utf8_, l);
            t += l;
            n -= l;
          }
        }
      }
      return t - s;
    case file_encoding::utf32be:
      while (n > 0 && ::fread(buf, 4, 1, file_) == 1)
      {
        int c = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
        if (c < 0x80)
        {
          *t++ = static_cast<char>(c);
          --n;
        }
        else
        {
          size_t l = utf8(c, utf8_);
          if (n < l)
          {
            std::memcpy(t, utf8_, n);
            utf8_[l] = '\0';
            uidx_ = static_cast<unsigned short>(n);
            t += n;
            n = 0;
          }
          else
          {
            std::memcpy(t, utf8_, l);
            t += l;
            n -= l;
          }
        }
      }
      return t - s;
    case file_encoding::utf32le:
      while (n > 0 && ::fread(buf, 4, 1, file_) == 1)
      {
        int c = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
        if (c < 0x80)
        {
          *t++ = static_cast<char>(c);
          --n;
        }
        else
        {
          size_t l = utf8(c, utf8_);
          if (n < l)
          {
            std::memcpy(t, utf8_, n);
            utf8_[l] = '\0';
            uidx_ = static_cast<unsigned short>(n);
            t += n;
            n = 0;
          }
          else
          {
            std::memcpy(t, utf8_, l);
            t += l;
            n -= l;
          }
        }
      }
      return t - s;
    default:
      return t - s + ::fread(t, 1, n, file_);
  }
}

void Input::file_size(void)
{
  if (size_ == 0)
  {
    off_t k = ftello(file_);
    if (k >= 0)
    {
      unsigned char buf[4];
      switch (utfx_)
      {
        case file_encoding::latin:
          while (::fread(buf, 1, 1, file_) == 1)
            size_ += 1 + (buf[0] >= 0x80);
          break;
        case file_encoding::ebcdic:
          while (::fread(buf, 1, 1, file_) == 1)
            size_ += 1 + (e2a[buf[0]] >= 0x80);
          break;
        case file_encoding::utf16be:
          while (::fread(buf, 2, 1, file_) == 1)
          {
            int c = buf[0] << 8 | buf[1];
            if (c >= 0xD800 && c < 0xE000)
            {
              // UTF-16 surrogate pair
              if (c < 0xDC00 && ::fread(buf + 2, 2, 1, file_) == 1 && (buf[2] & 0xFC) == 0xDC)
                c = 0x010000 - 0xDC00 + ((c - 0xD800) << 10) + (buf[2] << 8 | buf[3]);
              else
                c = REFLEX_NONCHAR;
            }
#ifndef WITH_UTF8_UNRESTRICTED
            else if (c > 0x10FFFF)
            {
              c = REFLEX_NONCHAR;
            }
#endif
            size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
          }
          break;
        case file_encoding::utf16le:
          while (::fread(buf, 2, 1, file_) == 1)
          {
            int c = buf[0] | buf[1] << 8;
            if (c >= 0xD800 && c < 0xE000)
            {
              // UTF-16 surrogate pair
              if (c < 0xDC00 && ::fread(buf + 2, 2, 1, file_) == 1 && (buf[2] & 0xFC) == 0xDC)
                c = 0x010000 - 0xDC00 + ((c - 0xD800) << 10) + (buf[2] << 8 | buf[3]);
              else
                c = REFLEX_NONCHAR;
            }
#ifndef WITH_UTF8_UNRESTRICTED
            else if (c > 0x10FFFF)
            {
              c = REFLEX_NONCHAR;
            }
#endif
            size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
          }
          break;
        case file_encoding::utf32be:
          while (::fread(buf, 4, 1, file_) == 1)
          {
            int c = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
#ifndef WITH_UTF8_UNRESTRICTED
            if (c > 0x10FFFF)
              c = REFLEX_NONCHAR;
#endif
            size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
          }
          break;
        case file_encoding::utf32le:
          while (::fread(buf, 4, 1, file_) == 1)
          {
            int c = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
#ifndef WITH_UTF8_UNRESTRICTED
            if (c > 0x10FFFF)
              c = REFLEX_NONCHAR;
#endif
            size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
          }
          break;
        default:
          fseeko(file_, k, SEEK_END);
          off_t n = ftello(file_);
          if (n >= k)
            size_ = static_cast<size_t>(n - k);
      }
      ::clearerr(file_);
      fseeko(file_, k, SEEK_SET);
    }
    ::clearerr(file_);
  }
}

void Input::file_encoding(unsigned short enc)
{
  if (file_ && utfx_ != enc)
  {
    if (utfx_ == file_encoding::plain && uidx_ < sizeof(utf8_))
    {
      // translate (non-BOM) plain bytes (1 to 4 bytes) buffered in utf8_[]
      unsigned char buf[sizeof(utf8_)];
      std::memcpy(buf, utf8_, sizeof(utf8_));
      unsigned char *b = buf;
      char *t = utf8_;
      int c1, c2;
      switch (enc)
      {
        case file_encoding::latin:
          for (unsigned short i = 0; *b != '\0'; ++i)
          {
            c1 = *b++;
            if (c1 < 0x80)
              *t++ = static_cast<char>(c1);
            else
              t += utf8(c1, t);
          }
          *t = '\0';
          uidx_ = 0;
          break;
        case file_encoding::ebcdic:
          for (unsigned short i = 0; *b != '\0'; ++i)
          {
            c1 = e2a[*b++];
            if (c1 < 0x80)
              *t++ = static_cast<char>(c1);
            else
              t += utf8(c1, t);
          }
          *t = '\0';
          uidx_ = 0;
          break;
        case file_encoding::utf16be:
          // enforcing non-BOM UTF-16: translate utf8_[] to UTF-16 then to UTF-8
          if (b[1] == '\0')
            ::fread(b + 1, 1, 1, file_);
          if (b[2] == '\0')
            ::fread(b + 2, 1, 2, file_);
          else if (b[3] == '\0')
            ::fread(b + 3, 1, 1, file_);
          c1 = b[0] << 8 | b[1];
          c2 = b[2] << 8 | b[3];
          if (c1 >= 0xD800 && c1 < 0xE000)
          {
            // UTF-16 surrogate pair
            if (c1 < 0xDC00 && (c2 & 0xFC00) == 0xDC00)
              c1 = 0x010000 - 0xDC00 + ((c1 - 0xD800) << 10) + c2;
            else
              c1 = REFLEX_NONCHAR;
            t += utf8(c1, t);
          }
          else
          {
            t += utf8(c1, t);
            t += utf8(c2, t);
          }
          *t = '\0';
          uidx_ = 0;
          break;
        case file_encoding::utf16le:
          // enforcing non-BOM UTF-16: translate utf8_[] to UTF-16 then to UTF-8
          if (b[1] == '\0')
            ::fread(b + 1, 1, 1, file_);
          if (b[2] == '\0')
            ::fread(b + 2, 1, 2, file_);
          else if (b[3] == '\0')
            ::fread(b + 3, 1, 1, file_);
          c1 = b[0] | b[1] << 8;
          c2 = b[2] | b[3] << 8;
          if (c1 >= 0xD800 && c1 < 0xE000)
          {
            // UTF-16 surrogate pair
            if (c1 < 0xDC00 && (c2 & 0xFC00) == 0xDC00)
              c1 = 0x010000 - 0xDC00 + ((c1 - 0xD800) << 10) + c2;
            else
              c1 = REFLEX_NONCHAR;
            t += utf8(c1, t);
          }
          else
          {
            t += utf8(c1, t);
            t += utf8(c2, t);
          }
          *t = '\0';
          uidx_ = 0;
          break;
        case file_encoding::utf32be:
          // enforcing non-BOM UTF-32: translate utf8_[] to UTF-32 then to UTF-8
          if (b[1] == '\0')
            ::fread(b + 1, 1, 3, file_);
          else if (b[2] == '\0')
            ::fread(b + 2, 1, 2, file_);
          else if (b[3] == '\0')
            ::fread(b + 3, 1, 1, file_);
          c1 = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
          t += utf8(c1, t);
          *t = '\0';
          uidx_ = 0;
          break;
        case file_encoding::utf32le:
          // enforcing non-BOM UTF-32: translate utf8_[] to UTF-32 then to UTF-8
          if (b[1] == '\0')
            ::fread(b + 1, 1, 3, file_);
          else if (b[2] == '\0')
            ::fread(b + 2, 1, 2, file_);
          else if (b[3] == '\0')
            ::fread(b + 3, 1, 1, file_);
          c1 = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
          t += utf8(c1, t);
          *t = '\0';
          uidx_ = 0;
          break;
        default:
          break;
      }
    }
    size_ = 0;
    utfx_ = enc;
  }
}

} // namespace reflex
