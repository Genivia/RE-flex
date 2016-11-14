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
@copyright (c) 2015-2016, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_ABSLEXER_H
#define REFLEX_ABSLEXER_H

#include "input.h"
#include <stack>

namespace reflex {

/**
The abstract lexer class template that is the abstract root class of all reflex-generated scanners.
*/
template<typename M> ///< @tparam <M> matcher class derived from reflex::AbstractMatcher
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
    /// @returns true if reflex::AbstractLexer::wrap() == 0
    virtual bool wrap(void)
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
      debug_(0)
  { }
  /// Delete lexer and its current matcher and input.
  virtual ~AbstractLexer(void)
  {
    if (matcher_)
      delete matcher_;
  }
  /// Set debug flag value.
  virtual void set_debug(int flag) ///< 0 or 1
  {
    debug_ = flag;
  }
  /// Get debug flag value.
  /// @returns debug flag value.
  virtual int debug(void) const
  {
    return debug_;
  }
  /// The default wrap operation at EOF: do not wrap input.
  /// @returns 1 (override to return 0 to indicate that new input is available after this invocation so that wrap after EOF is OK).
  virtual int wrap(void)
  {
    return 1;
  }
  /// Start scanning from the given input character sequence.
  /// @returns reference to *this.
  AbstractLexer& in(const Input& input) ///< reflex::Input character sequence to scan
  {
    in_ = input;
    if (has_matcher())
      matcher().in = input; // FIXME or should we also matcher().reset()?
    return *this;
  }
  /// Returns the current input character sequence that is being scanned.
  /// @returns reference to the current reflex::Input object.
  Input& in(void)
  {
    if (has_matcher())
      return matcher().in;
    return in_;
  }
  /// Set the current output to the given output stream to echo text matches to.
  /// @returns reference to *this.
  AbstractLexer& out(std::ostream& os) ///< output stream to echo text matches to
  {
    os_ = &os;
    return *this;
  }
  /// Returns the current output stream used to echo text matches to.
  /// @returns reference to the current std::ostream object.
  std::ostream& out(void) const
  {
    assert(os_ != NULL);
    return *os_;
  }
  /// Returns true if a matcher was assigned to this lexer for scanning.
  /// @returns true if a matcher was assigned.
  bool has_matcher(void) const
  {
    return matcher_ != NULL;
  }
  /// Set the matcher (and its current state) for scanning.
  /// @returns reference to *this.
  AbstractLexer& matcher(Matcher *matcher) ///< points to a matcher object
  {
    matcher_ = matcher;
    return *this;
  }
  /// Returns the current matcher.
  /// @returns reference to the current matcher.
  Matcher& matcher(void) const
  {
    assert(has_matcher() == true);
    return *matcher_;
  }
  /// Returns a new matcher for the given input.
  /// @returns pointer to new reflex::AbstractLexer::Matcher.
  virtual Matcher *new_matcher(const Input& input) ///< reflex::Input character sequence to match
  {
    return new Matcher(matcher().pattern(), input, this);
  }
  /// Delete a matcher.
  void del_matcher(Matcher *matcher) const
  {
    delete matcher;
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
    assert(stack_.empty() == false);
    delete matcher_;
    matcher_ = stack_.top();
    stack_.pop();
  }
  /// Echo the matched text to the current output.
  void echo(void) const
  {
    out().write(matcher().text(), matcher().size());
  }
  /// Matched text.
  /// @returns matched text.
  const char *text(void) const
  {
    return matcher().text();
  }
  /// Matched text size (length in bytes).
  /// @returns matched text size.
  size_t size(void) const
  {
    return matcher().size();
  }
  /// Line number of matched text.
  /// @returns line number.
  size_t lineno(void) const
  {
    return matcher().lineno();
  }
 protected:
  /// Transition to the given start condition state.
  /// @returns reference to *this.
  AbstractLexer& start(int state) ///< start condition state to transition to
  {
    start_ = state;
    return *this;
  }
  /// Returns the current start condition state.
  /// @returns start condition (integer).
  int start(void) const
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
    start_ = state_.top();
    state_.pop();
  }
  /// Returns the stack top start condition state.
  /// @returns start condition (integer).
  int top_state(void) const
  {
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
