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
@file      abslexer.h
@brief     RE/flex abstract lexer base class for reflex-generated scanners
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2017, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_ABSLEXER_H
#define REFLEX_ABSLEXER_H

#include "input.h"
#include <stack>

namespace reflex {

/// The abstract lexer class template that is the abstract root class of all reflex-generated scanners.
/** More info TODO */
template<typename M> /// @tparam <M> matcher class derived from reflex::AbstractMatcher
class AbstractLexer {
 public:
  /// Extend matcher class M with a member pointing to the instantiating lexer class.
  class Matcher : public M {
   public:
    Matcher(
        const typename M::Pattern& pattern,    ///< regex pattern to instantiate matcher class M(pattern, input)
        const Input&               input,      ///< the reflex::Input to instantiate matcher class M(pattern, input)
        AbstractLexer             *lexer,      ///< points to the instantiating lexer class
        const char                *opt = NULL) ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
      :
        M(pattern, input, opt),
        lexer_(lexer)
    { }
   protected:
    /// Returns true if matcher should wrap input after EOF.
    virtual bool wrap(void)
      /// @returns true if reflex::AbstractLexer::wrap() == 0
    {
      return lexer_->wrap() == 0;
    }
    /// Points to the lexer class that instantiated this Matcher.
    AbstractLexer *lexer_;
  };
  /// Construct abstract lexer to scan an input character sequence and echo the text matches to output.
  AbstractLexer(
      const Input&  input, ///< reflex::Input character sequence to read from
      std::ostream& os)    ///< echo the text matches to this std::ostream or to std::cout
    :
      matcher_(NULL),
      in_(input),
      os_(&os),
      start_(),
      debug_(),
      stack_(),
      state_()
  { }
  /// Delete lexer and its current matcher and input.
  virtual ~AbstractLexer(void)
  {
    if (matcher_)
      delete matcher_;
  }
  /// Set debug flag value.
  virtual void set_debug(int flag) ///< 0 or 1 (false or true)
  {
    debug_ = flag;
  }
  /// Get debug flag value.
  virtual int debug(void) const
    /// @returns debug flag value.
  {
    return debug_;
  }
  /// The default wrap operation at EOF: do not wrap input.
  virtual int wrap(void)
    /// @returns 1 (override to return 0 to indicate that new input is available after this invocation so that wrap after EOF is OK).
  {
    return 1;
  }
  /// Start scanning from the given input character sequence.
  AbstractLexer& in(const Input& input) ///< reflex::Input character sequence to scan
    /// @returns reference to *this.
  {
    in_ = input;
    if (has_matcher())
      matcher().in = input; // FIXME or should we also matcher().reset()?
    return *this;
  }
  /// Returns the current input character sequence that is being scanned.
  Input& in(void)
    /// @returns reference to the current reflex::Input object.
  {
    if (has_matcher())
      return matcher().in;
    return in_;
  }
  /// Set the current output to the given output stream to echo text matches to.
  AbstractLexer& out(std::ostream& os) ///< output stream to echo text matches to
    /// @returns reference to *this.
  {
    os_ = &os;
    return *this;
  }
  /// Returns the current output stream used to echo text matches to.
  std::ostream& out(void) const
    /// @returns reference to the current std::ostream object.
  {
    return *os_;
  }
  /// Returns true if a matcher was assigned to this lexer for scanning.
  bool has_matcher(void) const
    /// @returns true if a matcher was assigned.
  {
    return matcher_ != NULL;
  }
  /// Set the matcher (and its current state) for scanning.
  AbstractLexer& matcher(Matcher *matcher) ///< points to a matcher object
    /// @returns reference to *this.
  {
    matcher_ = matcher;
    return *this;
  }
  /// Returns a reference to the current matcher.
  Matcher& matcher(void) const
    /// @returns reference to the current matcher.
  {
    ASSERT(matcher_ != NULL);
    return *matcher_;
  }
  /// Returns a pointer to the current matcher, NULL if none was set.
  Matcher *ptr_matcher(void) const
    /// @returns pointer to the current matcher or NULL if no matcher was set.
  {
    return matcher_;
  }
  /// Returns a new matcher for the given input.
  virtual Matcher *new_matcher(const Input& input) ///< reflex::Input character sequence to match
    /// @returns pointer to new reflex::AbstractLexer::Matcher.
  {
    return new Matcher(matcher().pattern(), input, this);
  }
  /// Delete a matcher.
  void del_matcher(Matcher *matcher)
  {
    if (matcher != NULL)
      delete matcher;
    if (matcher_ == matcher)
      matcher_ = NULL;
  }
  /// Push the current matcher on the stack and use the given matcher for scanning.
  void push_matcher(Matcher *matcher) ///< points to a matcher object
  {
    stack_.push(matcher_);
    matcher_ = matcher;
  }
  /// Pop matcher from the stack and continue scanning where it left off, delete the current matcher.
  void pop_matcher(void)
  {
    ASSERT(!stack_.empty());
    if (matcher_)
      delete matcher_;
    matcher_ = stack_.top();
    stack_.pop();
  }
  /// Echo the matched text to the current output.
  void echo(void) const
  {
    out().write(matcher().text(), matcher().size());
  }
  /// Returns string with the text matched.
  const char *text(void) const
    /// @returns matched text.
  {
    return matcher().text();
  }
  /// Matched text size (length in bytes).
  size_t size(void) const
    /// @returns size of the matched text.
  {
    return matcher().size();
  }
  /// Matched number of (wide) characters (length in multibyte UTF-8 chars).
  size_t wsize(void) const
    /// @returns number of (wide) characters matched.
  {
    return matcher().wsize();
  }
  /// Line number of matched text.
  size_t lineno(void) const
    /// @returns line number.
  {
    return matcher().lineno();
  }
  /// Column number of matched text.
  size_t columno(void) const
    /// @returns column number.
  {
    return matcher().columno();
  }
 protected:
  /// Transition to the given start condition state.
  AbstractLexer& start(int state) ///< start condition state to transition to
    /// @returns reference to *this.
  {
    start_ = state;
    return *this;
  }
  /// Returns the current start condition state.
  int start(void) const
    /// @returns start condition (integer).
  {
    return start_;
  }
  /// Push the current start condition state on the stack and transition to the given start condition state.
  void push_state(int state) ///< start condition state to transition to
  {
     state_.push(start_);
     start_ = state;
  }
  /// Pop the stack start condition state and transition to that state.
  void pop_state(void)
  {
    ASSERT(!state_.empty());
    start_ = state_.top();
    state_.pop();
  }
  /// Returns the stack top start condition state.
  int top_state(void) const
    /// @returns start condition (integer).
  {
    ASSERT(!state_.empty());
    return state_.top();
  }
  Matcher             *matcher_; ///< the matcher used for scanning
  Input                in_;      ///< the input character sequence to scan
  std::ostream        *os_;      ///< the output stream to echo text matches to
  int                  start_;   ///< the current start condition state
  int                  debug_;   ///< 1 if -d (--debug) 0 otherwise:
  std::stack<Matcher*> stack_;   ///< a stack of pointers to matchers
  std::stack<int>      state_;   ///< a stack of start condition states
};

} // namespace reflex

#endif
