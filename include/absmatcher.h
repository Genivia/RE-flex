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
@file      absmatcher.h
@brief     RE/flex abstract matcher base class and pattern matcher class
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2016, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_ABSMATCHER_H
#define REFLEX_ABSMATCHER_H

// TODO FIXME this option speeds up matching somewhat, but slows down input() a lot
// #define FAST_GET

#include "debug.h"
#include "input.h"
#include "traits.h"
#include <cctype>
#include <iterator>

namespace reflex {

/// Check word character.
inline int isword(int c) ///< Character to check
  /// @returns nonzero if argument c is in [A-Za-z0-9_], zero otherwise.
{
  return std::isalnum(c) || c == '_';
}

/// The abstract matcher base class template defines an interface for all pattern matcher engines.
/**
The buffer expands when matches do not fit.  The buffer size is initially
2*BLOCK size.

```
      _________________
     |  |    |    |    |
buf_=|  |text|rest|free|
     |__|____|____|____|
        ^    ^    ^    ^
        cur_ pos_ end_ max_

buf_ // points to buffered input, grows to fit long matches
cur_ // current position in buf_ while matching text, cur_ = pos_ afterwards
pos_ // position in buf_ to start the next match
end_ // position in buf_ that is free to fill with more input
max_ // allocated size of buf_
txt_ // buf_ + cur_ points to the match, \0-terminated
len_ // length of the match
chr_ // buf_[pos_] character after this match, with buf_[pos_] replaced with \0
got_ // buf_[cur_-1] character before this match (assigned before each match)
```
 
More info TODO
*/
class AbstractMatcher {
 protected:
  typedef int Method; ///< a method is one of Const::SCAN, Const::FIND, Const::SPLIT, Const::MATCH
  /// AbstractMatcher::Const common constants.
  struct Const {
    static const Method SCAN  = 0;      ///< AbstractMatcher::match method is to scan input (tokenizer)
    static const Method FIND  = 1;      ///< AbstractMatcher::match method is to find pattern in input
    static const Method SPLIT = 2;      ///< AbstractMatcher::match method is to split input at pattern matches
    static const Method MATCH = 3;      ///< AbstractMatcher::match method is to match the entire input
    static const int NUL      = '\0';   ///< NUL string terminator
    static const int UNK      = 256;    ///< unknown meta-char marker
    static const int BOB      = 257;    ///< begin of buffer meta-char marker
    static const int EOB      = EOF;    ///< end of buffer meta-char marker
    static const size_t EMPTY = 0xffff; ///< accept() returns empty last split at end of input
    static const size_t BLOCK = 4096;   ///< buffer growth factor, buffer is initially 2*BLOCK size
  };
  /// AbstractMatcher::Options for matcher engines.
  struct Option {
    Option()
      :
        A(false),
        N(false),
        T(8)
    { }
    bool A; ///< accept any/all (?^X) negative patterns
    bool N; ///< nullable, find may return empty match (N/A to scan, split, matches)
    char T; ///< tab size, between 1 and 9, default is 8, for indent \i and \j
  };
  /// AbstractMatcher::Iterator class for scanning, searching, and splitting input character sequences.
  template<typename T> /// @tparam <T> AbstractMatcher or const AbstractMatcher
  class Iterator : public std::iterator<std::input_iterator_tag,T> {
   friend class AbstractMatcher;
   friend class Iterator<typename reflex::TypeOp<T>::ConstType>;
   friend class Iterator<typename reflex::TypeOp<T>::NonConstType>;
   public:
    /// Construct an AbstractMatcher::Iterator such that Iterator() == AbstractMatcher::Operation(*this, method).end().
    Iterator(void)
      :
        matcher_(NULL)
    { }
    /// Copy constructor.
    Iterator(const Iterator<typename reflex::TypeOp<T>::NonConstType>& it)
      :
        matcher_(it.matcher_),
        method_(it.method_)
    { }
    /// AbstractMatcher::Iterator dereference.
    T& operator*() const
      /// @returns (const) reference to the iterator's matcher.
    {
      return *matcher_;
    }
    /// AbstractMatcher::Iterator pointer.
    T* operator->() const
      /// @returns (const) pointer to the iterator's matcher.
    {
      return matcher_;
    }
    /// AbstractMatcher::Iterator equality.
    bool operator==(const Iterator<typename reflex::TypeOp<T>::ConstType>& rhs) const
      /// @returns true if iterator equals RHS.
    {
      return matcher_ == rhs.matcher_;
    }
    /// AbstractMatcher::Iterator inequality.
    bool operator!=(const Iterator<typename reflex::TypeOp<T>::ConstType>& rhs) const
      /// @returns true if iterator does not equal RHS.
    {
      return matcher_ != rhs.matcher_;
    }
    /// AbstractMatcher::Iterator preincrement.
    Iterator& operator++()
      /// @returns reference to this iterator.
    {
      if (matcher_->match(method_) == 0)
        matcher_ = NULL;
      return *this;
    }
    /// AbstractMatcher::Iterator postincrement.
    Iterator operator++(int)
      /// @returns iterator to current match.
    {
      Iterator it = *this;
      operator++();
      return it;
    }
    /// Construct an AbstractMatcher::Iterator to scan, search, or split an input character sequence.
    Iterator(
        AbstractMatcher *matcher, ///< iterate over pattern matches with this matcher
        Method           method)  ///< match using method Const::SCAN, Const::FIND, or Const::SPLIT
      :
        matcher_(matcher),
        method_(method)
    {
      if (matcher_)
      {
        matcher_->reset();
        if (matcher_->match(method_) == 0)
          matcher_ = NULL;
      }
    }
   private:
    AbstractMatcher *matcher_; ///< the matcher used by this iterator
    Method           method_;  ///< the method for pattern matching by this iterator's matcher
  };
 public:
  typedef AbstractMatcher::Iterator<AbstractMatcher>       iterator;       ///< std::input_iterator for scanning, searching, and splitting input character sequences
  typedef AbstractMatcher::Iterator<const AbstractMatcher> const_iterator; ///< std::input_iterator for scanning, searching, and splitting input character sequences
  /// AbstractMatcher::Operation functor to match input to a pattern, also provides a (const) AbstractMatcher::iterator to iterate over matches.
  class Operation {
   friend class AbstractMatcher;
   public:
    /// AbstractMatcher::Operation() matches input to a pattern using method Const::SCAN, Const::FIND, or Const::SPLIT.
    size_t operator()(void) const
      /// @returns true (match) or false
    {
      return matcher_->match(method_);
    }
    /// AbstractMatcher::Operation.begin() returns a std::input_iterator to the start of the matches.
    iterator begin(void) const
      /// @returns input iterator.
    {
      return iterator(matcher_, method_);
    }
    /// AbstractMatcher::Operation.end() returns a std::input_iterator to the end of matches.
    iterator end(void) const
      /// @returns input iterator.
    {
      return iterator();
    }
    /// AbstractMatcher::Operation.cbegin() returns a const std::input_iterator to the start of the matches.
    const_iterator cbegin(void) const
      /// @returns input const_iterator.
    {
      return const_iterator(matcher_, method_);
    }
    /// AbstractMatcher::Operation.cend() returns a const std::input_iterator to the end of matches.
    const_iterator cend(void) const
      /// @returns input const_iterator.
    {
      return const_iterator();
    }
   private:
    /// Construct an AbstractMatcher::Operation functor to scan, search, or split an input character sequence.
    Operation(
        AbstractMatcher *matcher, ///< use this matcher for this functor
        Method           method)  ///< match using method Const::SCAN, Const::FIND, or Const::SPLIT
      :
        matcher_(matcher),
        method_(method)
    { }
    AbstractMatcher *matcher_; ///< the matcher used by this functor
    Method           method_;  ///< the method for pattern matching by this functor's matcher
  };
 public:
  /// Reset this matcher's state to the initial state and set options (when provided).
  virtual void reset(const char *opt = NULL)
  {
    DBGLOG("AbstractMatcher::reset(%s)", opt ? opt : "(null)");
    if (opt)
    {
      opt_.A = false;
      opt_.N = false;
      opt_.T = 8;
      if (opt)
      {
        for (const char *s = opt; *s != '\0'; ++s)
        {
          switch (*s)
          {
            case 'A':
              opt_.A = true;
              break;
            case 'N':
              opt_.N = true;
              break;
            case 'T':
              opt_.T = isdigit(*(s += (s[1] == '=') + 1)) ? *s - '0' : 0;
              break;
          }
        }
      }
    }
    buf_[0] = '\0';
    txt_ = buf_;
    len_ = 0;
    cap_ = 0;
    cur_ = 0;
    pos_ = 0;
    end_ = 0;
    ind_ = 0;
    lno_ = 1;
    cno_ = 0;
    num_ = 0;
    got_ = Const::BOB;
    chr_ = Const::UNK;
    eof_ = false;
    mat_ = false;
    blk_ = 0;
  }
  /// Set buffer block size for reading: use 1 for interactive input, 0 (or omit argument) to buffer all input (returns true if the size of the input source is determinable).
  bool buffer(size_t blk = 0) ///< new block size between 1 and Const::BLOCK, or 0 to buffer all
    /// @returns true when successful to buffer all input when n=0.
    /// @warning Use this method before any matching is done and before any input is read since the last time input was (re)set.
  {
    if (blk > Const::BLOCK)
      blk = Const::BLOCK;
    DBGLOG("AbstracMatcher::buffer(%zu)", blk);
    blk_ = blk;
    if (blk == 0)
    {
      if (!at_bob()) // only possible to fetch all if not at begin of input
        return false;
      size_t n = in.size(); // get the data size
      if (n == 0)
        return false;
      (void)grow(n); // attempt to fetch all (remaining) input data to store in the buffer
      end_ = get(buf_, n);
      if (end_ > 0)
        chr_ = static_cast<unsigned char>(buf_[pos_]);
      if (end_ < n)
        return false; // could not fetch all
    }
    return true;
  }
  /// Set buffer to 1 for interactive input.
  void interactive(void)
    /// @warning Use this method before any matching is done and before any input is read since the last time input was (re)set.
  {
    (void)buffer(1);
  }
  /// Flush the buffer's remaining content.
  void flush(void)
  {
    DBGLOG("AbstracMatcher::flush()");
    pos_ = end_;
  }
  /// Set the input character sequence for this matcher and reset the matcher.
  virtual AbstractMatcher& input(const Input& inp) ///< input character sequence for this matcher
    /// @returns this matcher.
  {
    DBGLOG("AbstracMatcher::input()");
    in = inp;
    reset();
    return *this;
  }
  /// Returns true if the entire input matches this matcher's pattern (and internally caches the true/false result for repeat invocations).
  size_t matches(void)
    /// @returns true if the entire input matched this matcher's pattern.
  {
    if (mat_ == 0 && at_bob())
      mat_ = match(Const::MATCH) && at_end();
    return mat_;
  }
  /// Returns a positive integer (true) indicating the capture index of the matched text in the pattern or zero (false) for a mismatch.
  size_t accept(void) const
    /// @returns nonzero capture index of the match in the pattern, which may be matcher dependent, or zero for a mismatch, or Const::EMPTY for the empty last split.
  {
    return cap_;
  }
  /// Returns string with the text matched by one of the match methods of this matcher.
  const char *text(void) const
    /// @returns NUL-terminated const char* string.
  {
    return txt_;
  }
  /// Returns the length of the text matched by one of the match methods of this matcher.
  size_t size(void) const
    /// @returns string length.
  {
    return len_;
  }
  /// Returns the line number of the match in the input character sequence.
  size_t lineno(void) const
    /// @returns line number.
  {
    size_t n = lno_;
    for (const char *s = buf_; s < txt_; ++s)
      n += (*s == '\n');
    return n;
  }
  /// Returns the column number of the match in the input character sequence.
  size_t columno(void) const
    /// @returns column number.
  {
    for (const char *s = txt_ - 1; s >= buf_; --s)
      if (*s == '\n')
        return txt_ - s - 1;
#ifdef WITH_BYTE_COLUMNO
    // count column offset in bytes
    return cno_ + txt_ - buf_;
#else
    // count column offset in UTF-8 chars
    size_t n = cno_;
    for (const char *t = buf_; t < txt_; ++t)
      n += (*t & 0xc0) != 0x80;
    return n;
#endif
  }
  /// Returns a pair of size_t accept() and std::string text(), useful for tokenizing input into containers of pairs.
  std::pair<size_t,std::string> pair() const
    /// @returns a std::pair of size_t accept() and std::string text().
  {
    return std::pair<size_t,std::string>(accept(), std::string(text(), size()));
  }
  /// Returns the position of the first character starting the match in the input character sequence.
  size_t first(void) const
    /// @returns position in the input character sequence.
  {
    return num_ + txt_ - buf_;
  }
  /// Returns the position of the last character + 1 after of the match in the input character sequence.
  size_t last(void) const
    /// @returns position in the input character sequence.
  {
    return first() + size();
  }
  /// Returns true if this matcher is at the start of an input character sequence. Use reset() to restart input.
  bool at_bob(void) const
    /// @returns true if at the begin of an input sequence.
  {
    return got_ == Const::BOB;
  }
  /// Returns true if this matcher has no more input to read from the input character sequence.
  bool at_end(void)
    /// @returns true if at end of input and a read attempt will produce EOF.
  {
    return pos_ == end_ && (eof_ || peek() == EOF);
  }
  /// Returns true if this matcher hit the end of the input character sequence.
  bool hit_end(void) const
    /// @returns true if EOF was hit (and possibly more input would have changed the result), false otherwise (but next read attempt may return EOF immediately).
  {
    return pos_ == end_ && eof_;
  }
  /// Set and force the end of input state.
  void set_end(bool eof)
  {
    if (eof)
      flush();
    eof_ = eof;
  }
  /// Returns true if this matcher reached the begin of a new line.
  bool at_bol(void) const
    /// @returns true if at begin of a new line.
  {
    return got_ == '\n';
  }
  /// Set the begin of a new line state.
  void set_bol(bool bol)
  {
    if (bol)
      got_ = '\n';
    else if (got_ == '\n')
      got_ = Const::UNK;
  }
  /// Returns the next character from the input character sequence while preserving the current text match.
  int input(void)
    /// @returns the character read (unsigned char 0..255) read or EOF (-1).
  {
    DBGLOG("AbstractMatcher::input() pos = %zu end = %zu chr = %c", pos_, end_, chr_);
    if (pos_ < end_)
    {
      if (chr_ != Const::UNK)
        got_ = chr_;
      else
        got_ = static_cast<unsigned char>(buf_[pos_]);
      ++pos_;
    }
    else
    {
#ifdef FAST_GET
      got_ = get_more();
#else
      got_ = get();
#endif
    }
    if (pos_ < end_)
      chr_ = static_cast<unsigned char>(buf_[pos_]);
    cur_ = pos_;
    return got_;
  }
  /// Put back one character on the input character sequence for matching, invalidating the current match info and text.
  void unput(char c) ///< character to put back
  {
    DBGLOG("AbstractMatcher::unput()");
    if (pos_ < end_)
      buf_[pos_] = chr_;
    if (pos_ > 0)
    {
      --pos_;
    }
    else
    {
      txt_ = buf_;
      len_ = 0;
      if (end_ == max_)
        (void)grow();
      std::memmove(buf_ + 1, buf_, end_);
      ++end_;
    }
    chr_ = static_cast<unsigned char>(c);
    cur_ = pos_;
  }
  /// Fetch the rest of the input as text, useful for searching/splitting up to n times after which the rest is needed.
  const char *rest(void)
    /// @returns const char* string of the remaining input (wrapped when AbstractMatcher::wrap is defined).
  {
    DBGLOG("AbstractMatcher::rest()");
    if (pos_ < end_)
      buf_[pos_] = chr_;
    if (pos_ > 0)
    {
      DBGLOGN("Shift buffer to close gap of %lu", pos_);
      txt_ = buf_ + pos_;
      update();
      end_ -= pos_;
      std::memmove(buf_, buf_ + pos_, end_);
    }
    txt_ = buf_;
    while (!eof_)
    {
      (void)grow();
      pos_ = end_;
      end_ += get(buf_ + end_, blk_ ? blk_ : max_ - end_);
      if (pos_ == end_)
      {
        DBGLOGN("rest(): EOF");
        if (!wrap())
        {
          eof_ = true;
          break;
        }
      }
    }
    cur_ = pos_ = 0;
    chr_ = static_cast<unsigned char>(buf_[0]);
    len_ = end_;
    buf_[len_] = '\0';
    DBGLOGN("rest() length = %zu", len_);
    return txt_;
  }
  /// Append the next match to the currently matched text returned by AbstractMatcher::text, when the next match found is adjacent to the current match.
  void more(void)
  {
    cur_ = txt_ - buf_;
  }
  /// Truncate the AbstractMatcher::text length of the match to n characters in length and reposition for next match.
  void less(size_t n) ///< truncated string length
  {
    if (n < len_)
    {
      DBGCHK(pos_ < max_);
      buf_[pos_] = chr_;
      pos_ = txt_ - buf_ + n;
      DBGCHK(pos_ < max_);
      chr_ = static_cast<unsigned char>(buf_[pos_]);
      buf_[pos_] = '\0';
      len_ = n;
      cur_ = pos_;
    }
  }
  /// Cast this matcher to positive integer indicating the nonzero capture index of the matched text in the pattern, same as AbstractMatcher::accept.
  operator size_t() const
    /// @returns nonzero capture index of a match, which may be matcher dependent, or zero for a mismatch.
  {
    return accept();
  }
  /// Cast this matcher to a std::string of the text matched by this matcher.
  operator std::string() const
    /// @returns std::string allocated from NUL-terminated matched text.
  {
    return std::string(text(), size());
  }
  /// Cast this matcher to a pair of size_t accept() and std::string text(), useful for tokenization into containers.
  operator std::pair<size_t,std::string>() const
    /// @returns a std::pair of size_t accept() and std::string text().
  {
    return pair();
  }
  /// Returns true if matched text is equal to a string, useful for std::algorithm.
  bool operator==(const char *rhs) ///< rhs string to compare to
    /// @returns true if matched text is equal to rhs string.
    const
  {
    return std::strcmp(rhs, text()) == 0;
  }
  /// Returns true if matched text is equalt to a string, useful for std::algorithm.
  bool operator==(const std::string& rhs) ///< rhs string to compare to
    /// @returns true if matched text is equal to rhs string.
    const
  {
    return rhs.compare(0, rhs.size(), text(), size()) == 0;
  }
  /// Returns true if capture index is equal to a given size_t value, useful for std::algorithm.
  bool operator==(size_t rhs) ///< capture index to compare accept() to
    /// @returns true if capture index is equal to rhs.
    const
  {
    return accept() == rhs;
  }
  /// Returns true if capture index is equal to a given int value, useful for std::algorithm.
  bool operator==(int rhs) ///< capture index to compare accept() to
    /// @returns true if capture index is equal to rhs.
    const
  {
    return static_cast<int>(accept()) == rhs;
  }
  /// Returns true if matched text is not equal to a string, useful for std::algorithm.
  bool operator!=(const char *rhs) ///< rhs string to compare to
    /// @returns true if matched text is not equal to rhs string.
    const
  {
    return std::strcmp(rhs, text()) != 0;
  }
  /// Returns true if matched text is not equal to a string, useful for std::algorithm.
  bool operator!=(const std::string& rhs) ///< rhs string to compare to
    /// @returns true if matched text is not equal to rhs string.
    const
  {
    return rhs.compare(0, rhs.size(), text(), size()) != 0;
  }
  /// Returns true if capture index is not equal to a given size_t value, useful for std::algorithm.
  bool operator!=(size_t rhs) ///< capture index to compare accept() to
    /// @returns true if capture index is not equal to rhs.
    const
  {
    return accept() != rhs;
  }
  /// Returns true if capture index is not equal to a given int value, useful for std::algorithm.
  bool operator!=(int rhs) ///< capture index to compare accept() to
    /// @returns true if capture index is not equal to rhs.
    const
  {
    return static_cast<int>(accept()) != rhs;
  }
  Operation scan;  ///< functor to scan input (to tokenize input)
  Operation find;  ///< functor to search input
  Operation split; ///< functor to split input
  Input in;        ///< input character sequence being matched by this matcher
 protected:
  /// Construct a base abstract matcher.
  AbstractMatcher(
      const Input& inp, ///< input character sequence for this matcher
      const char  *opt) ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      scan(this, Const::SCAN),
      find(this, Const::FIND),
      split(this, Const::SPLIT)
  {
    in = inp;
    init(opt);
  }
  /// Initialize the base abstract matcher at construction.
  void init(const char *opt) ///< options
  {
    DBGLOG("AbstractMatcher::init(%s)", opt ? opt : "");
    buf_ = new char[max_ = 2 * Const::BLOCK];
    reset(opt);
  }
  /// Returns more input (method can be overriden as by reflex::FlexLexer::get to invoke reflex::FlexLexer::LexerInput).
  virtual size_t get(
      /// @returns the nonzero number of (less or equal to n) 8-bit characters added to buffer s from the current input, or zero when EOF.
      char  *s, ///< points to the string buffer to fill with input
      size_t n) ///< size of buffer pointed to by s
  {
    return in.get(s, n);
  }
  /// Returns true if wrapping of input after EOF is supported.
  virtual bool wrap(void)
    /// @returns true if input was succesfully wrapped.
  {
    return false;
  }
  /// The abstract match operation implemented by pattern matching engines derived from AbstractMatcher.
  virtual size_t match(Method method)
    /// @returns nonzero when input matched the pattern using method Const::SCAN, Const::FIND, Const::SPLIT, or Const::MATCH.
    = 0;
  /// Shift or expand the internal buffer when it is too small to accommodate more input, where the buffer size is doubled when needed.
  bool grow(size_t need = Const::BLOCK) ///< optional needed space = Const::BLOCK size by default
    /// @returns true if buffer was shifted or was enlarged
  {
    if (max_ - end_ >= need)
      return false;
    size_t gap = txt_ - buf_;
    if (gap >= need)
    {
      DBGLOGN("Shift buffer to close gap of %lu", gap);
      update();
      cur_ -= gap;
      ind_ -= gap;
      pos_ -= gap;
      end_ -= gap;
      std::memmove(buf_, txt_, end_);
      txt_ = buf_;
    }
    else
    {
      size_t newmax = end_ - gap + need;
      size_t oldmax = max_;
      while (max_ < newmax)
        max_ *= 2;
      if (oldmax < max_)
      {
        DBGLOGN("Expand buffer from %lu to %lu", oldmax, max_);
        update();
        cur_ -= gap;
        ind_ -= gap;
        pos_ -= gap;
        end_ -= gap;
        char *newbuf = new char[max_];
        std::memcpy(newbuf, txt_, end_);
        delete[] buf_;
        txt_ = buf_ = newbuf;
      }
    }
    return true;
  }
  /// Returns the next character from the buffered input character sequence.
  int get(void)
    /// @returns the character read (unsigned char 0..255) or EOF (-1).
  {
    DBGLOG("AbstractMatcher::get()");
#ifdef FAST_GET
    return pos_ < end_ ? static_cast<unsigned char>(buf_[pos_++]) : get_more();
#else
    if (pos_ < end_)
      return static_cast<unsigned char>(buf_[pos_++]);
    if (eof_)
      return EOF;
    if (end_ + blk_ >= max_)
      (void)grow();
    while (true)
    {
      end_ += get(buf_ + end_, blk_ ? blk_ : max_ - end_);
      if (pos_ < end_)
        return static_cast<unsigned char>(buf_[pos_++]);
      DBGLOGN("get(): EOF");
      if (!wrap())
      {
        eof_ = true;
        return EOF;
      }
    }
#endif
  }
  /// Peek at the next character in the buffered input without consuming it.
  int peek(void)
    /// @returns the character (unsigned char 0..255) or EOF (-1).
  {
    DBGLOG("AbstractMatcher::peek()");
#ifdef FAST_GET
    return pos_ < end_ ? static_cast<unsigned char>(buf_[pos_]) : peek_more();
#else
    if (pos_ < end_)
      return static_cast<unsigned char>(buf_[pos_]);
    if (eof_)
      return EOF;
    if (end_ + blk_ >= max_)
      (void)grow();
    while (true)
    {
      end_ += get(buf_ + end_, blk_ ? blk_ : max_ - end_);
      if (pos_ < end_)
        return static_cast<unsigned char>(buf_[pos_]);
      DBGLOGN("peek(): EOF");
      if (!wrap())
      {
        eof_ = true;
        return EOF;
      }
    }
#endif
  }
  /// Set the current position to advance to the next match.
  void set_current(size_t loc) ///< new location in buffer
  {
    DBGCHK(loc <= end_);
    pos_ = cur_ = loc;
    if (cur_ > 0)
      got_ = static_cast<unsigned char>(buf_[loc - 1]);
    else
      got_ = Const::UNK;
    if (loc < end_)
      chr_ = static_cast<unsigned char>(buf_[loc]);
    else
      chr_ = peek();
  }
  Option         opt_; ///< options for matcher engines
  char          *buf_; ///< input character sequence buffer
  const char    *txt_; ///< points to the matched text in buffer AbstractMatcher::buf_
  size_t         len_; ///< size of the matched text
  size_t         cap_; ///< nonzero capture index of an accepted match or zero
  size_t         cur_; ///< next position in AbstractMatcher::buf_ to assign to AbstractMatcher::txt_
  size_t         pos_; ///< position in AbstractMatcher::buf_ after AbstractMatcher::txt_
  size_t         end_; ///< ending position of the input buffered in AbstractMatcher::buf_
  size_t         max_; ///< total buffer size and max position + 1 to fill
  size_t         ind_; ///< current indent position
  size_t         blk_; ///< block size for block-based input reading, as set by AbstractMatcher::buffer
  int            got_; ///< last unsigned character we looked at (to determine anchors and boundaries)
  int            chr_; ///< the character located at AbstractMatcher::buf_[AbstractMatcher::pos_]
  size_t         lno_; ///< line number count (prior to this buffered input)
  size_t         cno_; ///< column number count (prior to this buffered input)
  size_t         num_; ///< character count (number of characters flushed prior to this buffered input)
  bool           eof_; ///< input has reached EOF
  bool           mat_; ///< true if AbstractMatcher::matches() was successful
 private:
#ifdef FAST_GET
  /// Get the next character if not currently buffered.
  int get_more(void)
    /// @returns the character read (unsigned char 0..255) or EOF (-1).
  {
    if (eof_)
      return EOF;
    if (end_ + blk_ >= max_)
      (void)grow();
    while (true)
    {
      end_ += get(buf_ + end_, blk_ ? blk_ : max_ - end_);
      if (pos_ < end_)
        return static_cast<unsigned char>(buf_[pos_++]);
      DBGLOGN("get(): EOF");
      if (!wrap())
      {
        eof_ = true;
        return EOF;
      }
    }
  }
  /// Peek at the next character if not currently buffered.
  int peek_more(void)
    /// @returns the character (unsigned char 0..255) or EOF (-1).
  {
    if (eof_)
      return EOF;
    if (end_ + blk_ >= max_)
      (void)grow();
    while (true)
    {
      end_ += get(buf_ + end_, blk_ ? blk_ : max_ - end_);
      if (pos_ < end_)
        return static_cast<unsigned char>(buf_[pos_]);
      DBGLOGN("peek(): EOF");
      if (!wrap())
      {
        eof_ = true;
        return EOF;
      }
    }
  }
#endif
  /// Update the newline count, column count, and character count when shifting the buffer. 
  void update(void)
  {
#ifdef WITH_BYTE_COLUMNO
    const char *t = buf_;
    for (const char *s = buf_; s < txt_; ++s)
    {
      if (*s == '\n')
      {
        ++lno_;
        t = s;
        cno_ = 0;
      }
    }
    // count column offset in bytes
    cno_ += txt_ - t;
    num_ += txt_ - buf_;
#else
    for (const char *s = buf_; s < txt_; ++s)
    {
      if (*s == '\n')
      {
        ++lno_;
        cno_ = 0;
      }
      else
      {
        // count column offset in UTF-8 chars
        cno_ += (*s & 0xc0) != 0x80;
      }
    }
    num_ += txt_ - buf_;
#endif
  }
};

/// The pattern matcher class template extends abstract matcher base class.
/** More info TODO */
template<typename P> /// @tparam <P> pattern class to instantiate a matcher
class PatternMatcher : public AbstractMatcher {
 public:
  typedef P Pattern; ///< pattern class of this matcher, a typedef of the PatternMatcher template parameter
  /// Copy constructor.
  PatternMatcher(const PatternMatcher& matcher)
    :
      AbstractMatcher(matcher.in, matcher.opt_),
      own_(false),
      pat_(matcher.pat_)
  { }
  /// Delete matcher, deletes pattern when owned, deletes this matcher's internal buffer.
  virtual ~PatternMatcher()
  {
    DBGLOG("PatternMatcher::~PatternMatcher()");
    if (own_)
      delete pat_;
    delete[] buf_;
  }
  /// Set the pattern to use with this matcher.
  virtual PatternMatcher& pattern(const Pattern& pat) ///< pattern object for this matcher
    /// @returns this matcher.
  {
    DBGLOG("Patternatcher::pattern()");
    if (pat_ != &pat)
    {
      if (own_)
        delete pat_;
      pat_ = &pat;
      own_ = false;
    }
    return *this;
  }
  /// Set the pattern from a regex string to use with this matcher.
  virtual PatternMatcher& pattern(const char *regex) ///< regex string to instantiate internal pattern object
    /// @returns this matcher.
  {
    DBGLOG("Patternatcher::pattern(\"%s\")", regex);
    if (own_)
      delete pat_;
    pat_ = new Pattern(regex);
    own_ = true;
    return *this;
  }
  /// Set the pattern from a regex string to use with this matcher.
  virtual PatternMatcher& pattern(const std::string& regex) ///< regex string to instantiate internal pattern object
    /// @returns this matcher.
  {
    DBGLOG("Patternatcher::pattern(\"%s\")", regex.c_str());
    if (own_)
      delete pat_;
    pat_ = new Pattern(regex);
    own_ = true;
    return *this;
  }
  /// Returns the pattern object associated with this matcher.
  const Pattern& pattern(void) const
    /// @returns reference to pattern object.
  {
    return *pat_;
  }
 protected:
  /// Construct a base abstract matcher from a pattern object and an input character sequence.
  PatternMatcher(
      const Pattern& pat,           ///< pattern object for this matcher
      const Input&   inp = Input(), ///< input character sequence for this matcher
      const char    *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      AbstractMatcher(inp, opt),
      own_(false),
      pat_(&pat)
  { }
  /// Construct a base abstract matcher from a pattern object pointer and an input character sequence.
  PatternMatcher(
      const Pattern *pat,           ///< points to pattern object for this matcher
      const Input&   inp = Input(), ///< input character sequence for this matcher
      const char    *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      AbstractMatcher(inp, opt),
      own_(false),
      pat_(pat)
  { }
  /// Construct a base abstract matcher from a regex pattern string and an input character sequence.
  PatternMatcher(
      const char  *regex,         ///< regex string instantiates pattern object for this matcher
      const Input& inp = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      AbstractMatcher(inp, opt),
      own_(true),
      pat_(new Pattern(regex))
  { }
  /// Construct a base abstract matcher from a regex pattern string and an input character sequence.
  PatternMatcher(
      const std::string& regex,         ///< regex string instantiates pattern object for this matcher
      const Input&       inp = Input(), ///< input character sequence for this matcher
      const char        *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      AbstractMatcher(inp, opt),
      own_(true),
      pat_(new Pattern(regex))
  { }
  bool           own_; ///< true if PatternMatcher::pat_ was internally allocated
  const Pattern *pat_; ///< points to the pattern object used by the matcher
};

} // namespace reflex

#endif
