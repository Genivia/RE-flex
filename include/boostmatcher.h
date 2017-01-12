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
@file      boostmatcher.h
@brief     Boost::regex-based matcher engines for pattern matching
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2017, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_BOOSTMATCHER_H
#define REFLEX_BOOSTMATCHER_H

#include "absmatcher.h"
#include <boost/regex.hpp>

namespace reflex {

/// Boost matcher engine class implements reflex::PatternMatcher pattern matching interface with scan, find, split functors and iterators, using the Boost::regex library.
/** More info TODO */
class BoostMatcher : public PatternMatcher<boost::regex> {
 public:
  /// Construct matcher engine from a boost::regex object or string regex, and an input character sequence.
  template<typename P> /// @tparam <P> pattern is a boost::regex or a string regex
  BoostMatcher(
      const P&     pat,           ///< a boost::regex or a string regex for this matcher
      const Input& inp = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      PatternMatcher(pat, inp, opt),
      flg_(boost::regex_constants::match_partial | boost::regex_constants::match_not_dot_newline)
  {
    reset();
  }
  /// Construct matcher engine from a boost::regex object or string regex, and an input character sequence.
  template<typename P> /// @tparam <P> pattern is a boost::regex or a string regex
  BoostMatcher(
      const P     *pat,           ///< points to a boost::regex or a string regex for this matcher
      const Input& inp = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      PatternMatcher(pat, inp, opt),
      flg_(boost::regex_constants::match_partial | boost::regex_constants::match_not_dot_newline)
  {
    reset();
  }
  /// Reset this matcher's state to the initial state and when assigned new input.
  virtual void reset(const char *opt = NULL)
  {
    DBGLOG("BoostMatcher::reset()");
    itr_ = fin_;
    PatternMatcher::reset(opt);
  }
  using PatternMatcher::pattern;
  /// Set the pattern to use with this matcher.
  virtual PatternMatcher& pattern(const Pattern& pat) ///< boost::regex for this matcher
    /// @returns this matcher.
  {
    itr_ = fin_;
    return PatternMatcher::pattern(pat);
  }
  /// Set the pattern from a regex string to use with this matcher.
  virtual PatternMatcher& pattern(const char *regex) ///< regex string to instantiate internal pattern object
    /// @returns this matcher.
  {
    itr_ = fin_;
    return PatternMatcher::pattern(regex);
  }
  /// Set the pattern from a regex string to use with this matcher.
  virtual PatternMatcher& pattern(const std::string& regex) ///< regex string to instantiate internal pattern object
    /// @returns this matcher.
  {
    itr_ = fin_;
    return PatternMatcher::pattern(regex);
  }
  /// Convert a string regex to a regex with group captures only for the outermost (top-level) choices. All outermost (top-level) choices `X1`, `X2`, ... `Xn` in a regex `X1|X2|...|Xn` are regrouped such that `X1|X2|...|Xn` => `(X1)|(X2)|...|(Xn)`. Innermost (nested) group captures `(Y)` are converted to non-captured `(Y)` => `(?:Y)`. All existing non-capturing groups are retained. For example, `a|b(c)|(?:d)` => `(a)|(b(?:c))|((?:d))`.
  static std::string regroup(const std::string& regex)
    /// @returns string regex.
  {
    DBGLOG("BoostMatcher::regroup(\"%s\")", regex.c_str());
    std::string newregex = "(";
    size_t pos = 0;
    size_t loc = 0;
    size_t lev = 0;
    size_t len = regex.size();
    if (len > 2 && regex.compare(0, 2, "(?") == 0 && regex.at(2) != '#')
    {
      pos = 2;
      while (pos < len && regex.at(pos) != ':' && regex.at(pos) != ')')
        ++pos;
      if (regex.at(pos) != ')')
        ++lev;
      newregex = regex.substr(0, pos + 1).append("(");
      loc = ++pos;
    }
    while (pos < len)
    {
      switch (regex.at(pos))
      {
        case '\\':
          if (pos + 1 < len && regex.at(++pos) == 'Q')
            while (pos + 2 < len && (regex.at(++pos) != '\\' || regex.at(pos + 1) != 'E'))
              continue;
          break;
        case '(':
          if (pos + 1 < len)
          {
            if (regex.at(pos + 1) != '?')
            {
              newregex.append(regex.substr(loc, pos - loc)).append("(?:"); // translate ( to (?:
              loc = pos + 1;
            }
            else if (pos + 2 < len && regex.at(pos + 2) == '#')
            {
              pos = regex.find(')', pos + 2) - 1; // skip over "(?#..." to reach ')'
            }
          }
          ++lev;
          break;
        case ')':
          --lev;
          break;
        case '|':
          if (lev == 0)
          {
            newregex.append(regex.substr(loc, pos - loc)).append(")|(");
            loc = pos + 1;
          }
          break;
        case '[':
          newregex.append(regex.substr(loc, pos - loc));
          loc = pos++;
          if (pos + 1 < len && regex.at(pos) == '^')
            ++pos;
          while (pos + 1 < len && regex.at(++pos) != ']')
            continue;
          break;
      }
      ++pos;
    }
    newregex.append(regex.substr(loc, pos - loc)).append(")");
    DBGLOGN("Regrouped \"%s\"", newregex.c_str());
    return newregex;
  }
 protected:
  /// The match method Const::SCAN, Const::FIND, Const::SPLIT, or Const::MATCH, implemented with boost::regex.
  virtual size_t match(Method method)
    /// @returns nonzero when input matched the pattern using method Const::SCAN, Const::FIND, Const::SPLIT, or Const::MATCH.
  {
    DBGLOG("BEGIN BoostMatcher::match(%d)", method);
    bool bob = at_bob();
    txt_ = buf_ + cur_; // set first of text(), cur_ was last pos_, or cur_ was set with more()
    cur_ = pos_; // reset cur_ when changed in more()
    if (pos_ < end_) // if next pos_ is not hitting the end_ then
      buf_[pos_] = chr_; // last of text() was set to NUL in buf_[], set it back
    if (itr_ != fin_) // if regex iterator is still valid then
    {
      if ((*itr_)[0].second == buf_ + pos_) // if last of regex iterator is still valid in buf_[] then
      {
        DBGLOGN("Continue iterating, pos = %zu", pos_);
        ++itr_;
        if (itr_ != fin_) // set pos_ to last of the (partial) match
          pos_ = (*itr_)[0].second - buf_;
      }
      else
      {
        itr_ = fin_; // need new iterator
      }
    }
    while (pos_ == end_ || itr_ == fin_) // fetch more data while pos_ is hitting the end_ or no iterator
    {
      if (pos_ == end_ && !eof_)
      {
        if (grow()) // make sure we have enough storage to read input
          itr_ = fin_; // buffer shifting/growing invalidates iterator
        end_ += get(buf_ + end_, blk_ ? blk_ : max_ - end_);
      }
      if (pos_ == end_) // if pos_ is hitting the end_ then
      {
        if (wrap())
          continue; // continue after successful wrap
        if (method == Const::SPLIT)
        {
          DBGLOGN("Split end");
          if (eof_)
          {
            cap_ = 0;
            len_ = 0;
          }
          else
          {
            if (itr_ == fin_)
              new_itr(method, bob);
            if (itr_ != fin_ && (*itr_)[0].matched && cur_ != pos_)
            {
              size_t n = (*itr_).size();
              for (cap_ = 1; cap_ < n && !(*itr_)[cap_].matched; ++cap_)
                continue; // set cap_ to the capture index
              len_ = (*itr_)[0].first - txt_; // size() spans txt_ to cur_ in buf_[]
            }
            else
            {
              DBGLOGN("Matched empty end");
              cap_ = Const::EMPTY;
              len_ = pos_ - (txt_ - buf_); // size() spans txt_ to cur_ in buf_[]
              eof_ = true;
            }
            itr_ = fin_;
            cur_ = pos_;
            buf_[txt_ - buf_ + len_] = '\0';
            DBGLOGN("Split: act = %zu txt = '%s' len = %zu pos = %zu eof = %d", cap_, txt_, len_, pos_, eof_ == true);
          }
          DBGLOG("END BoostMatcher::match()");
          return cap_;
        }
        eof_ = true;
        if (method == Const::FIND && opt_.N)
          return 0;
        if (itr_ != fin_)
          break; // OK if iterator is still valid
      }
      new_itr(method, bob); // need new iterator
      if (itr_ != fin_)
      {
        DBGLOGN("Possible (partial) match, pos = %zu", pos_);
        // check for scan/match AND txt_ == (*itr_)[0].first to avoid consuming ALL input when no match is found
        // not needed with match_continuous
        //if ((method == Const::SCAN || method == Const::MATCH) && buf_ + cur_ != (*itr_)[0].first)
          //break;
        pos_ = (*itr_)[0].second - buf_; // set pos_ to last of the (partial) match
        if (pos_ == cur_ && !bob) // match is at same pos as previous
        {
          ++itr_; // advance to next match
          if (itr_ != fin_)
            pos_ = (*itr_)[0].second - buf_; // set pos_ to last of the (partial) match
          else
            pos_ = end_;
        }
      }
      else // no (partial) match
      {
        if ((method == Const::SCAN || method == Const::MATCH))
        {
          itr_ = fin_;
          pos_ = cur_;
          len_ = 0;
          cap_ = 0;
          chr_ = static_cast<unsigned char>(buf_[pos_]);
          buf_[pos_] = '\0';
          DBGLOGN("No (partial) match, pos = %zu", pos_);
          DBGLOG("END BoostMatcher::match()");
          return 0;
        }
        pos_ = end_;
        if (eof_)
        {
          len_ = 0;
          cap_ = 0;
          DBGLOGN("No match at EOF, pos = %zu", pos_);
          DBGLOG("END BoostMatcher::match()");
          return 0;
        }
      }
    }
    if (method == Const::SPLIT)
    {
      DBGLOGN("Split match");
      size_t n = (*itr_).size();
      for (cap_ = 1; cap_ < n && !(*itr_)[cap_].matched; ++cap_)
        continue; // set cap_ to the capture index
      len_ = (*itr_)[0].first - txt_; // cur_ - (txt_ - buf_); // size() spans txt_ to cur_ in buf_[]
      set_current(pos_);
      buf_[txt_ - buf_ + len_] = '\0';
      DBGLOGN("Split: act = %zu txt = '%s' len = %zu pos = %zu", cap_, txt_, len_, pos_);
      DBGLOG("END BoostMatcher::match()");
      return cap_;
    }
    else if (!(*itr_)[0].matched || (buf_ + cur_ != (*itr_)[0].first && method != Const::FIND)) // if no match at first and we're not searching then
    {
      itr_ = fin_;
      pos_ = cur_;
      len_ = 0;
      cap_ = 0;
      chr_ = static_cast<unsigned char>(buf_[pos_]);
      buf_[pos_] = '\0';
      DBGLOGN("No match, pos = %zu", pos_);
      DBGLOG("END BoostMatcher::match()");
      return 0;
    }
    if (method == Const::FIND)
      txt_ = (*itr_)[0].first;
    size_t n = (*itr_).size();
    for (cap_ = 1; cap_ < n && !(*itr_)[cap_].matched; ++cap_)
      continue; // set cap_ to the capture group index
    set_current(pos_);
    buf_[pos_] = '\0';
    len_ = cur_ - (txt_ - buf_); // size() spans txt_ to cur_ in buf_[]
    if (len_ == 0 && cap_ != 0 && opt_.N && pos_ + 1 == end_)
      set_current(end_);
    DBGLOGN("Accept: act = %zu txt = '%s' len = %zu", cap_, txt_, len_);
    DBGCHK(len_ != 0 || method == Const::MATCH || (method == Const::FIND && opt_.N));
    DBGLOG("END BoostMatcher::match()");
    return cap_;
  }
  /// Create a new boost::regex iterator to (continue to) advance over input.
  inline void new_itr(Method method, bool bob)
  {
    DBGLOGN("New iterator");
    bool bol = bob || at_bol();
    bool eow = isword(got_);
    boost::match_flag_type flg = flg_;
    if (!bob)
      flg |= boost::regex_constants::match_not_bob;
    if (!bol)
      flg |= boost::regex_constants::match_not_bol;
    if (eow)
      flg |= boost::regex_constants::match_not_bow;
    if (method == Const::SCAN)
      flg |= boost::regex_constants::match_continuous | boost::regex_constants::match_not_null;
    else if (method == Const::FIND && !opt_.N)
      flg |= boost::regex_constants::match_not_null;
    else if (method == Const::MATCH)
      flg |= boost::regex_constants::match_continuous;
    itr_ = boost::cregex_iterator(txt_, buf_ + end_, *pat_, flg);
  }
  boost::match_flag_type flg_; ///< boost match flags
  boost::cregex_iterator itr_; ///< boost regex iterator
  boost::cregex_iterator fin_; ///< boost regex iterator final end
};

/// Boost matcher engine class, extends reflex::BoostMatcher for Boost POSIX regex matching.
/**
Boost POSIX regex matching enables Boost match flags `match_posix` and
`match_not_dot_newline`. Lazy quantifiers are not supported by this matcher
engine.
*/
class BoostPosixMatcher : public BoostMatcher {
 public:
  /// Construct a POSIX matcher engine from a boost::regex pattern and an input character sequence.
  template<typename P> /// @tparam <P> pattern is a boost::regex or a string regex
  BoostPosixMatcher(
      const P&     pat,           ///< a boost::regex or a string regex for this matcher
      const Input& inp = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      BoostMatcher(pat, inp, opt)
  {
    flg_ |= boost::regex_constants::match_posix;
  }
  /// Construct a POSIX matcher engine from a boost::regex pattern and an input character sequence.
  template<typename P>
  BoostPosixMatcher(
      const P     *pat,           ///< points to a boost::regex or a string regex for this matcher
      const Input& inp = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      BoostMatcher(pat, inp, opt)
  {
    flg_ |= boost::regex_constants::match_posix;
  }
};

/// Boost matcher engine class, extends reflex::BoostMatcher for Boost Perl regex matching.
/**
Boost Perl regex matching enables Boost match flag `match_perl` and
`match_not_dot_newline`.
*/
class BoostPerlMatcher : public BoostMatcher {
 public:
  /// Construct a Perl matcher engine from a boost::regex pattern and an input character sequence.
  template<typename P> /// @tparam <P> pattern is a boost::regex or a string regex
  BoostPerlMatcher(
      const P&     pat,           ///< a boost::regex or a string regex for this matcher
      const Input& inp = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      BoostMatcher(pat, inp, opt)
  {
    flg_ |= boost::regex_constants::match_perl;
  }
  /// Construct a Perl matcher engine from a boost::regex pattern and an input character sequence.
  template<typename P>
  BoostPerlMatcher(
      const P     *pat,           ///< points to a boost::regex or a string regex for this matcher
      const Input& inp = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      BoostMatcher(pat, inp, opt)
  {
    flg_ |= boost::regex_constants::match_perl;
  }
};

} // namespace reflex

#endif
