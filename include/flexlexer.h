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
@file      flexlexer.h
@brief     RE/flex Flex-compatible FlexLexer base class and Lex/Flex-compatible macros
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2016, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_FLEXLEXER_H
#define REFLEX_FLEXLEXER_H

#include "abslexer.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Flex-compatibility definitions (redefinable)                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// Flex-compatible user-definable macro.
#ifndef YY_USER_INIT
# define YY_USER_INIT
#endif

/// Flex-compatible user-definable macro.
#ifndef YY_USER_ACTION
# define YY_USER_ACTION
#endif

/// Flex-compatible user-definable macro.
#ifndef YY_BREAK
# define YY_BREAK break;
#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Flex-compatibility definitions (built-in)                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// Flex and Bison-compatible `%option bison` uses global `YY_SCANNER` scanner object, otherwise *this.
#if defined(REFLEX_OPTION_bison_bridge) || (!defined(REFLEX_OPTION_bison) && !defined(REFLEX_OPTION_bison_locations) && !defined(YY_SCANNER))
# define YY_SCANNER (*this)
#endif

/// Flex-compatible macro defining the type of the scanner buffer.
#define yy_buffer_state       reflex::FlexLexer::Matcher

/// Flex-compatible macro defining a pointer to the type of the scanner buffer.
#define YY_BUFFER_STATE       yy_buffer_state*

/// Flex-compatible macro.
#define YY_BUF_SIZE           (16384)

/// Flex-compatible macro defining the type of a state variable.
#define yy_state_type         int

/// Flex-compatible macro.
#define yy_size_t             size_t

/// Flex-compatible macro defining ECHO action to output the content of yytext.
#define ECHO                  YY_SCANNER.LexerOutput(YYText(), YYLeng())

/// Flex-compatible macro defining BEGIN action to set a start condition.
#define BEGIN                 YY_SCANNER.start_ =

/// Flex-compatible macro defining the current start condition.
#define YYSTATE               YY_SCANNER.start()

/// Flex-compatible macro defining the current start condition.
#define YY_START              YY_SCANNER.start()

/// Flex-compatible macro defining the current start condition.
#define yy_current_state      YY_SCANNER.start()

/// Flex-compatible macro defining the current matcher buffer.
#define YY_CURRENT_BUFFER     YY_SCANNER.matcher()

/// Flex-compatible macro defining the buffer flush action.
#define YY_FLUSH_BUFFER       YY_SCANNER.matcher().flush()

/// Flex-compatible macro defining the matched text.
#define yytext                const_cast<char*>(YY_SCANNER.YYText())

/// Flex-compatible macro defining the matched text length.
#define yyleng                static_cast<int>(YY_SCANNER.YYLeng())

/// Flex-compatible macro defining the line number of the matched text.
#define yylineno              static_cast<int>(YY_SCANNER.matcher().lineno())

/// Flex-compatible macro defining the matched action index.
#define yy_act                YY_SCANNER.matcher().accept()

/// Flex-compatible macro defining the current input (reflex::Input can be a stream, file, or string).
#define yyin                  YY_SCANNER.matcher().in

/// Flex-compatible macro defining the current output stream.
#define yyout                 YY_SCANNER.os_

/// Flex-compatible macro defining a at-begin-of-line check.
#define YY_AT_BOL()           YY_SCANNER.matcher().at_bol()

/// Flex-compatible macro defining a begin-of-line forced set.
#define yy_set_bol(b)         YY_SCANNER.matcher().set_bol(b)

/// Flex-compatible macro defining interactive mode on/off.
#define yy_set_interactive(b) YY_SCANNER.matcher().buffer((b) ? 1 : 0)

/// Flex-compatible macro defining the terminating action.
#define yyterminate()         return 0

/// Flex-compatible macro defining the debug flag;
#define yy_flex_debug         YY_SCANNER.debug_;

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Flex-compatibility reentrant definitions                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/// Flex-compatible macro defining reentrant use of yyscanner.
#define yyscanner             this

/// Flex-compatible macro defining the text accessor for reentrant scanner.
#define yyget_text(s)         static_cast<FlexLexer*>(s)->YYText()

