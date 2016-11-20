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
@file      matcher.h
@brief     RE/flex matcher engine
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2016, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_MATCHER_H
#define REFLEX_MATCHER_H

#include "absmatcher.h"
#include "pattern.h"
#include <stack>

namespace reflex {

/**
RE/flex matcher engine class, implements reflex::PatternMatcher pattern matching interface with scan, find, split functors and iterators.
*/
class Matcher : public PatternMatcher<reflex::Pattern> {
 protected:
  typedef std::vector<size_t> Stops; ///< indent margin/tab stops
 public:
  /// Construct matcher engine from a pattern or a string regex, and an input character sequence.
  template<typename P> ///< @tparam <P> a reflex::Pattern or a string regex 
  Matcher(
      const P&     pat,            ///< a reflex::Pattern or a string regex for this matcher
      const Input& inp = Input(),  ///< input character sequence for this matcher
      const char   *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      PatternMatcher<reflex::Pattern>(pat, inp, opt)
  {
    reset(opt);
  }
  /// Construct matcher engine from a pattern or a string regex, and an input character sequence.
  template<typename P> ///< @tparam <P> a reflex::Pattern or a string regex 
  Matcher(
      const P     *pat,           ///< points to a reflex::Pattern or a string regex for this matcher
      const Input& inp = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      PatternMatcher<reflex::Pattern>(pat, inp, opt)
  {
    reset(opt);
  }
  /// Reset this matcher's state to the initial state.
  virtual void reset(const char *opt = NULL)
  {
    DBGLOG("Matcher::reset()");
    PatternMatcher<reflex::Pattern>::reset(opt);
    ded_ = 0;
    tab_.resize(0);
  }
  /// Returns vector of tab stops.
  /// @return vector of size_t.
  const std::vector<size_t>& stops(void) const
  {
    return tab_;
  }
  /// Clear tab stops.
  void clear_stops(void)
  {
    tab_.clear();
  }
  /// Push stops and clear stops.
  void push_stops(void)
  {
    stk_.push(std::vector<size_t>());
    stk_.top().swap(tab_);
  }
  /// Pop stops.
  void pop_stops(void)
  {
    stk_.top().swap(tab_);
    stk_.pop();
  }
 protected:
  /// Returns true if input matched the pattern using method Const::SCAN, Const::FIND, Const::SPLIT, or Const::MATCH.
  /// @returns nonzero if input matched the pattern.
  virtual size_t match(Method method); ///< Const::SCAN, Const::FIND, Const::SPLIT, or Const::MATCH
  /// Update indentation column counter for indent() and dedent().
  void newline(size_t& col) ///< indent column counter
  {
    while (ind_ + 1 < pos_)
      col += buf_[ind_++] == '\t' ? 1 + (-1 - col & (opt_.T - 1)) : 1;
    DBGLOG("Newline with indent/dedent? col = %zu", col);
  }
  /// Returns true if looking at indent.
  /// @returns true if indent.
  bool indent(size_t& col) ///< indent column counter
  {
    newline(col);
    return col > 0 && (tab_.empty() || tab_.back() < col);
  }
  /// Returns true if looking at dedent.
  /// @returns true if dedent.
  bool dedent(size_t& col) ///< indent column counter
  {
    newline(col);
    return !tab_.empty() && tab_.back() > col;
  }
  size_t            ded_; ///< dedent count
  Stops             tab_; ///< tab stops set by detecting indent margins
  std::vector<int>  lap_; ///< lookahead position in input that heads a lookahead match (indexed by lookahead number)
  std::stack<Stops> stk_; ///< stack to push/pop stops
};

} // namespace reflex

#endif
