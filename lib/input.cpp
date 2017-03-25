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

void Input::file_init(void)
{
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
  // assume plain (ASCII or UTF-8 without BOM) content by default
  utfx_ = file_encoding::plain;
  // if file size could be determined, then check for a UTF BOM in the file
  if (size_ > 3)
  {
    ::fread(utf8_, 1, 2, file_);
    utf8_[2] = '\0';
    if (utf8_[0] == '\0' && utf8_[1] == '\0')  // UTF-32 big endian BOM?
    {
      ::fread(utf8_ + 2, 1, 2, file_);
      utf8_[4] = '\0';
      if (utf8_[2] == '\xFE' && utf8_[3] == '\xFF')
      {
        utfx_ = file_encoding::utf32be;
        size_ = 0;
      }
      else
      {
        uidx_ = 0;
      }
    }
    else if (utf8_[0] == '\xFE' && utf8_[1] == '\xFF') // UTF-16 big endian BOM?
    {
      utfx_ = file_encoding::utf16be;
      size_ = 0;
    }
    else if (utf8_[0] == '\xFF' && utf8_[1] == '\xFE') // UTF-16 little endian BOM?
    {
      ::fread(utf8_ + 2, 1, 2, file_);
      utf8_[4] = '\0';
      if (utf8_[2] == '\0' && utf8_[3] == '\0') // UTF-32 little endian BOM?
      {
        utfx_ = file_encoding::utf32le;
        size_ = 0;
      }
      else
      {
        utfx_ = file_encoding::utf16le;
        wchar_t c = static_cast<unsigned int>(utf8_[2] | utf8_[3] << 8);
        size_ = utf8(c, utf8_);
        utf8_[size_] = '\0';
        uidx_ = 0;
      }
    }
    else if (utf8_[0] == '\xEF' && utf8_[1] == '\xBB') // UTF-8 BOM?
    {
      ::fread(utf8_ + 2, 1, 1, file_);
      utf8_[3] = '\0';
      if (utf8_[2] == '\xBF')
      {
        utfx_ = file_encoding::utf8;
        size_ -= 3;
      }
      else
      {
        uidx_ = 0;
      }
    }
    else
    {
      uidx_ = 0;
    }
  }
}

size_t Input::file_get(char *s, size_t n)
{
  size_t k = 0;
  if (uidx_ < sizeof(utf8_))
  {
    while (k < n && utf8_[uidx_ + k] != '\0')
      *s++ = utf8_[uidx_ + k++];
    n -= k;
    uidx_ += static_cast<unsigned short>(k);
    if (n == 0)
      return k;
    uidx_ = sizeof(utf8_);
  }
  unsigned char buf[4];
  switch (utfx_)
  {
    case file_encoding::latin:
      k = n;
      while (k > 0 && ::fread(s, 1, 1, file_) == 1)
      {
	int c = static_cast<unsigned char>(*s);
	if (c < 0x80)
        {
          s++;
          --k;
        }
        else
        {
          utf8(c, utf8_);
          if (k < 2)
          {
            *s++ = utf8_[0];
            utf8_[2] = '\0';
            uidx_ = 1;
            k = 0;
          }
          else
          {
            *s++ = utf8_[0];
            *s++ = utf8_[1];
            k -= 2;
          }
        }
      }
      return n - k;
    case file_encoding::utf16be:
      k = n;
      while (k > 0 && ::fread(buf, 2, 1, file_) == 1)
      {
        int c = buf[0] << 8 | buf[1];
        if (c < 0x80)
        {
          *s++ = static_cast<char>(c);
          --k;
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
          if (k < l)
          {
            std::memcpy(s, utf8_, k);
            utf8_[l] = '\0';
            uidx_ = static_cast<unsigned short>(k);
            s += k;
            k = 0;
          }
          else
          {
            std::memcpy(s, utf8_, l);
            s += l;
            k -= l;
          }
        }
      }
      return n - k;
    case file_encoding::utf16le:
      k = n;
      while (k > 0 && ::fread(buf, 2, 1, file_) == 1)
      {
        int c = buf[0] | buf[1] << 8;
        if (c < 0x80)
        {
          *s++ = static_cast<char>(c);
          --k;
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
          if (k < l)
          {
            std::memcpy(s, utf8_, k);
            utf8_[l] = '\0';
            uidx_ = static_cast<unsigned short>(k);
            s += k;
            k = 0;
          }
          else
          {
            std::memcpy(s, utf8_, l);
            s += l;
            k -= l;
          }
        }
      }
      return n - k;
    case file_encoding::utf32be:
      k = n;
      while (k > 0 && ::fread(buf, 4, 1, file_) == 1)
      {
        int c = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
        if (c < 0x80)
        {
          *s++ = static_cast<char>(c);
          --k;
        }
        else
        {
          size_t l = utf8(c, utf8_);
          if (k < l)
          {
            std::memcpy(s, utf8_, k);
            utf8_[l] = '\0';
            uidx_ = static_cast<unsigned short>(k);
            s += k;
            k = 0;
          }
          else
          {
            std::memcpy(s, utf8_, l);
            s += l;
            k -= l;
          }
        }
      }
      return n - k;
    case file_encoding::utf32le:
      k = n;
      while (k > 0 && ::fread(buf, 4, 1, file_) == 1)
      {
        int c = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
        if (c < 0x80)
        {
          *s++ = static_cast<char>(c);
          --k;
        }
        else
        {
          size_t l = utf8(c, utf8_);
          if (k < l)
          {
            std::memcpy(s, utf8_, k);
            utf8_[l] = '\0';
            uidx_ = static_cast<unsigned short>(k);
            s += k;
            k = 0;
          }
          else
          {
            std::memcpy(s, utf8_, l);
            s += l;
            k -= l;
          }
        }
      }
      return n - k;
    default:
      if (n == 1)
      {
	int c = ::fgetc(file_);
	*s = c;
	return k + (c != EOF);
      }
      return k + ::fread(s, 1, n, file_);
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
      if (utfx_ == file_encoding::plain)
      {
        fseeko(file_, k, SEEK_END);
        off_t n = ftello(file_);
        if (n >= k)
          size_ = static_cast<size_t>(n - k);
      }
      else if (utfx_ == file_encoding::utf16be)
      {
        while (::fread(buf, 2, 1, file_) == 1)
        {
          wchar_t c = static_cast<unsigned>(buf[0] << 8 | buf[1]);
#ifndef WITH_UTF8_UNRESTRICTED
          size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000);
#else
          size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
#endif
        }
      }
      else if (utfx_ == file_encoding::utf32le)
      {
        while (::fread(buf, 2, 1, file_) == 1)
        {
          wchar_t c = static_cast<unsigned>(buf[0] | buf[1] << 8);
#ifndef WITH_UTF8_UNRESTRICTED
          size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000);
#else
          size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
#endif
        }
      }
      else if (utfx_ == file_encoding::utf32be)
      {
        while (::fread(buf, 4, 1, file_) == 1)
        {
          wchar_t c = static_cast<unsigned>(buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]);
#ifndef WITH_UTF8_UNRESTRICTED
          size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000);
#else
          size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
#endif
        }
      }
      else if (utfx_ == file_encoding::utf32le)
      {
        while (::fread(buf, 4, 1, file_) == 1)
        {
          wchar_t c = static_cast<unsigned>(buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24);
#ifndef WITH_UTF8_UNRESTRICTED
          size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000);
#else
          size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
#endif
        }
      }
      ::clearerr(file_);
      fseeko(file_, k, SEEK_SET);
    }
    ::clearerr(file_);
  }
}

} // namespace reflex