/// Flex-compatible macro defining the leng accessor for reentrant scanner.
#define yyget_leng(s)         static_cast<FlexLexer*>(s)->YYLeng()

/// Flex-compatible macro defining the lineno accessor for reentrant scanner.
#define yyget_lineno(s)       static_cast<FlexLexer*>(s)->lineno()

/// Flex-compatible macro defining the in accessor for reentrant scanner.
#define yyget_in(s)           static_cast<FlexLexer*>(s)->matcher().in

/// Flex-compatible macro defining the out accessor for reentrant scanner.
#define yyget_out(s)          static_cast<FlexLexer*>(s)->os_

/// Flex-compatible macro defining the in accessor for reentrant scanner.
#define yyset_in(i, s)        static_cast<FlexLexer*>(s)->matcher().input(i)

/// Flex-compatible macro defining the out accessor for reentrant scanner.
#define yyset_out(o, s)       (static_cast<FlexLexer*>(s)->os_ = o)

/// Flex-compatible macro defining the extra type for reentrant scanner.
#ifdef REFLEX_OPTION_extra_type
#define YY_EXTRA_TYPE         REFLEX_OPTION_extra_type
#else
#define YY_EXTRA_TYPE         void*
#endif

/// Flex-compatible macro defining yyget_extra() for reentrant scanner.
#define yyget_extra(s)        static_cast<FlexLexer*>(s)->yyextra

/// Flex-compatible macro defining yyset_extra() for reentrant scanner.
#define yyset_extra(x, s)     (static_cast<FlexLexer*>(s)->yyextra = x)

