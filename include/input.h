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
@file      input.h
@brief     RE/flex input character sequence class
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2016, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_INPUT_H
#define REFLEX_INPUT_H

#include "utf8.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>

namespace reflex {

/// Input character sequence class for unified access to sources of input.
/**
Description
-----------

The Input class unifies access to a source of input of a character sequence as
follows:

- An Input object is instantiated and (re)assigned a (new) source input: either
  a `char*` string, a `wchar_t*` wide string, a `std::string`, a
  `std::wstring`, a `FILE*` descriptor, or a `std::istream` object.

- When assigned a wide string source as input, the wide character content is
  automatically converted to an UTF-8 character sequence when reading with
  get().

- When assigned a `FILE*` source as input, the file is checked for the presence
  of a UTF-8 or a UTF-16 BOM (Byte Order Mark). A UTF-8 BOM is ignored and will
  not appear on the input character stream (and size is adjusted by 3 bytes). A
  UTF-16 BOM is intepreted, resulting in the conversion of the file content
  automatically to an UTF-8 character sequence when reading the file with
  get(). Also, size() gives the content size in the number of UTF-8 bytes.

- An input object can be reassigned a new source of input for reading at any
  time.

- An input object obeys move semantics. That is, after assigning an input
  object to another, the former can no longer be used to read input. This
  prevents adding the overhead and complexity of file and stream duplication.

- `size_t Input::get(char *buf, size_t len);` reads source input and fills `buf`
  with up to `len` bytes, returning the number of bytes read or zero when a
  stream or file is bad or when EOF is reached.

- `size_t Input::size(void);` returns the number of ASCII/UTF-8 bytes available
  to read from the source input or zero (zero is also returned when the size is
  not determinable). Use this function only before reading input with get().
  Wide character strings and UTF-16 `FILE*` content is counted as the total
  number of UTF-8 bytes that will be produced by get(). The size of a
  `std::istream` cannot be determined.

- `bool Input::good(void);` returns true if the input is readable and a
  non-empty sequence of characters is available to get. Returns false on EOF or
  if an error condition is present.

- `bool Input::eof(void);` returns true if the input reached EOF. Note that
  good() == ! eof() for string source input only, since files and streams may
  have error conditions that prevent reading. That is, for files and streams
  eof() implies good() == false, but not vice versa. Thus, an error is
  diagnosed when the condition good() == false && eof() == false holds. Note
  that get(buf, len) == 0 && len > 0 implies good() == false.

Example
-------

The following example shows how to read a character sequence in blocks from a
`std::ifstream`:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    std::ifstream ifs;
    ifs.open("input.h", std::ifstream::in);
    reflex::Input input(ifs);
    char buf[1024];
    size_t len;
    while ((len = input.get(buf, sizeof(buf))) > 0)
      fwrite(buf, 1, len, stdout);
    if (!input.eof())
      std::cerr << "An IO error occurred" << std::endl;
    ifs.close();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Example
-------

The following example shows how to buffer the entire content of a file:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    reflex::Input input(fopen("input.h", "r"));
    if (!input.file())
      abort();
    size_t len = input.size();
    char *buf = new char[len];
    input.get(buf, len);
    if (!input.eof())
      std::cerr << "An IO error occurred" << std::endl;
    fwrite(buf, 1, len, stdout);
    delete[] buf;
    fclose(input.file());
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Files with UTF-16 content are converted to UTF-8 by get(buf, len), where size()
gives the total number of UTF-8 bytes that will be produced by get(buf, len).

Example
-------

The following example shows how to read a character sequence in blocks from a
file:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    reflex::Input input(fopen("input.h", "r"));
    char buf[1024];
    size_t len;
    while ((len = input.get(buf, sizeof(buf))) > 0)
      fwrite(buf, 1, len, stdout);
    fclose(input);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Example
-------

The following example shows how to echo characters one by one from stdin
(reading input from a tty):

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    reflex::Input input(stdin);
    char c;
    while (input.get(&c, 1))
      fputc(c, stdout);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Example
-------

The following example shows how to read a character sequence in blocks from a
wide character string while converting it to UTF-8:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    reflex::Input input(L"Copyright ©"); // © is unicode U+00A9 and UTF-8 C2 A9
    char buf[8];
    size_t len;
    while ((len = input.get(buf, sizeof(buf))) > 0)
      fwrite(buf, 1, len, stdout);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Example
-------

The following example shows how to convert a wide character string to UTF-8:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    reflex::Input input(L"Copyright ©"); // © is unicode U+00A9 and UTF-8 C2 A9
    size_t len = input.size(); // size of UTF-8 string
    char *buf = new char[len];
    input.get(buf, len);
    fwrite(buf, 1, len, stdout);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Example
-------

The following example shows how to switch source inputs while reading input
byte by byte (use a buffer as shown in other examples to improve efficiency):

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    reflex::Input input = "Hello";
    std::string message;
    char c;
    while (input.get(&c, 1))
      message.append(c);
    input = L" world! To ∞ and beyond."; // switch input to a wide string
    while (input.get(&c, 1))
      message.append(c);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*/
class Input {
 public:
  /// Common constants.
  struct Const {
    static const short plain   = 0; ///< plain ASCII/UTF-8 file_encoding
    static const short utf16be = 1; ///< UTF-16 big endian file_encoding
    static const short utf16le = 2; ///< UTF-16 little endian file_encoding
    static const short utf32be = 3; ///< UTF-32 big endian file_encoding
    static const short utf32le = 4; ///< UTF-32 little endian file_encoding
  };
  /// Copy constructor (with intended "move semantics" as internal state is shared, should not rely on using the rhs after copying).
  Input(const Input& input) ///< an Input object to share state with (undefined behavior results from using both objects at the same time)
    :
      cstring_(input.cstring_),
      wstring_(input.wstring_),
      file_(input.file_),
      istream_(input.istream_),
      size_(input.size_),
      uidx_(input.uidx_),
      utfx_(input.utfx_)
  {
    std::memcpy(utf8_, input.utf8_, sizeof(utf8_));
  }
  /// Construct empty input character sequence.
  Input(void)
    :
      cstring_(NULL),
      wstring_(NULL),
      file_(NULL),
      istream_(NULL)
  {
    init();
  }
  /// Construct input character sequence from a NUL-terminated string.
  Input(const char *cstring) ///< NUL-terminated char* string
    :
      cstring_(cstring),
      wstring_(NULL),
      file_(NULL),
      istream_(NULL)
  {
    init();
  }
  /// Construct input character sequence from a std::string.
  Input(const std::string& string) ///< input string
    :
      cstring_(string.c_str()),
      wstring_(NULL),
      file_(NULL),
      istream_(NULL)
  {
    init();
  }
  /// Construct input character sequence from a pointer to a std::string.
  Input(const std::string *string) ///< input string
    :
      cstring_(string ? string->c_str() : NULL),
      wstring_(NULL),
      file_(NULL),
      istream_(NULL)
  {
    init();
  }
  /// Construct input character sequence from a NUL-terminated wide character string.
  Input(const wchar_t *wstring) ///< NUL-terminated wchar_t* input string
    :
      cstring_(NULL),
      wstring_(wstring),
      file_(NULL),
      istream_(NULL)
  {
    init();
  }
  /// Construct input character sequence from a std::wstring.
  Input(const std::wstring& wstring) ///< input wide string
    :
      cstring_(NULL),
      wstring_(wstring.c_str()),
      file_(NULL),
      istream_(NULL)
  {
    init();
  }
  /// Construct input character sequence from a pointer to a std::wstring.
  Input(const std::wstring *wstring) ///< input wide string
    :
      cstring_(NULL),
      wstring_(wstring ? wstring->c_str() : NULL),
      file_(NULL),
      istream_(NULL)
  {
    init();
  }
  /// Construct input character sequence from an open FILE* file descriptor, supports UTF-8 conversion from UTF-16 and UTF-32, use stdin if file == NULL.
  Input(FILE *file) ///< input file
    :
      cstring_(NULL),
      wstring_(NULL),
      file_(file ? file : stdin),
      istream_(NULL)
  {
    init();
  }
  /// Construct input character sequence from a std::istream.
  Input(std::istream& istream) ///< input stream
    :
      cstring_(NULL),
      wstring_(NULL),
      file_(NULL),
      istream_(&istream)
  {
    init();
  }
  /// Construct input character sequence from a pointer to a std::istream, use stdin if istream == NULL.
  Input(std::istream *istream) ///< input stream
    :
      cstring_(NULL),
      wstring_(NULL),
      file_(istream ? NULL : stdin),
      istream_(istream)
  {
    init();
  }
  /// Cast this Input object to string.
  operator const char *()
    /// @returns remaining unbuffered part of the NUL-terminated string or NULL.
  {
    return cstring_;
  }
  /// Cast this Input object to wide character string.
  operator const wchar_t *()
    /// @returns remaining unbuffered part of the NUL-terminated wide character string or NULL.
  {
    return wstring_;
  }
  /// Cast this Input object to file descriptor FILE*.
  operator FILE *()
    /// @returns pointer to current file descriptor or NULL.
  {
    return file_;
  }
  /// Cast this Input object to std::istream*.
  operator std::istream *()
    /// @returns pointer to current std::istream or NULL.
  {
    return istream_;
  }
  /// Get the remaining string of this Input object.
  const char *cstring(void)
    /// @returns remaining unbuffered part of the NUL-terminated string or NULL.
  {
    return cstring_;
  }
  /// Get the remaining wide character string of this Input object.
  const wchar_t *wstring(void)
    /// @returns remaining unbuffered part of the NUL-terminated wide character string or NULL.
  {
    return wstring_;
  }
  /// Get the FILE* of this Input object.
  FILE *file(void)
    /// @returns pointer to current file descriptor or NULL.
  {
    return file_;
  }
  /// Get the std::istream of this Input object.
  std::istream *istream(void)
    /// @returns pointer to current std::istream or NULL.
  {
    return istream_;
  }
  /// Get the size of the input character sequence in number of ASCII/UTF-8 bytes (zero if size is not determinable from a `FILE*` or `std::istream` source).
  size_t size(void)
    /// @returns the nonzero number of ASCII/UTF-8 bytes available to read, or zero when source is empty or if size is not determinable.
    /// @warning This function SHOULD NOT be used after get().
  {
    if (size_ > 0)
      return size_;
    if (cstring_)
    {
      size_ = std::strlen(cstring_);
    }
    else if (wstring_)
    {
      wchar_t c;
      for (const wchar_t *s = wstring_; (c = *s) != L'\0'; ++s)
        size_ += 1 + (c >= 0x80) + (c >= 0x0800) + (c >= 0x010000) + (c >= 0x200000) + (c >= 0x04000000);
    }
    else if (file_)
    {
      file_size();
    }
    return size_;
  }
  /// Check if input is available.
  /// @returns true if a non-empty sequence of characters is available to get.
  bool good(void)
  {
    if (cstring_)
      return *cstring_ != '\0';
    if (wstring_)
      return *wstring_ != L'\0';
    if (file_)
      return file_good();
    if (istream_)
      return istream_->good();
    return false;
  }
  /// Check if input reached EOF.
  /// @returns true if input is at EOF and no characters are available.
  bool eof(void)
  {
    if (cstring_)
      return *cstring_ == '\0';
    if (wstring_)
      return *wstring_ == L'\0';
    if (file_)
      return file_eof();
    if (istream_)
      return istream_->eof();
    return true;
  }
  /// Copy subsequent character sequence data into buffer.
  /// @returns the nonzero number of (less or equal to n) 8-bit characters added to buffer s from the current input, or zero when EOF.
  size_t get(
      char  *s, ///< points to the string buffer to fill with input
      size_t n) ///< size of buffer pointed to by s
  {
    if (cstring_)
    {
      size_t k = std::strlen(cstring_);
      if (k > n)
        k = n;
      std::memcpy(s, cstring_, k);
      cstring_ += k;
      return k;
    }
    if (wstring_)
    {
      size_t k = n;
      if (uidx_ < sizeof(utf8_))
      {
        size_t l = std::strlen(utf8_ + uidx_);
        if (l > k)
          l = k;
        std::memcpy(s, utf8_ + uidx_, l);
        uidx_ += static_cast<unsigned short>(l);
        k -= l;
        if (k == 0)
          return n;
        s += l;
        uidx_ = sizeof(utf8_);
      }
      wchar_t c;
      while ((c = *wstring_) != L'\0' && k > 0)
      {
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
            std::memcpy(s, utf8_, k);
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
        ++wstring_;
      }
      return n - k;
    }
    if (file_)
      return file_get(s, n);
    if (istream_ && istream_->good())
      return static_cast<size_t>(n == 1 ? istream_->get(s[0]).gcount() : istream_->read(s, static_cast<std::streamsize>(n)).gcount());
    return 0;
  }
  /// Set encoding for `FILE*` input to Const::plain, Const::utf16be, Const::utf16le, Const::utf32be, or Const::utf32le. File encodings are automatically detected by the presence of a UTF BOM in the file. This function may be used when a BOM is not present and file encoding is known or to override the BOM.
  void file_encoding(short enc) ///< Const::plain, Const::utf16be, Const::utf16le, Const::utf32be, or Const::utf32le
  {
    if (file_ && utfx_ != enc)
    {
      size_ = 0;
      uidx_ = 0;
      utfx_ = enc;
    }
  }
  /// Get encoding of the current `FILE*` input, Const::plain, Const::utf16be, Const::utf16le, Const::utf32be, or Const::utf32le.
  /// @returns Const::plain, Const::utf16be, Const::utf16le, Const::utf32be, or Const::utf32le.
  short file_encoding(void) const
  {
    return utfx_;
  }
 protected:
  /// Initialize the state after (re)setting the input source.
  void init(void)
  {
    size_ = 0;
    uidx_ = sizeof(utf8_);
    if (file_)
      file_init();
  }
  /// Implements init() on a FILE*.
  void file_init(void);
  /// Implements get() on a FILE*.
  size_t file_get(
      char  *s, ///< points to the string buffer to fill with input
      size_t n) ///< size of buffer pointed to by s
      ;
  /// Implements size() on a FILE*.
  void file_size(void);
  /// Implements good()operation on a FILE*.
  bool file_good(void)
  {
    return !::feof(file_) && !::ferror(file_);
  }
  /// Implements eof() on a FILE*.
  bool file_eof(void)
  {
    return ::feof(file_) != 0;
  }
  const char    *cstring_; ///< NUL-terminated char string input (when non-null)
  const wchar_t *wstring_; ///< NUL-terminated wide string input (when non-null)
  FILE          *file_;    ///< FILE* input (when non-null)
  std::istream  *istream_; ///< stream input (when non-null)
  size_t         size_;    ///< size of the input in bytes, when known
  char           utf8_[8]; ///< UTF-8 conversion buffer
  unsigned short uidx_;    ///< index in utf8_[] or >= 8 when unused
  unsigned short utfx_;    ///< 0 = ASCII/UTF-8, 1 = UTF-16 BE, 2 = UTF-16 LE, 3 = UTF-32 BE, 4 = UTF-32 LE
};

} // namespace reflex

#endif
