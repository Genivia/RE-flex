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
@brief     RE/Flex input character sequence class
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2016, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#include "input.h"

namespace reflex {

void Input::file_init(void)
{
#if !defined(HAVE_CONFIG_H) || defined(HAVE_FSTAT)
  struct stat st;
  if (
#if defined(WIN32)
      ::_fstat(::_fileno(file_), &st) == 0
#else
      ::fstat(::fileno(file_), &st) == 0
#endif
      && st.st_size <= 4294967295LL)
    size_ = static_cast<size_t>(st.st_size);
#endif
  utfx_ = Const::plain;
  if (size_ > 3)
  {
    std::fread(utf8_, 1, 2, file_);
    utf8_[2] = '\0';
    if (utf8_[0] == '\0' && utf8_[1] == '\0')  // UTF-32 big endian BOM?
    {
      std::fread(utf8_ + 2, 1, 2, file_);
      utf8_[4] = '\0';
      if (utf8_[2] == '\xFE' && utf8_[3] == '\xFF')
      {
	utfx_ = Const::utf32be;
	size_ = 0;
      }
      else
      {
	uidx_ = 0;
      }
    }
    else if (utf8_[0] == '\xFE' && utf8_[1] == '\xFF') // UTF-16 big endian BOM?
    {
      utfx_ = Const::utf16be;
      size_ = 0;
    }
    else if (utf8_[0] == '\xFF' && utf8_[1] == '\xFE') // UTF-16 little endian BOM?
    {
      std::fread(utf8_ + 2, 1, 2, file_);
      utf8_[4] = '\0';
      if (utf8_[2] == '\0' && utf8_[3] == '\0') // UTF-32 little endian BOM?
      {
	utfx_ = Const::utf32le;
	size_ = 0;
      }
      else
      {
	utfx_ = Const::utf16le;
	size_ = 2;
	uidx_ = 2;
      }
    }
    else if (utf8_[0] == '\xEF' && utf8_[1] == '\xBB') // UTF-8 BOM?
    {
      std::fread(utf8_ + 2, 1, 1, file_);
      utf8_[3] = '\0';
      if (utf8_[2] == '\xBF')
	size_ -= 3;
      else
	uidx_ = 0;
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
    char *t = ::stpncpy(s, utf8_ + uidx_, n);
    k = t - s;
    s = t;
    n -= k;
    uidx_ += k;
    if (n == 0)
      return k;
    uidx_ = sizeof(utf8_);
  }
  else if (utfx_)
  {
    if (utfx_ == Const::utf16be)
    {
      unsigned char c2[2];
      size_t k = n;
      while (k > 0 && std::fread(c2, 2, 1, file_) == 1)
      {
	wchar_t c = static_cast<unsigned>(c2[0] << 8 | c2[1]);
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
	    ::memcpy(s, utf8_, k);
	    utf8_[l] = '\0';
	    uidx_ = static_cast<unsigned short>(k);
	    s += k;
	    k = 0;
	  }
	  else
	  {
	    ::memcpy(s, utf8_, l);
	    s += l;
	    k -= l;
	  }
	}
      }
      return n - k;
    }
    if (utfx_ == Const::utf16le)
    {
      unsigned char c2[2];
      size_t k = n;
      while (k > 0 && std::fread(c2, 2, 1, file_) == 1)
      {
	wchar_t c = static_cast<unsigned>(c2[0] | c2[1] << 8);
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
	    utf8_[l] = '\0';
	    uidx_ = static_cast<unsigned short>(k);
	    ::memcpy(s, utf8_, k);
	    s += k;
	    k = 0;
	  }
	  else
	  {
	    ::memcpy(s, utf8_, l);
	    s += l;
	    k -= l;
	  }
	}
      }
      return n - k;
    }
    if (utfx_ == Const::utf32be)
    {
      unsigned char c4[4];
      size_t k = n;
      while (k > 0 && std::fread(c4, 4, 1, file_) == 1)
      {
	wchar_t c = static_cast<unsigned>(c4[0] << 24 | c4[1] << 16 | c4[2] << 8 | c4[3]);
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
	    utf8_[l] = '\0';
	    uidx_ = static_cast<unsigned short>(k);
	    ::memcpy(s, utf8_, k);
	    s += k;
	    k = 0;
	  }
	  else
	  {
	    ::memcpy(s, utf8_, l);
	    s += l;
	    k -= l;
	  }
	}
      }
      return n - k;
    }
    if (utfx_ == Const::utf16le)
    {
      unsigned char c4[4];
      size_t k = n;
      while (k > 0 && std::fread(c4, 4, 1, file_) == 1)
      {
	wchar_t c = static_cast<unsigned>(c4[0] | c4[1] << 8 | c4[2] << 16 | c4[3] << 24);
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
	    utf8_[l] = '\0';
	    uidx_ = static_cast<unsigned short>(k);
	    ::memcpy(s, utf8_, k);
	    s += k;
	    k = 0;
	  }
	  else
	  {
	    ::memcpy(s, utf8_, l);
	    s += l;
	    k -= l;
	  }
	}
      }
      return n - k;
    }
  }
  if (n == 1)
  {
    int c = std::fgetc(file_);
    *s = c;
    return k + (c != EOF);
  }
  return k + std::fread(s, 1, n, file_);
}

void Input::file_size(void)
{
  if (size_ == 0)
  {
    off_t k = ftello(file_);
    if (k >= 0)
    {
      if (utfx_ == Const::plain)
      {
	fseeko(file_, k, SEEK_END);
        off_t n = ftello(file_);
	if (n >= 0)
	  size_ = static_cast<size_t>(n);
      }
      else if (utfx_ == Const::utf16be)
      {
	unsigned char c2[2];
	while (std::fread(c2, 2, 1, file_) == 1)
	{
	  wchar_t c = static_cast<unsigned>(c2[0] << 8 | c2[1]);
	  size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
	}
      }
      else if (utfx_ == Const::utf32le)
      {
	unsigned char c2[2];
	while (std::fread(c2, 2, 1, file_) == 1)
	{
	  wchar_t c = static_cast<unsigned>(c2[0] | c2[1] << 8);
	  size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
	}
      }
      else if (utfx_ == Const::utf32be)
      {
	unsigned char c4[4];
	while (std::fread(c4, 4, 1, file_) == 1)
	{
	  wchar_t c = static_cast<unsigned>(c4[0] << 24 | c4[1] << 16 | c4[2] << 8 | c4[3]);
	  size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
	}
      }
      else if (utfx_ == Const::utf32le)
      {
	unsigned char c4[4];
	while (std::fread(c4, 4, 1, file_) == 1)
	{
	  wchar_t c = static_cast<unsigned>(c4[0] | c4[1] << 8 | c4[2] << 16 | c4[3] << 24);
	  size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
	}
      }
      std::clearerr(file_);
      fseeko(file_, k, SEEK_SET);
    }
    std::clearerr(file_);
  }
}

} // namespace reflex