namespace reflex {

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  FlexLexer abstract base class template                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
Flex-compatible FlexLexer abstract base class template derived from
reflex::AbstractMatcher for the reflex-generated yyFlexLexer scanner class.

TODO

*/
template<typename M> ///< @tparam <M> matcher class derived from reflex::AbstractMatcher
class FlexLexer : public AbstractLexer<M> {
 public:
  /// Extends reflex::AbstractLexer::Matcher for Flex-compatibility.
  class Matcher : public AbstractLexer<M>::Matcher {
   public:
    Matcher(
        const typename M::Pattern& pattern, ///< regex pattern to instantiate matcher class M(pattern, input)
        const Input&               input,   ///< the reflex::Input to instantiate matcher class M(pattern, input)
        FlexLexer                 *lexer)   ///< points to the instantiating lexer class
      :
        AbstractLexer<M>::Matcher(pattern, input, lexer)
    { }
   protected:
    /// Read character sequences via a Flex-compatible virtual FlexLexer::LexerInput method.
    virtual size_t get(
        char  *s,
        size_t n)
    {
      FlexLexer *lexer = dynamic_cast<FlexLexer*>(this->lexer_);
      assert(lexer != NULL);
      return lexer->LexerInput(s, n); // a nice trick to get input from LexerInput()
    }
    /// Check the Flex-compatible FlexLexer::yywrap method to determine if matcher should wrap input after EOF.
    /// @returns true if FlexLexer::yywrap() == 0
    virtual bool wrap(void)
    {
      FlexLexer *lexer = dynamic_cast<FlexLexer*>(this->lexer_);
      assert(lexer != NULL);
      return lexer->yywrap() == 0;
    }
  };
  /// Construct Flex-compatible lexer to read from a std::istream (and echo the text matches to a std::ostream).
  FlexLexer(
      const Input&  input = stdin, ///< std::istream (stdin by default) to read a character sequence from
      std::ostream *os    = NULL)  ///< echo the text matches to this std::ostream (std::cout by default)
    :
      AbstractLexer<M>(input, os ? *os : std::cout)
  { }
  /// The matched text.
  /// @returns NUL-terminated string.
  const char *YYText(void) const
  {
    return this->matcher().text();
  }
  /// The matched text length.
  /// @returns matched text length.
  size_t YYLeng(void) const
  {
    return this->matcher().size();
  }
  /// Switch to another matcher (and use its buffer).
  void yy_switch_to_buffer(Matcher *m) ///< the matcher to switch to
  {
    this->matcher(m);
  }
  /// Instantiate a new FlexLexer::Matcher and buffer.
  /// @returns pointer to new FlexLexer::Matcher.
  Matcher *yy_create_buffer(
      const Input& input, ///< read from an input source
      int)                ///< unused (Flex compatibility)
  {
    return this->new_matcher(input);
  }
  /// Delete a matcher.
  void yy_delete_buffer(Matcher *m) ///< victim to delete
  {
    this->del_matcher(m);
  }
  /// Restart reading from a stream.
  void yyrestart(const Input& input) ///< read from an input source
  {
    this->in(input);
  }
  /// Push the current matcher on the stack and use a new matcher for scanning.
  void yypush_buffer_state(Matcher *m) ///< new FlexLexer::Matcher for scanning
  {
    this->push_matcher(m);
  }
  /// Pop matcher from the stack and delete the current matcher.
  void yypop_buffer_state(void)
  {
    this->pop_matcher();
  }
  /// Switch input and output streams.
  virtual void switch_streams(
      const Input&  input = stdin,      ///< input to read from
      std::ostream *os    = &std::cout) ///< echo the text matches to this std::ostream
  {
    this->in(input);
    this->out(*os);
  }
  /// Line number of matched text.
  int lineno(void) const
  {
    return this->matcher().lineno();
  }
  /// Default yywrap operation at EOF: do not wrap input.
  /// @returns 1 (0 indicates that new input was set and wrap after EOF is OK).
  virtual int yywrap(void)
  {
    return 1;
  }
  /// Pure virtual lexer (implemented by a reflex-generated yyFlexLexer).
  virtual int yylex(void) = 0;
 protected:
  /// Invoked by FlexLexer::Matcher to read input character sequence.
  /// @returns the nonzero number of (less or equal to n) 8-bit characters added to buffer s from the current input, or zero when EOF.
  virtual size_t LexerInput(
      char  *s, ///< points to the string buffer to fill with input
      size_t n) ///< size of buffer pointed to by s
  {
    return this->matcher().in.get(s, n);
  }
  /// Invoked by ECHO and FlexLexer::output.
  virtual void LexerOutput(
      const char *s, ///< points to text to output
      size_t      n) ///< length of text to output
  {
    this->out().write(s, n);
  }
  /// Invoked by reflex-generated yyFlexLexer when an error occurs.
  virtual void LexerError(const char *s)
  {
    std::cerr << s << std::endl;
    exit(2);
  }
  /// Read one character, returns zero when EOF.
  /// @returns the character read.
  int yyinput(void)
  {
    int c = this->matcher().input();
    return c == EOF ? 0 : c;
  }
  /// Read one character, returns zero when EOF.
  /// @returns the character read.
  int input(void)
  {
    return yyinput();
  }
  /// Put one character back onto the input stream to be read again.
  void unput(char c) ///< char to put back onto the input stream
  {
    this->matcher().unput(c);
  }
  /// Output one character.
  void output(char c) ///< char to output via LexerOutput
  {
    char t[1] = { c };
    LexerOutput(t, 1);
  }
  /// Append the next matched text to the currently matched text.
  void yymore(void)
  {
    this->matcher().more();
  }
  /// Truncate the yytext length of the match to n characters in length and reposition for next match.
  void yyless(int n) ///< truncated string length
  {
    this->matcher().less(n);
  }
  /// Push the current start condition state on the stack and transition to the given start condition state.
  void yy_push_state(
      int   state,       ///< start condition state to transition to
      void *self = NULL) ///< reentrant or self
  {
    (self ? static_cast<FlexLexer*>(self) : this)->push_state(state);
  }
  /// Pop the stack start condition state and transition to that state.
  void yy_pop_state(void *self = NULL) ///< reentrant or self
  {
    (self ? static_cast<FlexLexer*>(self) : this)->pop_state();
  }
  /// Returns the stack top start condition state.
  /// @returns start condition (integer).
  int yy_top_state(void)
  {
    return this->top_state();
  }
  YY_EXTRA_TYPE yyextra;       ///< Flex-compatible reentrant YY_EXTRA_TYPE yyextra
};

} // namespace reflex

#endif
