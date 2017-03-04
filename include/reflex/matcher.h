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
@copyright (c) 2015-2017, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_MATCHER_H
#define REFLEX_MATCHER_H

#include <reflex/absmatcher.h>
#include <reflex/pattern.h>
#include <stack>

namespace reflex {

/// RE/flex matcher engine class, implements reflex::PatternMatcher pattern matching interface with scan, find, split functors and iterators.
/** More info TODO */
class Matcher : public PatternMatcher<reflex::Pattern> {
 public:
  /// Convert a regex to an acceptable form, given the specified regex library signature `"[decls:]escapes[?+]"`, see reflex::convert.
  template<typename T>
  static std::string convert(T regex, convert_flag_type flags = convert_flag::none)
  {
    return reflex::convert(regex, "imsx#=^:abcdefhijlnprstuvwxzABDHLPQSUW<>?+", flags);
  }
  /// Default constructor.
  Matcher() : PatternMatcher<reflex::Pattern>()
  {
    reset();
  }
  /// Construct matcher engine from a pattern or a string regex, and an input character sequence.
  template<typename P> /// @tparam <P> a reflex::Pattern or a string regex 
  Matcher(
      const P     *pat,           ///< points to a reflex::Pattern or a string regex for this matcher
      const Input& inp = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      PatternMatcher<reflex::Pattern>(pat, inp, opt)
  {
    reset(opt);
  }
  /// Construct matcher engine from a pattern or a string regex, and an input character sequence.
  template<typename P> /// @tparam <P> a reflex::Pattern or a string regex 
  Matcher(
      const P&     pat,            ///< a reflex::Pattern or a string regex for this matcher
      const Input& inp = Input(),  ///< input character sequence for this matcher
      const char   *opt = NULL)    ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
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
  virtual std::pair<const char*,size_t> operator[](size_t n) const
  {
    if (n == 0)
      return std::pair<const char*,size_t>(text(), size());
    return std::pair<const char*,size_t>(NULL, 0);
  }
  /// Returns vector of tab stops.
  const std::vector<size_t>& stops(void) const
    /// @returns vector of size_t.
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
  /// FSM code INIT,
  inline void FSM_INIT(int& c1)
  {
    c1 = fsm_.c1;
  }
  /// FSM code CHAR.
  inline int FSM_CHAR(void)
  {
    return get();
  }
  /// FSM code HALT.
  inline void FSM_HALT(int c1)
  {
    fsm_.c1 = c1;
  }
  /// FSM code TAKE.
  inline void FSM_TAKE(Pattern::Index cap)
  {
    cap_ = cap;
    cur_ = pos_;
  }
  /// FSM code TAKE.
  inline void FSM_TAKE(Pattern::Index cap, int c1)
  {
    cap_ = cap;
    cur_ = pos_;
    if (c1 != EOF)
      --cur_;
  }
  /// FSM code REDO.
  inline void FSM_REDO(void)
  {
    cap_ = Const::EMPTY;
    cur_ = pos_;
  }
  /// FSM code REDO.
  inline void FSM_REDO(int c1)
  {
    cap_ = Const::EMPTY;
    cur_ = pos_;
    if (c1 != EOF)
      --cur_;
  }
  /// FSM code HEAD.
  inline void FSM_HEAD(Pattern::Index la)
  {
    if (lap_.size() <= la)
      lap_.resize(la + 1, -1);
    lap_[la] = static_cast<int>(pos_ - (txt_ - buf_));
  }
  /// FSM code TAIL.
  inline void FSM_TAIL(Pattern::Index la)
  {
    if (lap_.size() > la && lap_[la] >= 0)
      cur_ = txt_ - buf_ + static_cast<size_t>(lap_[la]);
  }
  /// FSM code DENT.
  inline bool FSM_DENT(void)
  {
    if (ded_ > 0)
    {
      fsm_.nul = true;
      return true;
    }
    return false;
  }
  /// FSM code META DED.
  inline bool FSM_META_DED(void)
  {
    return fsm_.bol && dedent(fsm_.col);
  }
  /// FSM code META IND.
  inline bool FSM_META_IND(void)
  {
    return fsm_.bol && indent(fsm_.col);
  }
  /// FSM code META EOB.
  inline bool FSM_META_EOB(int c1)
  {
    return c1 == EOF;
  }
  /// FSM code META BOB.
  inline bool FSM_META_BOB(void)
  {
    return fsm_.bob;
  }
  /// FSM code META EOL.
  inline bool FSM_META_EOL(int c1)
  {
    return c1 == EOF || c1 == '\n';
  }
  /// FSM code META BOL.
  inline bool FSM_META_BOL(void)
  {
    return fsm_.bol;
  }
  /// FSM code META EWE.
  inline bool FSM_META_EWE(int c0, int c1)
  {
    return isword(c0) && !isword(c1);
  }
  /// FSM code META BWE.
  inline bool FSM_META_BWE(int c0, int c1)
  {
    return !isword(c0) && isword(c1);
  }
  /// FSM code META EWB.
  inline bool FSM_META_EWB(void)
  {
    return fsm_.eow;
  }
  /// FSM code META BWB.
  inline bool FSM_META_BWB(void)
  {
    return fsm_.bow;
  }
  /// FSM code META NWE.
  inline bool FSM_META_NWE(int c0, int c1)
  {
    return isword(c0) == isword(c1);
  }
  /// FSM code META NWB.
  inline bool FSM_META_NWB(void)
  {
    return !fsm_.bow && !fsm_.eow;
  }
 protected:
  typedef std::vector<size_t> Stops; ///< indent margin/tab stops
  /// FSM data for FSM code
  struct FSM {
    bool bob;
    bool bol;
    bool bow;
    bool eow;
    bool nul;
    size_t col;
    int c1;
  };
  /// Returns true if input matched the pattern using method Const::SCAN, Const::FIND, Const::SPLIT, or Const::MATCH.
  virtual size_t match(Method method) ///< Const::SCAN, Const::FIND, Const::SPLIT, or Const::MATCH
    /// @returns nonzero if input matched the pattern.
    ;
  /// Update indentation column counter for indent() and dedent().
  void newline(size_t& col) ///< indent column counter
  {
    while (ind_ + 1 < pos_)
      col += buf_[ind_++] == '\t' ? 1 + ((-1 - col) & (opt_.T - 1)) : 1;
    DBGLOG("Newline with indent/dedent? col = %zu", col);
  }
  /// Returns true if looking at indent.
  bool indent(size_t& col) ///< indent column counter
    /// @returns true if indent.
  {
    newline(col);
    return col > 0 && (tab_.empty() || tab_.back() < col);
  }
  /// Returns true if looking at dedent.
  bool dedent(size_t& col) ///< indent column counter
    /// @returns true if dedent.
  {
    newline(col);
    return !tab_.empty() && tab_.back() > col;
  }
  size_t            ded_; ///< dedent count
  Stops             tab_; ///< tab stops set by detecting indent margins
  std::vector<int>  lap_; ///< lookahead position in input that heads a lookahead match (indexed by lookahead number)
  std::stack<Stops> stk_; ///< stack to push/pop stops
  FSM               fsm_; ///< local state for FSM code
};

} // namespace reflex

#endif
