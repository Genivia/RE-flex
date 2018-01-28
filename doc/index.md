
RE/flex user guide                                                  {#mainpage}
==================
                                                               @tableofcontents


  "The asteroid to kill this dinosaur is still in orbit."  -- Lex Manual

  "Optimize: this currently has no effect in Boost.Regex." -- Boost Manual

  "Reflex: a thing that is determined by and reproduces
   the essential features or qualities of something else." -- Oxford Dictionary


What is RE/flex?                                                       {#intro}
================

RE/flex is a flexible scanner-generator framework for generating regex-centric,
Flex-compatible scanners.  The RE/flex command-line tool is compatible with the
Flex command-line tool.  RE/flex accepts standard Flex specification syntax and
supports Flex options.

Features:

- integrated support for Unicode, auto-detects BOM in files (UTF-8/16/32);
- optional "free space mode" improves readability of lexer specifications;
- regular expressions may contain lazy quantifiers;
- regular expressions may contain word boundary anchors;
- regular expressions may contain indent/dedent markers for matching;
- intuitive customization of the C++ lexer class code output;
- efficient matching in direct code or with finite state machine tables;
- visualization of finite state machines;
- generates scanners that are thread-safe by default;
- works with Bison and supports reentrant, bison-bridge and bison-locations;
- includes a regex class library that is extensible;
- Boost.Regex can be used as a regex engine;
- C++11 std::regex can be used as a regex engine (but not with a scanner);
- conversion of regex expressions, for regex engines that lack regex features;
- released under a permissive open source license (BSD-3).

RE/flex is not merely designed to fix the limitations of Flex and Lex!  RE/flex
balances efficiency with flexibility by offering a choice of regex engines that
are used by the generated scanner.  The choice includes Boost.Regex and RE/flex
matcher engines that offer a rich regex syntax.  The RE/flex POSIX matcher adds
lazy quantifiers, word boundary anchors, and other useful patterns to the POSIX
mode of matching.  Also Unicode character sets and ASCII/UTF-8/16/32 file input
is supported by RE/flex, without any additional coding required.  RE/flex regex
patterns are converted to efficient deterministic finite state machines.  These
machines differ from Flex in supporting the new pattern-matching features.

RE/flex incorporates proper object-oriented design principles and does not rely
on macros and globals as Flex does.  Macros and globals are only added when the
Flex-compatibility option `−−flex` is used when invoking the **reflex** scanner
generator.  However, in all cases the **reflex** scanner generator produces C++
scanner classes derived from a base lexer class template, with a matcher engine
as the template parameter.  This offers an extensible approach that permits new
regex matching engines to be included in this framework in the future.

Use the **reflex** scanner generator with two options `−−flex` and `−−bison` to 
output Flex C-compatible code.  These options generate the global non-reentrant
"yy" functions and variables, such as `yylex()` and `yytext`.

In this document we refer to a *regex* as a shorthand for *regular expression*.
Some of you may not agree with this broad use of terminology.  The term regular
expressions refers to the formal concept of *regular languages*, wheras *regex*
refers to backtracking-based regex matching that Perl introduced, that could no
longer be said to be regular in a true mathematical sense.

In summary, RE/flex is

- a *feature-rich replacement* of both [Flex](dinosaur.compilertools.net/#flex)
  and [Lex](dinosaur.compilertools.net/#lex), preserving the compatibility with
  the Bison (Yacc) parser generators;

- an *enhancement* of [Boost.Regex](www.boost.org/libs/regex) to use its engine
  for matching, seaching, splitting and for scanning of tokens on various types
  of data sources, such as strings, files, and streams of unlimited length.

- a stand-alone *regex library* is included with RE/flex for fast matching with
  efficient deterministic finite state machines (FSMs) contructed from patterns
  of POSIX-mode regular expressions that are extended with lazy quantifiers for
  lazy repeats, word boundary anchors, Unicode UTF-8, and much more;

- a *flexible regex framework* that combines the above with a collection of C++
  class templates that are easy to use and that offer a rich API for searching,
  matching, scanning, and splitting of input strings, files and streaming data.
  This framework is flexible and can be extended to include other regex pattern
  matchers that can operate seamlessly with the RE/flex scanner generator.

🔝 [Back to table of contents](#)


Yet another scanner generator                                         {#intro1}
-----------------------------

Lex, Flex and variants are powerful *scanner generators* that generate scanners
(a.k.a. *lexical analyzers* and *lexers*) from lexer specifications.  The lexer
specifications define patterns with user-defined actions that are executed when
their patterns match the input stream.  The scanner repeatedly matches patterns
and triggers these actions until the end of the input stream is reached.

Both Lex and Flex are popular to develop *tokenizers* in which the user-defined
actions emit or return a *token* when the corresponding pattern matches.  These
tokenizers are typically implemented to scan and tokenize the source code for a
compiler or an interpreter of a programming language.  The *regular expression*
patterns in a tokenizer define the make-up of identifiers, constants, keywords,
punctuation, and to skip over white space in the source code that is scanned.

Consider for example the following patterns and associated actions defined in a
lexer specification:

<div class="alt">
```cpp
%%
/* PATTERN */           /* ACTION */
"if"                    return KEYWORD_IF;
"*"                     return OP_MUL;
"="                     return OP_ASSIGN;
[a-zA-Z_][a-zA-Z0-9_]*  return ASCII_IDENTIFIER;
[0-9]+                  return CONST_NUMBER;
\"([^\\"]|\\")*\"       return CONST_STRING;
[ \t\r\n]               /* no action: ignore all white space */
%%
```
</div>

When the tokenizer matches a pattern, the corresponding action is invoked.  The
example above returns tokens to the compiler's parser, which repeatedly invokes
the tokenizer for more tokens until the tokenizer reaches the end of the input.
The tokenizer returns zero (0) when the end of the input is reached.

Lex and Flex have remained relatively stable (inert) tools while the demand has
increased for tokenizing Unicode texts encoded in common wide character formats
such as UTF-8, UCS/UTF-16, and UTF-32.  Lex/Flex still use 8-bit character sets
for regex patterns.  Regex pattern syntax in Lex/Flex is also limited.  No lazy
repetitions.  No word boundary anchors.  No indent and dedent matching.

It is possible, but not trivial to implement scanners with Lex/Flex to tokenize
the source code of more modern programming languages with Unicode-based lexical
structures, such as Java, C#, and C++11.

A possible approach is to use UTF-8 in patterns and reformat the input to UTF-8
for matching.  However, the UTF-8 patterns for common Unicode character classes
are unrecognizable by humans and are prone to errors when written by hand.  The
UTF-8 pattern to match a Unicode letter `\p{L}` is hundreds of lines long!

Furthermore, the regular expression syntax in Lex/Flex is limited to meet POSIX
mode matching constraints.  Scanners should use POSIX mode matching, as we will
explain below.  To make things even more interesting, scanners should avoid the
"greedy trap" when matching input.

Lex/Flex scanners use POSIX pattern matching, meaning that the leftmost longest 
match is returned (among a set of patterns that match the same input).  Because
POSIX matchers produce the longest match for any given input text, we should be
careful when using patterns with "greedy" repetitions (`X*`, `X+` etc.) because
our pattern may gobble up more input than intended.  We end up falling into the
"greedy trap".

To illustrate this trap consider matching HTML comments `<!−− ... −−>` with the
pattern `<!−−.*−−>`.  The problem is that the repetition `X*` is greedy and the
`.*−−>` pattern matches everything until the last `−−>` while moving over `−−>` 
that are between the `<!−−` and the last `−−>`.

@note Dot `.` normally does not match newline `\n` in Lex/Flex patterns, unless
we use *dot-all mode* that is sometimes confusingly called "single line mode".

We can use much more complex patterns such as `<!−−([^−]|−[^−]|−−+[^−>])*−*−−>`
just to match comments in HTML, by ensuring the pattern ends at the first match
of a `−−>` in the input and not at the very last `−−>` in the input.  The POSIX
leftmost longest match can be surprisingly effective in rendering our tokenizer
into works of ASCII art!

We may claim our intricate pattern trophies as high achievements to the project
team, but our team will quickly point out that a regex `<!−−.*?−−>` suffices to
match HTML comments with the *lazy repetition* `X*?` construct, also known as a
*non-greedy repeat*.  The `?` is a *lazy quantifier* that modifies the behavior
of the `X*?` repeat to match only `X` repeately if the rest of the pattern does
not match.  Therefore, the regex `<!−−.*?−−>` matches HTML comments and nothing
more.

But Lex/Flex does not permit us to be lazy!

Not surprising, even the Flex manual shows ad-hoc code rather than a pattern to
scan over C/C++ source code input to match multiline comments that start with a
`/*` and end with the first occurrence of a `*/`.  The Flex manual recommends:

<div class="alt">
```cpp
"/*"    {  /* skip multiline comments */
  int c;
  while ((c = yyinput()) != 0)
  {
    if (c == '\n')
      ++mylineno;
    else if (c == '*')
    {
      if ((c = yyinput()) == '/')
        break;
      unput(c);
    }
  }
}
```
</div>

Another argument to use this code with Flex is that the internal Flex buffer is
limited to 16KB.  By contrast, RE/flex buffers are dynamically resized and will
accept long matches.

Workarounds such as these are not necessary with RE/flex.  The RE/flex scanners
use regex libraries with expressive pattern syntax.  We can use lazy repetition
to write a regex pattern for multiline comments as follows:

<div class="alt">
```cpp
"/*"(.|\n)*?"*/"  /* no action: ignore multiline comments */
```
</div>

Most regex libraries support syntaxes and features that we have come to rely on
for pattern matching.  A regex with lazy quantifiers can be much easier to read
and comprehend compared to a greedy variant.  Most regex libraries that support
lazy quantifiers run in Perl mode, using backtracking over the input.  Scanners
use POSIX mode matching, meaning that the leftmost longest match is found.  The
difference is important as we saw earlier and even more so when we consider the
problems with Perl mode matching when specifying patterns to tokenize input, as
we will explain next.

Consider the lexer specification example shown earlier.  Suppose the input text 
to tokenize is `iflag = 1`.  In POSIX mode we return `ASCII_IDENTIFIER` for the
name `iflag`, `OP_ASSIGN` for `=`, and `NUMBER` for `1`.  In Perl mode, we find
that `iflag` matches `if` and the rest of the name is not consumed, which gives
`KEYWORD_IF` for `if`, `ASCII_IDENTIFIER` for `lag`, `OP_ASSIGN` for `=`, and a
`NUMBER` for `1`.  Perl mode matching greedely returns leftmost matches.

Using Perl mode in a scanner requires all overlapping patterns to be defined in
a lexer specification such that all longest matching patterns are defined first
to ensure longest matches.  By contrast, POSIX mode is *declarative* and allows
you to define the patterns in the specification in any order.  Perhaps the only
ordering constraint on patterns is for patterns that match the same input, such
such as matching the keyword `if` in the example:  `KEYWORD_IF` must be matched
before `ASCII_IDENTIFIER`.

For this reason, RE/flex scanners use a regex library in POSIX mode by default.

In summary, the advantages that RE/flex has to offer include:

- RE/flex is fully compatible with Flex and Bison, by using the `−−flex` and/or
  `−−bison` options.  This eliminates a learning curve to use RE/flex.

- The RE/flex scanner generator accepts specifications that are compatible with
  [Flex](dinosaur.compilertools.net/#flex), with additional options to select a
  matcher engine and to specify names and options for C++ class generation.
  
- The RE/flex scanner generator option `−−bison` generates a scanner compatible
  with [Bison](dinosaur.compilertools.net/#bison).  RE/flex also supports Bison
  bridge (pure reentrant and MT-safe) parsers.

- The regular expression syntax in Flex and Lex specifications is restricted to
  POSIX ERE.  By contrast, the RE/flex specification syntax is regex-centric by
  design and offers a much richer pattern syntax, including lazy quantifiers.

- A matcher engine for a lexer class has a common interface API declared by the
  abstract base matcher class template.

- RE/flex scanners are not implemented as a set of global functions and tables.
  RE/flex scanners are instances of generated lexer classes.  Thus are MT-safe.
  A lexer class is derived from an abstract base lexer class template and it is
  instantiated with a regex matcher engine class that is provided as a template
  parameter.

- Lex and Flex scanners are restricted to ASCII or 8-bit character sets without
  adequate support for Unicode.  RE/flex specifications are extended to support
  Unicode such that the RE/flex scanner generator produces scanners that handle
  UTF-8/16/32 input files.

- Input to matcher engines and scanners is implemented as a class that supports
  streaming sources of input and automatically decodes UTF-encoded files.

- RE/flex scanners can be stand-alone applications, similar to Flex scanners.  

\ref reflex section has more details on the RE/flex scanner generator tool.

In the next part of this manual, we will take a quick look at the RE/flex regex
API that can be used as a stand-alone library for matching, searching, scanning
and splitting input from strings, files and streams in regular C++ applications
(i.e. applications that are not necessarily tokenizers for compilers).

🔝 [Back to table of contents](#)


A flexible regex library                                              {#intro2}
------------------------

The RE/flex regex pattern matching classes include two classes for Boost.Regex,
two classes for C++11 std::regex, and a RE/flex class:

  Engine        | Header file to include  | reflex matcher classes
  ------------- | ----------------------- | -----------------------------------
  RE/flex regex | `reflex/matcher.h`      | `Matcher`
  Boost.Regex   | `reflex/boostmatcher.h` | `BoostMatcher`, `BoostPosixMatcher`
  std::regex    | `reflex/stdmatcher.h`   | `StdMatcher`, `StdPosixMatcher`

The RE/flex `reflex::Matcher` class compiles regex patterns to efficient finite
state machines (FSMs) when instantiated.  These deterministic automata speed up
matching considerably, at the cost of the initial FSM construction (see further
below for hints on how to avoid this run time overhead).

C++11 std::regex supports ECMAScript and AWK POSIX syntax with the `StdMatcher`
and `StdPosixMatcher` classes respectively.  The std::regex syntax is therefore
a lot more limited compared to Boost.Regex and RE/flex.

The RE/flex regex common interface API is implemented in an abstract base class
template `reflex::AbstractMatcher` from which regex matchers are derived.  This
regex API offers a common interface that is used in the generated scanner.  You
can also use this API in your C++ application for pattern matching.

The RE/flex abstract matcher offers four operations for matching with the regex
engines that are derived from this base abstract class:

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `matches()` | returns nonzero if the input from begin to end matches
  `find()`    | search input and return nonzero if a match was found
  `scan()`    | return nonzero if input at current position matches partially
  `split()`   | return nonzero for a split of the input at the next match

These methods return a nonzero value for a match, meaning the `size_t accept()`
value, and are repeatable, where the last three return additional matches.

For example, to check if a string is a valid date:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

// use a BoostMatcher to check if the birthdate string is a valid date
if (reflex::BoostMatcher("\\d{4}-\\d{2}-\\d{2}", birthdate).matches())
  std::cout << "Valid date!" << std::endl;
```

To search a string for words `\w+`:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

// use a BoostMatcher to search for words in a sentence
reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
while (matcher.find() != 0)
  std::cout << "Found " << matcher.text() << std::endl;
```

When executed this code prints:

    Found How
    Found now
    Found brown
    Found cow

The `scan` method is similar to the `find` method, but `scan` matches only from
the current position in the input.  It fails when no partial match was possible
at the current position.  Repeately scanning an input source means that matches
must be continuous, otherwise `scan` returns zero (no match).

The `split` method is roughly the inverse of the `find` method and returns text
located between matches.  For example using non-word matching `\W+`:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

// use a BoostMatcher to search for words in a sentence
reflex::BoostMatcher matcher("\\W+", "How now brown cow.");
while (matcher.split() != 0)
  std::cout << "Found " << matcher.text() << std::endl;
```

When executed this code prints:

    Found How
    Found now
    Found brown
    Found cow
    Found

Note that split also returns the (possibly empty) remaining text after the last
match, as you can see in the output above: the last split with `\W+` returns an
empty string, which is the remaining input after the period in the sentence.

The `find()`, `scan()` and `split()` methods return a nonzero *"accept"* value,
which corresponds to the regex group captured, or the methods return zero if no
match was found.  The methods return 1 for a match when no groups are used.

For example:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

// use a BoostMatcher to tokenize a string
reflex::BoostMatcher matcher("(\\w+)|(\\s+)|(.)", "How now brown cow.");
size_t accept;
while ((accept = matcher.scan()) != 0)
{
  switch (accept)
  {
    case 1: std::cout << "word\n";  break;
    case 2: std::cout << "space\n"; break;
    case 3: std::cout << "other\n"; break;
  }
}
```

When executed this code prints:

    word
    space
    word
    space
    word
    space
    word
    other

The regex engines currently available as classes in the `reflex` namespace are:

  Class               | Mode  | Engine      | Performance
  ------------------- | ----- |------------ | ---------------------------------
  `Matcher`           | POSIX | RE/flex lib | deterministic finite automaton
  `BoostMatcher`      | Perl  | Boost.Regex | regex backtracking
  `BoostPerlMatcher`  | Perl  | Boost.Regex | regex backtracking
  `BoostPosixMatcher` | POSIX | Boost.Regex | regex backtracking
  `StdMatcher`        | ECMA  | std::regex  | regex backtracking
  `StdEcmaMatcher`    | ECMA  | std::regex  | regex backtracking
  `StdPosixMatcher`   | POSIX | std::regex  | regex backtracking

The RE/flex regex engine uses a deterministic finite state machine (FSM) to get
the best performance when matching.  However, constructing a FSM adds overhead.
This matcher is better suitable for searching long texts.  The FSM construction 
overhead can be eliminated by pre-converting the regex to C++ code tables ahead
of time as we will see shortly.

The Boost.Regex engines normally use Perl mode matching.  We added a POSIX mode
Boost.Regex engine class for the RE/flex scanner generator.  Scanners typically
use POSIX mode matching.  See \ref reflex-posix-perl for more information.

The Boost.Regex engines are all initialized with `match_not_dot_newline`, which
disables dotall matching as the default setting.  Dotall can be re-enabled with
the `(?s)` regex mode modifier.  This is done for compatibility with scanners.

A matcher can be applied to strings and wide strings, such as `std::string` and
`std::wstring`, `char*` and `wchar_t*`.  Wide strings are converted to UTF-8 to
enable matching with regular expressions that contain Unicode patterns.

To match Unicode patterns with regex library engines that are 8-bit based or do
not support Unicode, you may want to convert your regex string first before you
use it with a regex matcher engine as follows:

```cpp
#include <reflex/matcher.h> // reflex::Matcher, reflex::Input, reflex::Pattern

// convert a regex with Unicode character classes to create a pattern FSM:
static const std::string regex = reflex::Matcher::convert("[\\p{Greek}\\p{Zs}\\pP]+");
static const reflex::Pattern pattern(regex);

// use a Matcher to check if sentence is in Greek:
if (reflex::Matcher(pattern, sentence).matches())
  std::cout << "This is Greek" << std::endl;
```

This converts the Unicode character classes to UTF-8 for matching with an 8-bit
regex engine.  The `convert` static method differs per matcher class.  An error
`reflex::regex_error` is thrown as an exception if conversion was not possible,
which is unlikely, or if the regex is syntactically incorrect.

Conversion is fast (it runs in linear time in the size of the regex), but it is
not without some overhead.  You can make the converted regex patterns `static`,
as shown above, to eliminate the cost of repeated conversions.

A `reflex::Pattern` object is immutable (it stores a constant table) and can be
shared among threads.

Use `convert` with option `reflex::convert_flag::unicode` to change the meaning
of `.` (dot), `\w`, `\s`, `\l`, `\u`, `\W`, `\S`, `\L`, `\U` character classes.

You can pattern match text in files.  File contents are streamed and not loaded
as a whole into memory, meaning that the data stream is not limited in size and
matching happens immediately.  Interactive mode permits matching the input from
a console (a TTY device generates a potentially endless stream of characters):

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

// use a BoostMatcher to search and display words from console input
reflex::BoostMatcher matcher("\\w+", std::cin);
matcher.interactive();
while (matcher.find() != 0)
  std::cout << "Found " << matcher.text() << std::endl;
```

We can also pattern match text from `FILE` descriptors.  The additional benefit
of using `FILE` descriptors is the automatic decoding of UTF-16/32 input to
UTF-8 by the `reflex::Input` class that manages input sources and their state.

For example, pattern matching the content of "cows.txt" that may use UTF-8, 16,
or 32 encodings:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

// use a BoostMatcher to search and display words from a FILE
FILE *fd = fopen("cows.txt", "r");
if (fd == NULL)
  exit(EXIT_FAILURE);
reflex::BoostMatcher matcher("\\w+", fd);
while (matcher.find() != 0)
  std::cout << "Found " << matcher.text() << std::endl;
```

The `find`, `scan`, and `split` methods are also implemented as input iterators
that apply filtering tokenization, and splitting:

  Iterator range                  | Acts as a | Iterates over
  ------------------------------- | --------- | -------------------------------
  `find.begin()`...`find.end()`   | filter    | all matches
  `scan.begin()`...`scan.end()`   | tokenizer | continuous matches
  `split.begin()`...`split.end()` | splitter  | text between matches

The type `reflex::AbstractMatcher::Operation` is a functor that defines `find`,
`scan`, and `split`.  The functor operation returns true upon success.  The use
of an iterator is simply supported by invoking `begin()` and `end()` methods of
the functor, which return `reflex::AbstractMatcher::iterator`.  Likewise, there
are also `cbegin()` and `cend()` methods that return a `const_iterator`.

We can use these RE/flex iterators in C++ for many tasks, including to populate
containers by stuffing the iterator's text matches into it:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex
#include <vector>                // std::vector

// use a BoostMatcher to convert words of a sentence into a string vector
reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
std::vector<std::string> words(matcher.find.begin(), matcher.find.end());
```

As a result, the `words` vector contains "How", "now", "brown", "cow".

Casting a matcher object to `std::string` is the same as converting `text()` to
a string with `std::string(text(), size())`, which in the example above is done
to construct the `words` vector.  Casting a matcher object to `std::wstring` is
similar, but also converts the UTF-8 `text()` match to a wide string.

RE/flex iterators are useful in C++11 range-based loops.  For example:

```cpp
// Requires C++11, compile with: cc -std=c++11
#include <reflex/stdmatcher.h> // reflex::StdMatcher, reflex::Input, std::regex

// use a StdMatcher to search for words in a sentence using an iterator
for (auto& match : reflex::StdMatcher("\\w+", "How now brown cow.").find)
  std::cout << "Found " << match.text() << std::endl;
```

When executed this code prints:

    Found How
    Found now
    Found brown
    Found cow

And RE/flex iterators are also useful with STL algorithms and lambdas, for
example to compute a histogram of word frequencies:

```cpp
// Requires C++11, compile with: cc -std=c++11
#include <reflex/stdmatcher.h> // reflex::StdMatcher, reflex::Input, std::regex
#include <algorithm>           // std::for_each

// use a StdMatcher to create a frequency histogram of group captures
reflex::StdMatcher matcher("(now)|(cow)|(ow)", "How now brown cow.");
size_t freq[4] = { 0, 0, 0, 0 };
std::for_each(matcher.find.begin(), matcher.find.end(), [&](size_t n){ ++freq[n]; });
```

As a result, the `freq` array contains 0, 1, 1, and 2.

Casting the matcher object to a `size_t` returns the group capture index, which
is used in the example shown above.  We also us it in the example below that is
capturing all regex pattern groupings into a vector:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex
#include <vector>                // std::vector

// use a BoostMatcher to convert captured groups into a numeric vector
reflex::BoostMatcher matcher("(now)|(cow)|(ow)", "How now brown cow.");
std::vector<size_t> captures(matcher.find.begin(), matcher.find.end());
```

As a result, the vector contains the group captures 3, 1, 3, and 2.

Casting the matcher object to `size_t` is the same as invoking `accept()`.

You can use this method and other methods to obtain the details of a match:

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `accept()`  | returns group capture index (or zero if not captured/matched)
  `text()`    | returns `const char*` to 0-terminated match (ends in `\0`)
  `str()`     | returns `std::string` text match (preserves `\0`s)
  `wstr()`    | returns `std::wstring` wide text match (converted from UTF-8)
  `chr()`     | returns first 8-bit character of the text match (`str()[0]`)
  `wchr()`    | returns first wide character of the text match (`wstr()[0]`)
  `pair()`    | returns `std::pair<size_t,std::string>(accept(),str())`
  `wpair()`   | returns `std::pair<size_t,std::wstring>(accept(),wstr())`
  `size()`    | returns the length of the text match in bytes
  `wsize()`   | returns the length of the match in number of wide characters
  `begin()`   | returns `const char*` to non-0-terminated text match begin
  `end()`     | returns `const char*` to non-0-terminated text match end
  `rest()`    | returns `const char*` to 0-terminated rest of input
  `more()`    | tells the matcher to append the next match (adjacent matches)
  `less(n)`   | cuts `text()` to `n` bytes and repositions the matcher
  `lineno()`  | returns line number of the match, starting with line 1
  `columno()` | returns column number of the match, starting with 0
  `first()`   | returns input position of the first character of the match
  `last()`    | returns input position of the last + 1 character of the match
  `at_bol()`  | true if matcher reached the begin of a new line `\n`
  `at_bob()`  | true if matcher is at the start of input, no matches consumed
  `at_end()`  | true if matcher is at the end of input
  `[0]`       | operator returns `std::pair<const char*,size_t>(begin(),size())`
  `[n]`       | operator returns n'th capture `std::pair<const char*,size_t>`

The `operator[n]` takes the group number `n` and returns the n'th group capture
match as a pair with a `const char*` pointer to the group-matching text and the
size of the matched text in bytes.  Because the pointer points to a string that
is not 0-terminated, you should use the size to determine the matching part.

The pointer is NULL when the group capture has no match.

For example:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

// a BoostMatcher to capture name and number:
reflex::BoostMatcher matcher("(\\w+)\\s+(\\d+)");

// use the matcher on a string:
if (matcher.input("cow 123").matches())
  std::cout <<
    "name: " << std::string(matcher[1].first, matcher[1].second) <<
    ", number: " << std::string(matcher[2].first, matcher[2].second) <<
    std::endl;
```

When executed this code prints:

    name: cow, number: 123

@warning The `text()` method returns the match by pointing to the `const char*`
string that is stored in an internal buffer.  This pointer *should not be used*
after matching continues and when the matcher object is deallocated.  To retain
the `text()` value use the `str()` method that returns a copy of `text()`.

@warning The `operator[]` method returns a pair with the match info of the n'th
group, which is a non-0-terminated `const char*` pointer (or NULL) and its size
in bytes of the captured match.  The string *should not be used* after matching
continues.

@note When using the `reflex::Matcher` class, the `accept()` method returns the
accepted pattern among the alternations in the regex that are specified only at
the top level in the regex.  For example, the regex `"(a(b)c)|([A-Z])"` has two
groups, because only the outer top-level groups are recognized.  Because groups
are specified at the top level only, the grouping parenthesis are optional.  We
can simplify the regex to `"a(b)c|[A-Z]"` and still capture the two patterns.

Three special methods can be used to manipulate the input stream directly:

  Method     | Result
  ---------- | ----------------------------------------------------------------
  `input()`  | returns next 8-bit char from the input, matcher then skips it
  `winput()` | returns next wide character from the input, matcher skips it
  `unput(c)` | put char `c` back unto the stream, matcher then takes it
  `peek()`   | returns next 8-bit char from the input without consuming it

The `input()`, `winput()`, and `peek()` methods return a non-negative character
code and EOF (-1) when the end of input is reached.

To initialize a matcher for interactive use, to assign a new input source or to
change its pattern, you can use the following methods:

  Method          | Result
  --------------- | -----------------------------------------------------------
  `input(i)`      | set input to `reflex::Input i` (string, stream, or `FILE*`)
  `pattern(p)`    | set pattern `p` string, `reflex::Pattern` or `boost::regex`
  `has_pattern()` | true if the matcher has a pattern assigned to it
  `own_pattern()` | true if the matcher has a pattern to manage and delete
  `pattern()`     | get the pattern object, `reflex::Pattern` or `boost::regex`
  `buffer()`      | buffer all input at once, returns true if successful
  `buffer(n)`     | set the adaptive buffer size to `n` bytes to buffer input
  `interactive()` | sets buffer size to 1 for console-based (TTY) input
  `flush()`       | flush the remaining input from the internal buffer
  `reset()`       | resets the matcher, restarting it from the remaining input
  `reset(o)`      | resets the matcher with new options string `o` ("A?N?T?")

A `reflex::Input` object represents the source of input for a matcher, which is
either a file `FILE*`, or a string (with UTF-8 character data) of `const char*`
or `std::string` type, or a stream pointer `std::istream*`. The `reflex::Input`
object is implicitly constructed from one of these input sources, for example:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

// set the input source to a string (or a stream or a FILE*)
reflex::Input source = "How now brown cow.";

reflex::BoostMatcher matcher("\\w+", source);

while (matcher.find() != 0)
  std::cout << "Found " << matcher.text() << std::endl;

// use the same matcher with a new source (an Input object):
std::ifstream ifs("cows.txt", std::ifstream::in);
source = ifs;           // Input source is reassignable
matcher.input(source);  // can use ifs as parameter also

while (matcher.find() != 0)
  std::cout << "Found " << matcher.text() << std::endl;

ifs.close();
```

The entire input is buffered in a matcher with `buffer()`, or is read piecemeal
with `buffer(n)`, or is read interactively with `interactive()`.  These methods
should be used after setting the input source.  Reading a stream with buffering
all data is done with the `>>` operator as a shortcut:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

// read and buffer cows.txt file
reflex::BoostMatcher matcher("\<cow\>");
std::ifstream cows("cows.txt", std::ifstream::in);
cows >> matcher;     // same as matcher.input(cows).buffer();
cows.close();        // can already close now because stream content is stored

// count number of 'cow' words:
std::out << std::distance(matcher.find.begin(), matcher.find.end()) << " 'cow' in cows.txt\n";
```

So far we explained how to use `reflex::BoostMatcher` for pattern matching.  We
can also use the RE/flex `reflex::Matcher` class for pattern matching.  The API
is exactly the same.  The `reflex::Matcher` class uses `reflex::Pattern`, which
internally represents an efficient finite state machine that is compiled from a
regex.  These state machines are used for fast matching.

The construction of deterministic finite state machines (FSMs) is optimized but
can take some time and therefore adds overhead before matching can start.  This
FSM construction should not be executed repeatedly if it can be avoided.  So we
recommend to construct static pattern objects to create the FSMs only once:

```cpp
#include <reflex/matcher.h> // reflex::Matcher, reflex::Pattern, reflex::Input

// statically allocate and construct a pattern, i.e. once and for all
static reflex::Pattern word_pattern("\\w+");

// use the RE/flex POSIX matcher to search for words in a string sentence
reflex::Matcher matcher(word_pattern, "How now brown cow.");
while (matcher.find() != 0)
  std::cout << "Found " << matcher.text() << std::endl;
```

A `reflex::Pattern` object is immutable (it stores a constant table) and can be
shared among threads.

The RE/flex matcher only supports POSIX mode matching and does not support Perl
mode matching.  See \ref reflex-posix-perl for more information.

The RE/flex `reflex::Pattern` class has several options that control the regex.
Options and modes for the regex are set as a string, for example:

```cpp
static reflex::Pattern word_pattern("\\w+", "f=graph.gv;f=machine.cpp");
```

The `f=graph.gv` option emits a Graphviz .gv file that can be visually rendered
with the open source [Graphviz dot tool](http://www.graphviz.org) by converting
the deterministic finite state machine (FSM) to PDF, PNG, or other formats:

@dot
digraph FSM {
                size="3,3";
                rankdir=LR;
                concentrate=true;
                node [fontname="ArialNarrow"];
                edge [fontname="Courier"];

                init [root=true,peripheries=0,label="",fontname="Courier"];
                init -> N0x7fff57feb0d0;

/*START*/       N0x7fff57feb0d0 [label=""];
                N0x7fff57feb0d0 -> N0x7fef78c03cb0 [label="0-9"];
                N0x7fff57feb0d0 -> N0x7fef78c03cb0 [label="A-Z"];
                N0x7fff57feb0d0 -> N0x7fef78c03cb0 [label="_"];
                N0x7fff57feb0d0 -> N0x7fef78c03cb0 [label="a-z"];

/*ACCEPT 1*/    N0x7fef78c03cb0 [label="[1]",peripheries=2];
                N0x7fef78c03cb0 -> N0x7fef78c03cb0 [label="0-9"];
                N0x7fef78c03cb0 -> N0x7fef78c03cb0 [label="A-Z"];
                N0x7fef78c03cb0 -> N0x7fef78c03cb0 [label="_"];
                N0x7fef78c03cb0 -> N0x7fef78c03cb0 [label="a-z"];
}
@enddot

The `f=machine.cpp` option emits opcode tables for the finite state machine,
which in this case is the following table of 11 code words:

```cpp
REFLEX_CODE_DECL reflex_code_FSM[11] =
{
  0x617A0005, // 0: GOTO 5 ON a-z
  0x5F5F0005, // 1: GOTO 5 ON _
  0x415A0005, // 2: GOTO 5 ON A-Z
  0x30390005, // 3: GOTO 5 ON 0-9
  0x00FFFFFF, // 4: HALT
  0xFF000001, // 5: TAKE 1
  0x617A0005, // 6: GOTO 5 ON a-z
  0x5F5F0005, // 7: GOTO 5 ON _
  0x415A0005, // 8: GOTO 5 ON A-Z
  0x30390005, // 9: GOTO 5 ON 0-9
  0x00FFFFFF, // 10: HALT
};
```

Option `o` can be used with `f=machine.cpp` to emit optimized native C++ code
for the finite state machine:

```cpp
void reflex_code_FSM(reflex::Matcher& m)
{
  int c0 = 0, c1 = c0;
  m.FSM_INIT(c1);

S0:
  c0 = c1, c1 = m.FSM_CHAR();
  if (97 <= c1 && c1 <= 122) goto S5;
  if (c1 == 95) goto S5;
  if (65 <= c1 && c1 <= 90) goto S5;
  if (48 <= c1 && c1 <= 57) goto S5;
  return m.FSM_HALT(c1);

S5:
  m.FSM_TAKE(1);
  c0 = c1, c1 = m.FSM_CHAR();
  if (97 <= c1 && c1 <= 122) goto S5;
  if (c1 == 95) goto S5;
  if (65 <= c1 && c1 <= 90) goto S5;
  if (48 <= c1 && c1 <= 57) goto S5;
  return m.FSM_HALT(c1);
}
```

The compact FSM opcode tables or the optimized larger FSM code may be used
directly in your code.  This omits the FSM construction overhead at runtime.
You can simply include this generated file in your source code and pass it on
to the `reflex::Pattern` constructor:

```cpp
#include <reflex/matcher.h>   // reflex::Matcher, reflex::Pattern, reflex::Input
#include "machine.cpp" // reflex_code_FSM[]

// use the pattern FSM (opcode table or C++ code) for fast search
static reflex::Pattern pattern(reflex_code_FSM);

// use the RE/flex POSIX matcher to search for words in a string sentence
reflex::Matcher matcher(pattern, "How now brown cow.");
while (matcher.find() != 0)
  std::cout << "Found " << matcher.text() << std::endl;
```

The RE/flex `reflex::Pattern` construction options are given as a string:

  Option        | Effect
  ------------- | -------------------------------------------------------------
  `b`           | bracket lists are parsed without converting escapes
  `e=c;`        | redefine the escape character
  `f=file.cpp;` | save finite state machine code to file.cpp
  `f=file.gv;`  | save deterministic finite state machine to file.gv
  `i`           | case-insensitive matching, same as `(?i)X`
  `l`           | Lex-style trailing context with `/`, same as `(?l)X`
  `m`           | multiline mode, same as `(?m)X`
  `n=name;`     | use `reflex_code_name` for the machine (instead of `FSM`)
  `o`           | only with option `f`: generate optimized FSM native C++ code
  `q`           | Lex-style quotations "..." equal `\Q...\E`, same as `(?q)X`
  `r`           | throw regex syntax error exceptions (not just fatal errors)
  `s`           | dot matches all (aka. single line mode), same as `(?s)X`
  `x`           | free space mode with inline comments, same as `(?x)X`
  `w`           | display regex syntax errors before raising them as exceptions

For example, `reflex::Pattern pattern(pattern, "isr")` enables case-insensitive
dot-all matching with syntax errors thrown as `reflex::Pattern::Error` types of
exceptions.  

In summary:

- RE/flex defines an extensible abstract class interface that offers a standard
  API to use regex matcher engines.  The API is used by the generated scanners.
  The API supports UTF-8/16/32-encoded FILE content, wide strings and streaming
  data.

- RE/flex includes a regex matcher class and a regex pattern class to implement
  fast matching with deterministic finite state machines (FSMs).  The FSM graph
  can be visualized with the Graphviz dot tool.  Furthermore, this FSM can also
  be exported and imported as source code to expedite pattern matching.

\ref regex section has more information about the RE/flex regex library.

🔝 [Back to table of contents](#)


The RE/flex scanner generator                                         {#reflex}
=============================

The RE/flex scanner generator takes a lexer specification and generates a
regex-based C++ lexer class that is saved in lex.yy.cpp, or saved to the file
specified by the `-o` command-line option.  This file is then compiled and
linked with a regex-library to produce a scanner.  A scanner can be a
stand-alone application or part of a larger program such as a compiler:

@dot
digraph build {
  ranksep=.25;
  node     [ shape=box, fontname=Helvetica, fontsize=11 ];
  spec     [ label="lexer specification\n(.l)", peripheries=0 ];
  reflex   [ label="reflex scanner generator" ];
  lexyycpp [ label="lexer class\n(lex.yy.cpp)", peripheries=0 ];
  cpp      [ label="C++ compiler & linker" ];
  scanner  [ label="scanner\n(a.out)", peripheries=0 ];
  libs     [ label="libraries\n(libreflex, libboost_regex)", peripheries=0 ];
  incs     [ label="option --header-file\n(lex.yy.h)", peripheries=0 ];
  spec -> reflex -> lexyycpp -> cpp -> scanner;
  incs -> cpp;
  libs -> cpp;
}
@enddot

The RE/flex-generated scanners use the RE/flex regex library API for pattern
matching.  The RE/flex regex library API is defined by the abstract class
`reflex::AbstractMatcher`.

There are two regex matching engines to choose from for the generated scanner:
the Boost.Regex library (assuming Boost.Regex is installed) or the RE/flex
POSIX matcher engine.  The `libreflex` library should be linked and also
`libboost_regex` when needed.

The input class `reflex::Input` of the `libreflex` library manages input from
strings, wide strings, streams, and data from `FILE` descriptors.  File data
may be encoded in ASCII, binary or in UTF-8/16/32.  UTF-16/32 is automatically
decoded and converted to UTF-8 for UTF-8-based regex matching:

@dot
digraph execute {
  ranksep=.25;
  node     [ shape=box, fontname=Helvetica, fontsize=11 ];
  string   [ label="string/buffer input\n(ASCII, binary, UTF-8)", peripheries=0 ];
  wstring  [ label="wide string input\n(UCS-4)", peripheries=0 ];
  file     [ label="FILE input\n(ASCII, binary, UTF-8/16/32)", peripheries=0 ];
  istream  [ label="istream input\n(cin, fstream, etc.)", peripheries=0 ];
  aout     [ label="scanner\n(a.out)" ];
  actions  [ label="actions & tokens", peripheries=0 ];
  string  -> aout -> actions;
  wstring -> aout;
  file    -> aout;
  istream -> aout;
}
@enddot

The generated scanner executes actions (typically to produce tokens for a
parser).  The actions are triggered by matching patterns to the input.

🔝 [Back to table of contents](#)


The reflex command line tool                                  {#reflex-command}
----------------------------

The **reflex** command takes a lexer specification from standard input or from
the specified file (usually with extension .l, .ll, .l++, .lxx, or .lpp) and
generates a C++ scanner class that is saved to the lex.yy.cpp source code file.

The **reflex** command accepts `−−flex` and `−−bison` options for
compatibility with Flex and Bison/Yacc, respectively.  These options allow
**reflex** to be used as a replacement of the classic Flex and Lex tools:

    $ reflex −−flex −−bison lexerspec.l

The first option `−−flex` specifies that `lexerspec.l` is a classic Flex/Lex
specification with `yytext` or `YYText()` and the usual "yy" variables and
functions.

The second option `−−bison` generates a scanner class and the usual global "yy"
variables and functions for compatibility with non-reentrant
[Bison](dinosaur.compilertools.net/#bison) parsers.  See \ref reflex-bison for
more details on Bison parsers that are reentrant and/or use bison-bridge and
bison-locations options.

The lex.yy.cpp source code output is structured in sections that are clean,
readable, and reusable.  To generate a C++ header file lex.yy.h to include in
your code base, use option `−−header-file`.

🔝 [Back to table of contents](#)


Command-line options                                          {#reflex-options}
--------------------

To control the output of the **reflex** scanner generator use the
command-line options described in the next subsections.  These options can also
be specified in the lexer specification with `%%option` (or as `%%o` for
short):

<div class="alt">
```cpp
%option flex
%option bison
%option graphs-file=mygraph
```
</div>

The above is equivalent to the `−−flex`, `−−bison`, and `−−graphs-file=mygraph`
command-line options.  Multiple options can be grouped on a single line:

<div class="alt">
```cpp
%o flex bison graphs-file=mygraph
```
</div>

Option parameters should be quoted when parameters contain special characters:

<div class="alt">
```cpp
%o flex bison graphs-file="dev/output/mygraph.gv"
```
</div>

Shorter forms can be used, with each option on a separate line:

<div class="alt">
```cpp
%flex
%bison
%graphs-file="dev/output/mygraph.gv"
```
</div>

Options that affect the regular expressions such as `%%option unicode` and
`%%option dotall` should be specified before any regular expressions are
defined and used in the specification.

The scanner code generated by **reflex** defines options as macros
`REFLEX_OPTION_name` with a value of `true` when the option is enabled or with
the value that is assigned to the option.  This allows your program code to
check and use RE/flex options.  For example, the lexer class name is
`REFLEX_OPTION_lexer` when the lexer class name is redefined with option
`−−lexer=NAME`.

🔝 [Back to table of contents](#)

### Scanner options                                   {#reflex-options-scanner}

#### `−+`, `−−flex`

This generates a `yyFlexLexer` scanner class that is compatible with the
Flex-generated `yyFlexLexer` scanner class (assuming Flex with option `−+` for
C++).  The generated `yyFlexLexer` class has the usual `yytext` and other "yy"
variables and functions, as defined by the Flex specification standard.  Without
this option, RE/flex actions should be used that are lexer class methods such
as `text()`, `echo()` and also the lexer's matcher methods, such as
`matcher().more()`, see \ref reflex-spec-rules for more details.

#### `-a`, `−−dotall`

This makes dot (`.`) in patterns match newline.  Normally dot matches a single
character except a newline (`\n` ASCII 0x0A).

#### `-B`, `−−batch`

This generates a batch input scanner that reads the entire input all at once
when possible.  This scanner is fast, but consumes more memory depending on the
input data size.

#### `-f`, `−−full`

(RE/flex matcher only).  This option adds the FSM to the generated code as a
static opcode table, thus generating the scanner in full.  FSM construction
overhead is eliminated when the scanner is initialized, resulting in a scanner
that starts scanning the input immediately.  This option has no effect when
option `−−fast` is used.

#### `-F`, `−−fast`

(RE/flex matcher only).  This option adds the FSM to the generated code as
optimized native C++ code.  FSM construction overhead is eliminated when the
scanner is initialized, resulting in a scanner that starts scanning the input
immediately.  The generated code takes more space compared to the `−−full`
option.

#### `-i`, `−−case-insensitive`

This ignore case in patterns.  Patterns match lower and upper case letters.
This switch only applies to ASCII letters.

#### `-I`, `−−interactive`, `−−always-interactive`

This generates an interactive scanner and permits console input by sacrificing
speed.  By contrast, the default buffered input strategy is more efficient.

#### `-m reflex`, `−−matcher=reflex`

This generates a scanner that uses the RE/flex `reflex::Matcher` class with a
POSIX matcher engine.  This is the default matcher for scanning.  This option
is best for Flex compatibility.  This matcher supports lazy quantifiers,
\ref reflex-pattern-unicode, \ref reflex-pattern-anchor,
\ref reflex-pattern-dents matching, and supports FSM output for visualization
with Graphviz.

#### `-m boost`, `−−matcher=boost`

This generates a scanner that uses the `reflex::BoostPosixMatcher` class with a
Boost.Regex POSIX matcher engine for scanning.  The matcher supports Unicode
and word boundary anchors, but not lazy quantifiers.  No Graphviz output.

#### `-m boost-perl`, `−−matcher=boost-perl`

This generates a scanner that uses the `reflex::BoostPerlMatcher` class with a
Boost.Regex normal (Perl) matcher engine for scanning.  The matching behavior
differs from the POSIX *leftmost longest rule* and results in the first
matching rule to be applied instead of the rule that produces the longest
match.  The matcher supports lazy quantifiers and word boundary anchors.  No
Graphviz output.

#### `−−pattern=NAME`

This defines a custom pattern class NAME for the custom matcher specified with
option `-m`.

#### `−−include=FILE`

This defines a custom include FILE.h to include for the custom matcher
specified with option `-m`.

#### `−−tabs=N`

This sets the tab size to N, where N > 0 must be a power of 2.  The tab size is
used internally to determine the column position for \ref reflex-pattern-dents
matching.  It has no effect otherwise.

#### `-u`, `−−unicode`

This makes `.`, `\s`, `\w`, `\l`, `\u`, `\S`, `\W`, `\L`, `\U` match Unicode.
Also groups UTF-8 sequences in the regex, such that each UTF-8 encoded
character in a regex is properly matched as one wide character.

#### `-x`, `−−freespace`

This switches the **reflex** scanner to *free space mode*.  Regular expressions
in free space mode may contain spacing to improve readability.  Spacing within
regular expressions is ignored, so use `" "` or `[ ]` to match a space and `\h`
to match a space or a tab character.  Actions in free space mode MUST be placed
in `{` and `}` blocks and all other code must be placed in `%{` and `%}`
blocks.  Patterns ending in an escape `\` continue on the next line.

🔝 [Back to table of contents](#)

### Output files options                                {#reflex-options-files}

#### `-o FILE`, `−−outfile=FILE`

This saves the scanner to FILE instead of lex.yy.cpp.

#### `-t`, `−−stdout`

This writes the scanner to stdout instead of to lex.yy.cpp.

#### `−−graphs-file[=FILE]`

(RE/flex matcher only).  This generates a Graphviz file FILE.gv, where FILE is
optional.  When FILE is omitted the **reflex** command generates the file
reflex.S.gv for each start condition state S defined in the lexer
specification.  This includes reflex.INITIAL.gv for the INITIAL start condition
state.  This option can be used to visualize the RE/flex matcher's finite state
machine with the [Graphviz dot](http://www.graphviz.org) tool.  For example:

@dot
digraph INITIAL {
                size="7,7";
                rankdir=LR;
                concentrate=true;
                node [fontname="ArialNarrow"];
                edge [fontname="Courier"];

                init [root=true,peripheries=0,label="INITIAL",fontname="Courier"];
                init -> N0x7fff52a39300;

/*START*/       N0x7fff52a39300 [label=""];
                N0x7fff52a39300 -> N0x7f947940ebf0 [label="\\0-#"];
                N0x7fff52a39300 -> N0x7f947940ecb0 [label="$"];
                N0x7fff52a39300 -> N0x7f947940ebf0 [label="%-\\xff"];

/*ACCEPT 2*/    N0x7f947940ebf0 [label="[2]",peripheries=2];

/*ACCEPT 2*/    N0x7f947940ecb0 [label="[2]",peripheries=2];
                N0x7f947940ecb0 -> N0x7f947940ee30 [label="0-9"];

/*ACCEPT 1*/    N0x7f947940ee30 [label="[1]",peripheries=2];
                N0x7f947940ee30 -> N0x7f947940ef50 [label="."];
                N0x7f947940ee30 -> N0x7f947940ee30 [label="0-9"];

/*STATE*/       N0x7f947940ef50 [label=""];
                N0x7f947940ef50 -> N0x7f947940f070 [label="0-9"];

/*STATE*/       N0x7f947940f070 [label=""];
                N0x7f947940f070 -> N0x7f947940f150 [label="0-9"];

/*ACCEPT 1*/    N0x7f947940f150 [label="[1]",peripheries=2];
}
@enddot

In case you are curious: the specification for this FSM digraph has two
patterns: [1] a pattern to match dollar amounts with the regex
`\$\d+(\.\d{2})?` and [2] the regex `.|\n` to skip a character and advance to
the next match.

#### `−−header-file[=FILE]`

This generates a C++ header file FILE.h that declares the lexer class, in
addition to the generated lexer class code, where FILE is optional.  When FILE
is omitted the **reflex** command generates lex.yy.h.

#### `−−regexp-file[=FILE]`

This generates a text file FILE.txt that contains the scanner's regular
expression patterns, where FILE is optional.  When FILE is omitted the
**reflex** command generates reflex.S.txt for each start condition state S.
The regular expression patterns are converted from the lexer specification and
translated into valid C++ strings that can be used with a regex library for
pattern matching.

#### `−−tables-file[=FILE]`

(RE/flex matcher only).  This generates a C++ file FILE.cpp with the finite state
machine in source code form, where FILE is optional.  When FILE is omitted the
**reflex** command generates reflex.S.cpp for each start condition state S.
This includes the file reflex.INITIAL.cpp for the INITIAL start condition
state.  When this option is used in combination with `−−full` or `−−fast`, the
`reflex::Pattern` is instantiated with the code table defined in this file.
Therefore, when you combine this option with `−−full` or `−−fast` then you
should compile the generated table file with the scanner.  Options `−−full` and
`−−fast` eliminate the FSM construction overhead when the scanner is
initialized.

🔝 [Back to table of contents](#)

### Output code options                                  {#reflex-options-code}

#### `−−namespace=NAME`

This places the generated scanner class in the C++ namespace NAME scope.  Which
means `NAME::Lexer` (and `NAME::yyFlexLexer` when option `−−flex` is used).

#### `−−lexer=NAME`

This defines the NAME of the generated scanner class and replaces the default
name `Lexer` (and replaces `yyFlexLexer` when option `−−flex` is used).

#### `−−lex=NAME`

This defines the NAME of the generated scanner function to replace the function
name `lex()` (and `yylex()` when option `−−flex` is used).

#### `−−class=NAME`

This defines the NAME of the scanner class that should be derived from the
generated base `Lexer` class.  Use this option when defining your own scanner
class named NAME.  You can declare a custom lexer class in the first section of
the lexer specification.  Because the custom lexer class is user-defined,
**reflex** generates the implementation of the `lex()` scanner function for
this specified class.

#### `−−yyclass=NAME`

This option combines options `−−flex` and `−−class=NAME`.

#### `−−main`

This generates a `main` function to create a stand-alone scanner that scans
data from standard input (using `stdin`).

#### `-L`, `−−noline`

This suppresses the `#line` directives in the generated scanner code.

#### `-P NAME`, `−−prefix=NAME`

This specifies NAME as a prefix for the generated `yyFlexLexer` class to
replace the default `yy` prefix.  Also renames the prefix of `yylex()`.
Generates `lex.NAME.cpp` file instead of `lex.yy.cpp`, and generates
`lex.NAME.h` with option `−−header-file`.

#### `−−nostdinit`

This initializes input to `std::cin` instead of using `stdin`.  Automatic
UTF decoding is not supported.  Use `stdin` for automatic UTF BOM detection and
UTF decoding.

#### `−−bison`

This generates a scanner that works with Bison parsers, by defining global
(non-MT-safe and non-reentrant) "yy" variables and functions.  See
\ref reflex-bison for more details.  Use option `−−noyywrap` to remove the
dependency on the global `yywrap()` function.

#### `−−bison-bridge`

This generates a scanner that works with Bison pure (MT-safe and reentrant)
parsers using a Bison bridge for one ore more scanner objects.  See \ref
reflex-bison for more details.

#### `−−bison-locations`

This generates a scanner that works with Bison with locations enabled.  See
\ref reflex-bison for more details.

#### `-R`, `−−reentrant`

This generates additional Flex-compatible `yylex()` reentrant scanner
functions.  RE/flex scanners are always reentrant, assuming that `%%class`
variables are used instead of the user declaring global variables.  This is a
Flex-compatibility option and should only be used with options `−−flex` and
`−−bison`.  With this option enabled, most Flex functions take a `yyscan_t`
scanner as an extra last argument.  See \ref reflex-reentrant and also
\ref reflex-bison.

#### `−−yywrap` and `−−noyywrap`

Option `−−yywrap` generates a scanner that calls the global `int yywrap()`
function when EOF is reached.  Only applicable when `−−flex` is used for
compatibility and when `−−flex`  and `−−bison` are used together.  Use
`−−noyywrap` to disable the dependence on this global function.  This option
has no effect for C++ lexer classes, which have a virtual `int wrap()` (or
`yywrap()` with option `−−flex`) method that can be overridden.

🔝 [Back to table of contents](#)

### Debugging options                                   {#reflex-options-debug}

#### `-d`, `−−debug`

This enables debug mode in the generated scanner.  Running the scanner produces
debug messages on `std::cerr` standard error and the `debug()` function returns
nonzero.  To temporarily turn off debug messages, use `set_debug(0)` in your
action code.  To turn debug messages back on, use `set_debug(1)`.  The
`set_debug()` and `debug()` methods are virtual methods of the lexer class, so
you can override their behavior in a derived lexer class.  This option also
enables assertions that check for internal errors.

#### `-p`, `−−perf-report`

This enables the collection and reporting of statistics by the generated
scanner.  The scanner reports the performance statistics on `std::cerr` when
EOF is reached.  If your scanner does not reach EOF, then invoke the lexer's
`perf_report()` method explicitly in your code.  Invoking this method also
resets the statistics and timers, meaning that this method will report the
statistics collected since it was last called.

#### `-s`, `−−nodefault`

This suppresses the default rule that ECHOs all unmatched text when no rule
matches.  With the `−−flex` option, the scanner reports "scanner jammed" when
no rule matches.  Without the `−−flex` option, unmatched input is silently
ignored.

#### `-v`, `−−verbose`

This displays a summary of scanner statistics.

#### `-w`, `−−nowarn`

This disables warnings.

🔝 [Back to table of contents](#)

### Miscellaneous options                       {#reflex-options-miscellaneous}

#### `-h`, `−−help`

This displays helpful information about **reflex**.

#### `-V`, `−−version`

This displays the current **reflex** release version.

#### `−−yylineno`, `−−yymore`

These options are enabled by default and have no effect.

🔝 [Back to table of contents](#)


Lexer specifications                                             {#reflex-spec}
--------------------

A lexer specification consists of three sections that are divided by `%%`
delimiters that are placed on a single line:

<div class="alt">
```cpp
Definitions
%%
Rules
%%
User code
```
</div>

\ref reflex-spec-defs is used to define named regex patterns, to set options
for the scanner, and for including C++ declarations.

\ref reflex-spec-rules is the main workhorse of the scanner and consists of patterns
and actions, where patterns may use named regex patterns that are defined in
\ref reflex-spec-defs.  The actions are executed when patterns match.  For
example, the following lexer specification replaces all occurrences of `cow` by
`chick` in the input to the scanner:

<div class="alt">
```cpp
%%
cow      out() << "chick";
%%
```
</div>

The default rule is to echo any input character that is read from input that
does not match a rule in \ref reflex-spec-rules, so all other text is faithfully
reproduced by this simple scanner example.

Because the pattern `cow` also matches words partly we get `chicks` for `cows`.
But we also get badly garbled output for words such as `coward` and we are
skipping capitalized Cows.  We can improve this with a pattern that anchors
word boundaries and accepts a lower or upper case C:

<div class="alt">
```cpp
cow      \<[Cc]ow\>
%%
{cow}    out() << text()[0] << "hick";
%%
```
</div>

Note that we defined a named pattern `cow` in \ref reflex-spec-defs to match
the start and end of a "cow" or capitalized "Cow" with the regex `\<[Cc]ow\>`.
We use `{cow}` in our rule for matching.  The matched text first character is
emitted with `text()[0]` and we simply append a `"hick"` to complete our chick.

Note that regex grouping with parenthesis to capture text matched by a
parenthesized sub-regex is generally not supported by scanner generators, so we
have to use the entire matched `text()` string.

Flex and Lex do not support word boundary anchors `\<`, `\>`, `\b`, and `\B` so
this example only works with RE/flex.

If you are wondering about the action code in our example not exactly
reflecting the C code expected with Flex, then rest assured that RE/flex
supports the classic Flex and Lex actions such as `yytext` instead of `text()`
and `*yyout` instead of `out()`.  Simply use option `−−flex` to regress to the
C-style Flex names and actions.  Use options `−−flex` and `−−bison` to regress
even further to generated a global `yylex()` function and "yy" variables.

To create a stand-alone scanner, we add `main` to the User code section:

<div class="alt">
```cpp
cow      \<[Cc]ow\>
%%
{cow}    out() << text()[0] << "hick";
%%
int main() { return Lexer().lex(); }
```
</div>

The main function instantiates the lexer class and invokes the scanner, which
will not return until the entire input is processed.  In fact, you can let
**reflex** generate this main function for you with option `−−main`.

More details on these three lexer specification sections is presented next.

🔝 [Back to table of contents](#)

### The definitions section                                 {#reflex-spec-defs}

The Definitions section includes name-pattern pairs to define names for
patterns.  Named patterns can be referenced in regex patterns by embracing them
in `{` and `}`.

The following example defines two names for two patterns, where the second
regex pattern uses the previously named pattern:

<div class="alt">
```cpp
digit     [0-9]
number    {digit}+
```
</div>

Patterns ending in an escape `\` continue on the next line with optional line
indentation.  This permits you to organize your layout of long patterns.  See
also \ref reflex-pattern-freespace to improve pattern readability. 

Names must be defined before being referenced.  Names are expanded as macros in
regex patterns.  For example, `{digit}+` is expanded into `[0-9]+`.

@note This is not entirely accurate: when a name is expanded to its right-hand
side pattern `φ` then the expanded pattern `φ` is placed in a non-capturing
group `(?:φ)` to preserve its structure.  For example, `{number}` expands to
`(?:{digit}+)` which in turn expands to `(?:(?:[0-9])+)`.

To inject code into the generated scanner, indent the code or place the code
within a `%{` and `%}`.  The `%{` and `%}` should be placed at the start of a
new line.  To inject code at the very top of the generated scanner, place
this code within `%%top{` and `%}`:

<div class="alt">
```cpp
%top{
  #include <iostream>    // std::cout etc.
%}
```
</div>

\ref reflex-spec-defs may also contain one or more options with `%%option`
(or `%%o` for short).  For example:

<div class="alt">
```cpp
%option dotall main
%o matcher=boost
```
</div>

Multiple options can be grouped on the same line as is shown above.  See
\ref reflex-options for a list of available options.

@note Options `freespace`, `case-insensitive`, `dotall`, and `unicode` affect
the named patterns defined in \ref reflex-spec-defs.  Therefore, we should
place these options ahead of all named patterns.  If a regex pattern
specifically requires one or more of these options, use the `(?isux:φ)`
modifier(s), see \ref reflex-patterns for details.

Consider the following example.  Say we want to count the number of occurrences
of the word "cow" in some text.  We declare a global counter, increment the
counter when we see a "cow", and finally report the total tally when we reach
the end of the input marked by the `<<EOF>>` rule:

<div class="alt">
```cpp
%option dotall main

%top{
  #include <iostream>    // std::cout etc.
%}

%{
  static int herd = 0;   // a global static variable to count cows
%}

cow        \<[Cc]ow\>

%%

{cow}      herd++;       // found a cow, bump count by one
.          // do nothing
<<EOF>>    out() << herd << " cows!" << std::endl; return 0;

%%
```
</div>

The above works fine, but we are using a global counter which is not a best
practice and is not MT-safe or reentrant: multiple Lexer class instances may
compete to bump the counter.  Another problem is that the Lexer can only be
used once, there is no proper initialization to restart the Lexer on new input.

RE/flex allows you to inject code in the generated Lexer class, meaning that
class members and constructor code can be added to manage the Lexer class
state.  All Lexer class members are visible in actions, even when private.  New
Lexers can be instantiated given some input to scan.  Lexers can run in
parallel in threads without requiring synchronization when their state is part
of the instance and not managed by global variables.

To inject Lexer class member declarations such as variables and methods, place
the declarations within `%%class{` and `%}`.  The `%%class{` and `%}` should be
placed at the start of a new line.

Likewise, to inject Lexer class constructor code, for example to initialize
members, place the code within `%%init{` and `%}`.  The `%%init{` and `%}`
should be placed at the start of a new line.

Additional constructors and/or a destructor may be placed in `%%class{` and
`%}`, using the class name `Lexer` (or `yyFlexLexer` with option `−−flex`),
unless the class is renamed with option `−−lexer=NAME` (`%%option lexer=NAME`).
For convenience you can use the `REFLEX_OPTION_lexer` macro in your code that
expands to the class name.  To do so, use **reflex** option `−−header-file` to
generate a header file to include in your code.

For example, we use these code injectors to make our cow counter `herd` part of
the Lexer class state:

<div class="alt">
```cpp
%option dotall main

%top{
  #include <iostream>    // std::cout etc.
%}

%class{
  int herd;  // lexer class member variable (private by default)
%}

%init{
  herd = 0;  // initialize member variable in Lexer class constructor
%}

cow        \<[Cc]ow\>

%%

{cow}      herd++;       // found a cow, bump count by one
.          // do nothing
<<EOF>>    out() << herd << " cows!" << std::endl; return 0;

%%
```
</div>

Note that nothing else needed to be changed, because the actions are part of
the generated Lexer class and can access the Lexer class members, in this
example the member variable `herd`.

To modularize specifications of lexers, use `%%include` (or `%%i` for short) to
include files into \ref reflex-spec-defs of a specification.  For example:

<div class="alt">
```cpp
%include "examples/jdefs.l"
```
</div>

This includes examples/jdefs.l with Java patterns into the current
specification so you can match Java lexical structures, such as copying Java
identifiers to the output given some Java source program as input:

<div class="alt">
```cpp
%include "examples/jdefs.l"
%%
{Identifier}    echo();
.|\n            // do nothing
%%
```
</div>

To declare start condition state names use `%%state` (`%%s` for short) to
declare inclusive states and use `%%xstate` (`%%x` for short) to declare
exclusive states:

<div class="alt">
```cpp
%s INCLUSIVE
%x EXCLUSIVE
```
</div>

See \ref reflex-states for more information about states.

🔝 [Back to table of contents](#)

### The rules section                                      {#reflex-spec-rules}

Each rule in the rules section consists of a pattern-action pair.  For example,
the following defines an action for a pattern:

<div class="alt">
```cpp
{number}    out() << "number " << text() << std::endl;
```
</div>

To add action code that spans multiple lines, indent the code or place the code
within a `{` and `}` code block.  When local variables are declared in an
action then the code should always be placed in a code block.

In free space mode you MUST place actions in `{` and `}` blocks and other code
in `%{` and `%}` instead of indented.

Actions in \ref reflex-spec-rules can use predefined RE/flex variables and
functions.  With **reflex** option `−−flex`, the variables and functions are
the classic Flex actions shown in the second column of this table:

  RE/flex action       | Flex action          | Result
  -------------------- | -------------------- | -------------------------------
  `text()`             | `YYText()`, `yytext` | 0-terminated text match
  `str()`              | *n/a*                | `std::string` text match
  `wstr()`             | *n/a*                | `std::wstring` wide text match
  `chr()`              | `yytext[0]`          | first 8-bit char of text match
  `wchr()`             | *n/a*                | first wide char of text match
  `size()`             | `YYLeng()`, `yyleng` | size of the match in bytes
  `wsize()`            | *n/a*                | number of wide chars matched
  `lineno()`           | `yylineno`           | line number of match (>=1)
  `columno()`          | *n/a*                | column number of match (>=0)
  `echo()`             | `ECHO`               | `out().write(text(), size())`
  `in(i)`              | `yyrestart(i)`       | set input to `reflex::Input i`
  `in()`, `in() = &i`  | `*yyin`, `yyin = &i` | get/set `reflex::Input` object
  `out(o)`             | `yyout = &o`         | set output to `std::ostream o`
  `out()`              | `*yyout`             | get `std::ostream` object
  `out().write(s, n)`  | `LexerOutput(s, n)`  | output chars `s[0..n-1]`
  `out().put(c)`       | `output(c)`          | output char `c`
  `start(n)`           | `BEGIN n`            | set start condition to `n`
  `start()`            | `YY_START`           | get current start condition
  `push_state(n)`      | `yy_push_state(n)`   | push current state, start `n`
  `pop_state()`        | `yy_pop_state()`     | pop state and make it current
  `top_state()`        | `yy_top_state()`     | get top state start condition
  `matcher().accept()` | `yy_act`             | number of the matched rule
  `matcher().text()`   | `YYText()`, `yytext` | same as `text()`
  `matcher().str()`    | *n/a*                | same as `str()`
  `matcher().wstr()`   | *n/a*                | same as `wstr()`
  `matcher().chr()`    | `yytext[0]`          | same as `chr()`
  `matcher().wchr()`   | *n/a*                | same as `wchr()`
  `matcher().size()`   | `YYLeng()`, `yyleng` | same as `size()`
  `matcher().wsize()`  | *n/a*                | same as `wsize()`
  `matcher().begin()`  | *n/a*                | non-0-terminated text match
  `matcher().end()`    | *n/a*                | non-0-terminated text match end
  `matcher().input()`  | `yyinput()`          | get next 8-bit char from input
  `matcher().winput()` | *n/a*                | get wide character from input
  `matcher().unput(c)` | `unput(c)`           | put back 8-bit char `c`
  `matcher().peek()`   | *n/a*                | peek at next 8-bit char on input
  `matcher().more()`   | `yymore()`           | append next match to this match
  `matcher().less(n)`  | `yyless(n)`          | shrink match length to `n`
  `matcher().first()`  | *n/a*                | first pos of match in input
  `matcher().last()`   | *n/a*                | last pos+1 of match in input
  `matcher().rest()`   | *n/a*                | get rest of input until end
  `matcher().at_bob()` | *n/a*                | true if at the begin of input
  `matcher().at_end()` | *n/a*                | true if at the end of input
  `matcher().at_bol()` | `YY_AT_BOL()`        | true if at begin of a newline
  `set_debug(n)`       | `set_debug(n)`       | reflex option `-d` sets `n=1`
  `debug()`            | `debug()`            | nonzero when debugging

Note that Flex `switch_streams(i, o)` is the same as invoking the `in(i)` and
`out(o)` methods.  Flex `yyrestart(i)` is the same as invoking `in(i)` to set
input to a file, stream, or string.  Invoking `switch_streams(i, o)` and
`in(i)` also resets the lexer's matcher (internally with `matcher.reset()`).
This clears the line and column counters, resets the internal anchor and
boundary flags for anchor and word boundary matching, and resets the matcher to
consume buffered input.

You can also set the input with `in() = &i` (or `yyin = &i)` with option
`−−flex`).  This does not reset the matcher.  This means that when an end of
input (EOF) was reached, you should clear the EOF state with
`matcher().set_end(false)` or reset the matcher state with `matcher().reset()`.  

The `matcher().input()`, `matcher().winput()`, and `matcher().peek()` methods
return a non-negative character code and EOF (-1) when the end of input is
reached.  These methods preserve the current `text()` match (and `yytext` with
option `−−flex`), but the pointer returned by `text()` (and `yytext`) may
change after these methods are called.  However, the `yytext` pointer is not
preserved when using these methods with **reflex** options `−−flex` and
`−−bison`.

@warning The Flex-compatible `yyinput()` function returns 0 when the end of
input is reached, which makes it impossible to distinguish `\0` (NUL) from EOF.
By contrast, `matcher().input()` returns EOF (-1) when the end of the input is
reached.

@warning Do not invoke `matcher()` before `lex()` (or `yylex()` with option
`−−flex`) is invoked!  A matcher is not initially assigned to the lexer when it
is constructed.

Use **reflex** options `−−flex` and `−−bison` to enable global Flex actions and
variables.  This makes Flex actions and variables globally accessible outside
of \ref reflex-spec-rules, with the exception of `yy_push_state()`,
`yy_pop_state()`, `yy_top_state()`.  Outside \ref reflex-spec-rules you must
use the global action `yyinput()` instead of `input()`, global action
`yyunput()` instead of `unput()`, and global action `yyoutput()` instead of
`output()`.  Because `yyin` and `yyout` are macros they cannot be (re)declared
or accessed as global variables, but they can be used as if these were
variables.  To avoid compilation errors, use **reflex** option `−−header-file`
to generate a header file `lex.yy.h` to include in your code to use the global
use Flex actions and variables.  See \ref reflex-bison for more details on the
`−−bison` options to use.

When using **reflex** options `−−flex`, `−−bison` and `−−reentrant`, most Flex
functions take a `yyscan_t` scanner as an extra last argument.  See
\ref reflex-reentrant for details.

From the first couple of entries in the table shown above you may have guessed
correctly that `text()` is just a shorthand for `matcher().text()`, since
`matcher()` is the matcher object associated with the generated Lexer class.
The same shorthands apply to `str()`, `wstr()`, `size()`, `wsize()`, `lineno()`
and `columno()`.  Use `text()` for fast access to the matched text.  The
`str()` method returns a string copy of the match and is less efficient.

Because `matcher()` returns the current matcher object, the following Flex-like
actions are also supported:

  RE/flex action            | Flex action             | Result
  ------------------------- | ----------------------- | -----------------------
  `matcher().buffer()`      | *n/a*                   | buffer entire input
  `matcher().buffer(n)`     | *n/a*                   | set buffer size to `n`
  `matcher().interactive()` | `yy_set_interactive(1)` | set interactive input
  `matcher().flush()`       | `YY_FLUSH_BUFFER`       | flush input buffer
  `matcher().get(s, n)`     | `LexerInput(s, n)`      | read `s[0..n-1]`
  `matcher().set_bol(b)`    | `yy_set_bol(b)`         | set begin of line
  `matcher().set_end(b)`    | *n/a*                   | set EOF flag to `b`
  `matcher().reset()   `    | *n/a*                   | reset the state as new

You can switch to a new matcher while scanning input, and use operations to
create a new matcher, push/pop a matcher on/from a stack, and delete a matcher:

  RE/flex action    | Flex action              | Result
  ----------------- | ------------------------ | ------------------------------
  `matcher(m)`      | `yy_switch_to_buffer(m)` | use matcher `m`
  `new_matcher(i)`  | `yy_create_buffer(i, n)` | new matcher `reflex::Input i`
  `del_matcher(m)`  | `yy_delete_buffer(m)`    | delete matcher `m`
  `push_matcher(m)` | `yypush_buffer_state(m)` | push current matcher, use `m`
  `pop_matcher()`   | `yypop_buffer_state()`   | pop matcher and delete current
  `ptr_matcher()`   | `YY_CURRENT_BUFFER`      | pointer to current matcher

The matcher type `m` is a Lexer class-specific `Matcher` type, which depends on
the underlying matcher used by the scanner.  Therefore, `new_matcher(i)`
instantiates a `reflex::Matcher` or `reflex::BoostPosixMatcher` depending on
the `−−matcher` option.

The following Flex functions are also supported with **reflex** option
`−−flex` and take `std::string` and `char*` arguments:

  Flex action                   | Result
  ----------------------------- | ---------------------------------------------
  `yy_scan_string(string)`      | scan `string`
  `yy_scan_buffer(string, len)` | scan `string` upto length `len`
  `yy_scan_bytes(bytes, len)`   | scan `bytes` upto length `len`

In addition, the following wide string versions take `std::wstring` and
`wchar_t*` arguments:
  
  RE/flex action                 | Result
  ------------------------------ | --------------------------------------------
  `yy_scan_wstring(string)`      | scan wide `string`
  `yy_scan_wbuffer(string, len)` | scan wide `string` upto length `len`

These functions create a new buffer (i.e. a new matcher in RE/flex).  A pointer
to the new buffer is returned, which becomes the `YY_CURRENT_BUFFER`.  You
should delete this new buffer with `yy_delete_buffer(YY_CURRENT_BUFFER)` when
you are done with it.

These functions take an extra last `yyscan_t` argument for reentrant Flex
scanners generated with option `−−reentrant`.

The generated scanner reads from the standard input by default or from an input
source specified as a `reflex::Input` object, such as a string, wide string,
file, or a stream.

See \ref reflex-input for more details on managing the input to a scanner.

🔝 [Back to table of contents](#)

### User code sections                                      {#reflex-spec-user}

To inject code at the end of the generated scanner, such as a `main`
function, we can use the third and final User code section.  All of the code in
the User code section is copied to the generated scanner.

Below is a User code section example with `main` that invokes the lexer to read
from standard input (the default input) and display all numbers found:

<div class="alt">
```cpp
%top{
  #include <iostream>
%}

digit       [0-9]
number      {digit}+

%%

{number}    out() << "number " << text() << std::endl;

%%

int main() { return Lexer().lex(); }
```
</div>

You can also automatically generate a `main` with the **reflex** `−−main`
option, which will produce the same `main` function shown in the example above.
This creates a stand-alone scanner that instantiates a Lexer that reads input
from standard input.

To scan from other input than standard input, such as from files, streams, and
strings, instantiate the Lexer class with the input source as the first
argument.  To set an alternative output stream than standard output, pass a
`std::ostream` object as the second argument to the Lexer class constructor:

<div class="alt">
```cpp
int main(int argc, char **argv)
{
  FILE *fd = stdin;
  if (argc > 1 && (fd = fopen(argv[1], "r")) == NULL)
    exit(EXIT_FAILURE);
  std::ofstream of("output.txt", std::ofstream::out);
  if (!of)
    exit(EXIT_FAILURE);
  Lexer(fd, of).lex();
  of.close();
  if (fd != stdin)
    fclose(fd);
  return 0;
}
```
</div>

The above uses a `FILE` descriptor to read input from, which has the advantage
of automatically decoding UTF-8/16/32 input.  Other permissible input sources
are `std::istream`, `std::string`, `std::wstring`, `char*`, and `wchar_t*`.

🔝 [Back to table of contents](#)


Patterns                                                     {#reflex-patterns}
--------

The regex pattern syntax you can use generally depends on the regex matcher
library that you use.  Fortunately, RE/flex accept a broad pattern syntax for
lexer specifications.  The **reflex** command internally converts the regex
patterns to regex forms that the underlying matcher engine library can handle
(except when specifically indicated in the tables that follow).  This ensures
that the same pattern syntax can be used with any matcher engine library that
RE/flex currently supports.

🔝 [Back to table of contents](#)

### Pattern syntax                                     {#reflex-pattern-syntax}

A pattern is an extended set of regular expressions, with nested sub-expression
patterns `φ` and `ψ`:

  Pattern   | Matches
  --------- | -----------------------------------------------------------------
  `x`       | matches the character `x`, where `x` is not a special character
  `.`       | matches any single character except newline (unless in dotall mode)
  `\.`      | matches `.` (dot), special characters are escaped with a backslash
  `\n`      | matches a newline, others are `\a` (BEL), `\b` (BS), `\t` (HT), `\v` (VT), `\f` (FF), and `\r` (CR)
  `\0`      | matches the NUL character
  `\cX`     | matches the control character `X` mod 32 (e.g. `\cA` is `\x01`)
  `\0177`   | matches an 8-bit character with octal value `177` (use `\177` in lexer specifications instea, see below)
  `\x7f`    | matches an 8-bit character with hexadecimal value `7f`
  `\x{7f}`  | matches an 8-bit character with hexadecimal value `7f`
  `\p{C}`   | matches a character in category C of \ref reflex-pattern-cat
  `\Q..\E`  | matches the quoted content between `\Q` and `\E` literally
  `[abc]`   | matches one of `a`, `b`, or `c` as \ref reflex-pattern-class
  `[0-9]`   | matches a digit `0` to `9` as \ref reflex-pattern-class
  `[^0-9]`  | matches any character except a digit as \ref reflex-pattern-class
  `φ?`      | matches `φ` zero or one time (optional)
  `φ*`      | matches `φ` zero or more times (repetition)
  `φ+`      | matches `φ` one or more times (repetition)
  `φ{2,5}`  | matches `φ` two to five times (repetition)
  `φ{2,}`   | matches `φ` at least two times (repetition)
  `φ{2}`    | matches `φ` exactly two times (repetition)
  `φ??`     | matches `φ` zero or once as needed (lazy optional)
  `φ*?`     | matches `φ` a minimum number of times as needed (lazy repetition)
  `φ+?`     | matches `φ` a minimum number of times at least once as needed (lazy repetition)
  `φ{2,5}?` | matches `φ` two to five times as needed (lazy repetition)
  `φ{2,}?`  | matches `φ` at least two times or more as needed (lazy repetition)
  `φψ`      | matches `φ` then matches `ψ` (concatenation)
  `φ⎮ψ`     | matches `φ` or matches `ψ` (alternation)
  `(φ)`     | matches `φ` as a group to capture (this is non-capturing in lexer specifications)
  `(?:φ)`   | matches `φ` without group capture
  `(?=φ)`   | matches `φ` without consuming it (\ref reflex-pattern-lookahead)
  `(?<=φ)`  | matches `φ` to the left without consuming it (\ref reflex-pattern-lookbehind, not supported by the RE/flex matcher)
  `(?^φ)`   | matches `φ` and ignore it to continue matching (RE/flex matcher only)
  `^φ`      | matches `φ` at the start of input or start of a line (requires multi-line mode)
  `φ$`      | matches `φ` at the end of input or end of a line (requires multi-line mode)
  `\Aφ`     | matches `φ` at the start of input
  `φ\z`     | matches `φ` at the end of input
  `\bφ`     | matches `φ` starting at a word boundary
  `φ\b`     | matches `φ` ending at a word boundary
  `\Bφ`     | matches `φ` starting at a non-word boundary
  `φ\B`     | matches `φ` ending at a non-word boundary
  `\<φ`     | matches `φ` that starts a word
  `\>φ`     | matches `φ` that starts a non-word
  `φ\<`     | matches `φ` that ends a non-word
  `φ\>`     | matches `φ` that ends a word
  `\i`      | matches an indent for \ref reflex-pattern-dents matching
  `\j`      | matches a dedent for \ref reflex-pattern-dents matching
  `(?i:φ)`  | \ref reflex-pattern-anycase matches `φ` ignoring case
  `(?m:φ)`  | \ref reflex-pattern-multiline `^` and `$` in `φ` match begin and end of a line (default in lexer specifications)
  `(?s:φ)`  | \ref reflex-pattern-dotall `.` (dot) in `φ` matches newline
  `(?u:φ)`  | \ref reflex-pattern-unicode `.`, `\s`, `\w`, `\l`, `\u`, `\S`, `\W`, `\L`, `\U` match Unicode
  `(?x:φ)`  | \ref reflex-pattern-freespace ignore all whitespace and comments in `φ`
  `(?#:..)` | all of `..` is skipped as a comment

@note The lazy quantifier `?` for optional patterns `φ??` and repetitions `φ*?`
`φ+?` is not supported by Boost.Regex in POSIX mode.  In general, POSIX
matchers do not support lazy quantifiers due to POSIX limitations that are
rooted in the theory of formal languages FSM of regular expressions.

The following patterns are available in RE/flex and adopt the same Flex/Lex
patterns syntax.  These pattern should only be used in lexer specifications:

  Pattern            | Matches
  ------------------ | --------------------------------------------------------
  `\177`             | matches an 8-bit character with octal value `177`
  `".."`             | matches the quoted content literally
  `φ/ψ`              | matches `φ` if followed by `ψ` as a \ref reflex-pattern-trailing
  `<S>φ`             | matches `φ` only if state `S` is enabled in \ref reflex-states
  `<S1,S2,S3>φ`      | matches `φ` only if state `S1`, `S2`, or state `S3` is enabled in \ref reflex-states
  `<*>φ`             | matches `φ` in any state of the \ref reflex-states
  `<<EOF>>`          | matches EOF in any state of the \ref reflex-states
  `<S><<EOF>>`       | matches EOF only if state `S` is enabled in \ref reflex-states
  `[a-z]{+}[A-Z]`    | matches a letter, same as `[a-z⎮⎮[A-Z]]` \ref reflex-pattern-class
  `[a-z]{-}[aeiou]`  | matches a consonant, same as `[a-z−−[aeiou]]` \ref reflex-pattern-class
  `[a-z]{&}[^aeiou]` | matches a consonant, same as `[a-z&&[^aeiou]]` \ref reflex-pattern-class
  `[a-z]{⎮}[A-Z]`    | matches a letter, same as `[a-z⎮⎮[A-Z]]` \ref reflex-pattern-class

Note that the characters `.` (dot), `\`, `?`, `*`, `+`, `|`, `(`, `)`, `[`,
`]`, `{`, `}`, `^`, and `$` are meta-characters and should be escaped to match.
Lexer specifications also include the `"` and `/` as meta-characters and these
should be escaped to match.

Spaces and tabs cannot be matched in patterns in lexer specifications.  To
match the space character use `" "` or `[ ]` and to match the tab character use
`\t`.  Use `\h` to match a space or tab.

The order of precedence for composing larger patterns from sub-patterns is as
follows, from high to low precedence:

1. Characters, character classes (bracket expressions), escapes, quotation
2. Grouping `(φ)`, `(?:φ)`, `(?=φ)`, and inline modifiers `(?imsux:φ)`
3. Quantifiers `?`, `*`, `+`, `{n,m}`
4. Concatenation `φψ` (including trailing context `φ/ψ`)
5. Anchoring `^`, `$`, `\<`, `\>`, `\b`, `\B`, `\A`, `\z` 
6. Alternation `φ|ψ`
7. Global modifiers `(?imsux)φ`

@note When using regex patterns in C++ literal strings, make sure that "regex
escapes are escaped", meaning that an extra backslash is needed for every
backslash in the regex string.

@note Trigraphs in C/C++ strings are special tripple-character sequences,
beginning with two question marks and followed by a character that is
translated.  Avoid `??` in regex strings.  Instead, use at least one escaped
question mark, such as `?\?`, which the compiler will translate to `??`.  This
problem does not apply to lexer specifications that the **reflex** command
converts to regex strings.  Fortunately, most C++ compilers ignore trigraphs
unless in standard-conforming modes, such as `-ansi` and `-std=c++98`.

🔝 [Back to table of contents](#)

### Character classes                                   {#reflex-pattern-class}

Character classes in bracket lists represent sets of characters.  Sets can be
inverted, subtracted, intersected, and merged:

  Pattern           | Matches
  ----------------- | ---------------------------------------------------------
  `[a-zA-Z]`        | matches a letter
  `[^a-zA-Z]`       | matches a non-letter (character class inversion)
  `[a-z−−[aeiou]]`  | matches a consonant (character class subtraction)
  `[a-z&&[^aeiou]]` | matches a consonant (character class intersection)
  `[a-z⎮⎮[A-Z]]`    | matches a letter (character class union)

Bracket lists cannot be empty, so `[]` and `[^]` are invalid.  In fact, the
first character after the bracket is always part of the list.  So `[][]` is a
list that matches a `]` and a `[`, `[^][]` is a list that matches anything but
`]` and `[`, and `[-^]` is a list that matches a `-` and a `^`.

Bracket lists may contain ASCII and Unicode \ref reflex-pattern-cat.

To add Unicode character categories and UTF-8 characters to bracket lists
\ref reflex-pattern-unicode should be enabled.

An inverted Unicode character class is constructed by subtracting the character
class from the Unicode range U+0000 to U+D7FF and U+E000 to U+10FFFF.

It is an error to construct an empty character class by subtraction or by
intersection.

🔝 [Back to table of contents](#)

### Character categories                                  {#reflex-pattern-cat}

The 7-bit ASCII character categories are:

  Category     | POSIX form   | Matches
  ------------ | ------------ | -----------------------------------------------
  `\p{Space}`  | `[:space:]`  | matches a white space character `[ \t\n\v\f\r]` same as `\s`
  `\p{Xdigit}` | `[:xdigit:]` | matches a hex digit `[0-9A-Fa-f]`
  `\p{Cntrl}`  | `[:cntrl:]`  | matches a control character `[\x00-\0x1f\x7f]`
  `\p{Print}`  | `[:print:]`  | matches a printable character `[\x20-\x7e]`
  `\p{Alnum}`  | `[:alnum:]`  | matches a alphanumeric character `[0-9A-Za-z]`
  `\p{Alpha}`  | `[:alpha:]`  | matches a letter `[A-Za-z]`
  `\p{Blank}`  | `[:blank:]`  | matches a blank `[ \t]` same as `\h`
  `\p{Digit}`  | `[:digit:]`  | matches a digit `[0-9]` same as `\d`
  `\p{Graph}`  | `[:graph:]`  | matches a visible character `[\x21-\x7e]`
  `\p{Lower}`  | `[:lower:]`  | matches a lower case letter `[a-z]` same as `\l`
  `\p{Punct}`  | `[:punct:]`  | matches a punctuation character `[\x21-\x2f\x3a-\x40\x5b-\x60\x7b-\x7e]`
  `\p{Upper}`  | `[:upper:]`  | matches an upper case letter `[A-Z]` same as `\u`
  `\p{Word}`   | `[:word:]`   | matches a word character `[0-9A-Za-z_]` same as `\w`
  `\d`         | `[:digit:]`  | matches a digit `[0-9]`
  `\D`         | `[:^digit:]` | matches a non-digit `[^0-9]`
  `\h`         | `[:blank:]`  | matches a blank character `[ \t]`
  `\H`         | `[:^blank:]` | matches a non-blank character `[^ \t]`
  `\s`         | `[:space:]`  | matches a white space character `[ \t\n\v\f\r]`
  `\S`         | `[:^space:]` | matches a non-white space `[^ \t\n\v\f\r]`
  `\l`         | `[:lower:]`  | matches a lower case letter `[a-z]`
  `\L`         | `[:^lower:]` | matches a non-lower case letter `[^a-z]`
  `\u`         | `[:upper:]`  | matches an upper case letter `[A-Z]`
  `\U`         | `[:^upper:]` | matches a nonupper case letter `[^A-Z]`
  `\w`         | `[:word:]`   | matches a word character `[0-9A-Za-z_]`
  `\W`         | `[:^word:]`  | matches a non-word character `[^0-9A-Za-z_]`

The POSIX form can only be used in bracket lists, for example
`[[:lower:][:digit:]]` matches an ASCII lower case letter or a digit.  

The following Unicode character categories are enabled with the **reflex**
`−−unicode` option or \ref reflex-pattern-unicode `(?u:φ)` and with the regex
matcher converter flag `reflex::convert_flag::unicode` when using a regex
library:

  Category                               | Matches
  -------------------------------------- | ------------------------------------
  `.`                                    | matches any Unicode character (beware of \ref invalid-utf)
  `\X`                                   | matches any ISO-8859-1 or Unicode character (with or without the `−−unicode` option)
  `\R`                                   | matches a Unicode line break
  `\s`, `\p{Zs}`                         | matches a white space character with Unicode sub-propert Zs
  `\l`, `\p{Ll}`                         | matches a lower case letter with Unicode sub-property Ll
  `\u`, `\p{Lu}`                         | matches an upper case letter with Unicode sub-property Lu
  `\w`, `\p{Word}`                       | matches a Unicode word character with property L, Nd, or Pc
  `\p{Unicode}`                          | matches any Unicode character (U+00 to U+10FFFF minus U+D800 to U+DFFF)
  `\p{ASCII}`                            | matches an ASCII character U+00 to U+007F)
  `\p{Non_ASCII_Unicode}`                | matches a non-ASCII character U+80 to U+10FFFF minus U+D800 to U+DFFF)
  `\p{Letter}`                           | matches a character with Unicode property Letter
  `\p{Mark}`                             | matches a character with Unicode property Mark
  `\p{Separator}`                        | matches a character with Unicode property Separator
  `\p{Symbol}`                           | matches a character with Unicode property Symbol
  `\p{Number}`                           | matches a character with Unicode property Number
  `\p{Punctuation}`                      | matches a character with Unicode property Punctuation
  `\p{Other}`                            | matches a character with Unicode property Other
  `\p{Lowercase_Letter}`, `\p{Ll}`       | matches a character with Unicode sub-property Ll
  `\p{Uppercase_Letter}`, `\p{Lu}`       | matches a character with Unicode sub-property Lu
  `\p{Titlecase_Letter}`, `\p{Lt}`       | matches a character with Unicode sub-property Lt
  `\p{Modifier_Letter}`, `\p{Lm}`        | matches a character with Unicode sub-property Lm
  `\p{Other_Letter}`, `\p{Lo}`           | matches a character with Unicode sub-property Lo
  `\p{Non_Spacing_Mark}`, `\p{Mn}`       | matches a character with Unicode sub-property Mn
  `\p{Spacing_Combining_Mark}`, `\p{Mc}` | matches a character with Unicode sub-property Mc
  `\p{Enclosing_Mark}`, `\p{Me}`         | matches a character with Unicode sub-property Me
  `\p{Space_Separator}`, `\p{Zs}`        | matches a character with Unicode sub-property Zs
  `\p{Line_Separator}`, `\p{Zl}`         | matches a character with Unicode sub-property Zl
  `\p{Paragraph_Separator}`, `\p{Zp}`    | matches a character with Unicode sub-property Zp
  `\p{Math_Symbol}`, `\p{Sm}`            | matches a character with Unicode sub-property Sm
  `\p{Currency_Symbol}`, `\p{Sc}`        | matches a character with Unicode sub-property Sc
  `\p{Modifier_Symbol}`, `\p{Sk}`        | matches a character with Unicode sub-property Sk
  `\p{Other_Symbol}`, `\p{So}`           | matches a character with Unicode sub-property So
  `\p{Decimal_Digit_Number}`, `\p{Nd}`   | matches a character with Unicode sub-property Nd
  `\p{Letter_Number}`, `\p{Nl}`          | matches a character with Unicode sub-property Nl
  `\p{Other_Number}`, `\p{No}`           | matches a character with Unicode sub-property No
  `\p{Dash_Punctuation}`, `\p{Pd}`       | matches a character with Unicode sub-property Pd
  `\p{Open_Punctuation}`, `\p{Ps}`       | matches a character with Unicode sub-property Ps
  `\p{Close_Punctuation}`, `\p{Pe}`      | matches a character with Unicode sub-property Pe
  `\p{Initial_Punctuation}`, `\p{Pi}`    | matches a character with Unicode sub-property Pi
  `\p{Final_Punctuation}`, `\p{Pf}`      | matches a character with Unicode sub-property Pf
  `\p{Connector_Punctuation}`, `\p{Pc}`  | matches a character with Unicode sub-property Pc
  `\p{Other_Punctuation}`, `\p{Po}`      | matches a character with Unicode sub-property Po
  `\p{Control}`, `\p{Cc}`                | matches a character with Unicode sub-property Cc
  `\p{Format}`, `\p{Cf}`                 | matches a character with Unicode sub-property Cf
  `\p{UnicodeIdentifierStart}`           | matches a character in the Unicode IdentifierStart class
  `\p{UnicodeIdentifierPart}`            | matches a character in the Unicode IdentifierPart class
  `\p{IdentifierIgnorable}`              | matches a character in the IdentifierIgnorable class
  `\p{JavaIdentifierStart}`              | matches a character in the Java IdentifierStart class
  `\p{JavaIdentifierPart}`               | matches a character in the Java IdentifierPart class
  `\p{CsIdentifierStart}`                | matches a character in the C# IdentifierStart class
  `\p{CsIdentifierPart}`                 | matches a character in the C# IdentifierPart class
  `\p{PythonIdentifierStart}`            | matches a character in the Python IdentifierStart class
  `\p{PythonIdentifierPart}`             | matches a character in the Python IdentifierPart class

In addition, the `−−unicode` option enables Unicode language scripts:

  `\p{Arabic}`, `\p{Armenian}`, `\p{Avestan}`, `\p{Balinese}`, `\p{Bamum}`,
  `\p{Bassa_Vah}`, `\p{Batak}`, `\p{Bengali}`, `\p{Bopomofo}`, `\p{Brahmi}`,
  `\p{Braille}`, `\p{Buginese}`, `\p{Buhid}`, `\p{Canadian_Aboriginal}`,
  `\p{Carian}`, `\p{Caucasian_Albanian}`, `\p{Chakma}`, `\p{Cham}`,
  `\p{Cherokee}`, `\p{Common}`, `\p{Coptic}`, `\p{Cuneiform}`, `\p{Cypriot}`,
  `\p{Cyrillic}`, `\p{Deseret}`, `\p{Devanagari}`, `\p{Duployan}`,
  `\p{Egyptian_Hieroglyphs}`, `\p{Elbasan}`, `\p{Ethiopic}`, `\p{Georgian}`,
  `\p{Glagolitic}`, `\p{Gothic}`, `\p{Grantha}`, `\p{Greek}`, `\p{Gujarati}`,
  `\p{Gurmukhi}`, `\p{Han}`, `\p{Hangul}`, `\p{Hanunoo}`, `\p{Hebrew}`,
  `\p{Hiragana}`, `\p{Imperial_Aramaic}`, `\p{Inherited}`,
  `\p{Inscriptional_Pahlavi}`, `\p{Inscriptional_Parthian}`, `\p{Javanese}`,
  `\p{Kaithi}`, `\p{Kannada}`, `\p{Katakana}`, `\p{Kayah_Li}`,
  `\p{Kharoshthi}`, `\p{Khmer}`, `\p{Khojki}`, `\p{Khudawadi}`, `\p{Lao}`,
  `\p{Latin}`, `\p{Lepcha}`, `\p{Limbu}`, `\p{Linear_A}`, `\p{Linear_B}`,
  `\p{Lisu}`, `\p{Lycian}`, `\p{Lydian}`, `\p{Mahajani}`, `\p{Malayalam}`,
  `\p{Mandaic}`, `\p{Manichaean}`, `\p{Meetei_Mayek}`, `\p{Mende_Kikakui}`,
  `\p{Meroitic_Cursive}`, `\p{Meroitic_Hieroglyphs}`, `\p{Miao}`, `\p{Modi}`,
  `\p{Mongolian}`, `\p{Mro}`, `\p{Myanmar}`, `\p{Nabataean}`,
  `\p{New_Tai_Lue}`, `\p{Nko}`, `\p{Ogham}`, `\p{Ol_Chiki}`, `\p{Old_Italic}`,
  `\p{Old_North_Arabian}`, `\p{Old_Permic}`, `\p{Old_Persian}`,
  `\p{Old_South_Arabian}`, `\p{Old_Turkic}`, `\p{Oriya}`, `\p{Osmanya}`,
  `\p{Pahawh_Hmong}`, `\p{Palmyrene}`, `\p{Pau_Cin_Hau}`, `\p{Phags_Pa}`,
  `\p{Phoenician}`, `\p{Psalter_Pahlavi}`, `\p{Rejang}`, `\p{Runic}`,
  `\p{Samaritan}`, `\p{Saurashtra}`, `\p{Sharada}`, `\p{Shavian}`,
  `\p{Siddham}`, `\p{Sinhala}`, `\p{Sora_Sompeng}`, `\p{Sundanese}`,
  `\p{Syloti_Nagri}`, `\p{Syriac}`, `\p{Tagalog}`, `\p{Tagbanwa}`,
  `\p{Tai_Le}`, `\p{Tai_Tham}`, `\p{Tai_Viet}`, `\p{Takri}`, `\p{Tamil}`,
  `\p{Telugu}`, `\p{Thaana}`, `\p{Thai}`, `\p{Tibetan}`, `\p{Tifinagh}`,
  `\p{Tirhuta}`, `\p{Ugaritic}`, `\p{Vai}`, `\p{Warang_Citi}`, `\p{Yi}`.

You can also use the capitalized `\P{C}` form that has the same meaning as
`\p{^C}`, which matches any character except characters in the class `C`.

🔝 [Back to table of contents](#)

### Anchors and boundaries                             {#reflex-pattern-anchor}

Anchors are used to demarcate the start and end of input or the start and end
of a line:

  Pattern   | Matches
  --------- | -----------------------------------------------------------------
  `^φ`      | matches `φ` at the start of input or start of a line (multi-line mode)
  `φ$`      | matches `φ` at the end of input or end of a line (multi-line mode)
  `\Aφ`     | matches `φ` at the start of input
  `φ\z`     | matches `φ` at the end of input

Anchors in lexer specifications require context, meaning that `φ` cannot be
empty.  Note that `<<EOF>>` in lexer specifications match the end of input
without requiring any context.  Actions for the start of input can be specified
in the code block preceding the rules.

Word boundaries demarcate words.  Word characters `\w` are letters, digits, and
the underscore.

  Pattern   | Matches
  --------- | -----------------------------------------------------------------
  `\bφ`     | matches `φ` starting at a word boundary
  `φ\b`     | matches `φ` ending at a word boundary
  `\Bφ`     | matches `φ` starting at a non-word boundary
  `φ\B`     | matches `φ` ending at a non-word boundary
  `\<φ`     | matches `φ` that starts as a word
  `\>φ`     | matches `φ` that starts as a non-word
  `φ\<`     | matches `φ` that ends as a non-word
  `φ\>`     | matches `φ` that ends as a word

@note The RE/flex regex library requires word boundaries to be specified in
patterns at the start or end of the pattern.  Boundaries are not permitted in
the middle of a pattern, see \ref reflex-limitations.

🔝 [Back to table of contents](#)

### Indent/nodent/dedent                                {#reflex-pattern-dents}

Automatic indent and dedent matching is a special feature of RE/flex and is
only available when the RE/flex matcher engine is used (the default matcher).
An indent and a dedent position is defined and matched with:

  Pattern | Matches
  ------- | -------------------------------------------------------------------
  `\i`    | indent: matches and adds a new indent stop position
  `\j`    | dedent: matches a previous indent position, removes one indent stop

These patterns should be used in combination with the start of a line anchor
`^` followed by a pattern that represents left margin spacing for indentations.
This spacing pattern may include any characters that are considered part of the
left margin, but must exclude `\n`.  For example:

<div class="alt">
```cpp
%o tabs=8
%%
^\h+      out() << "| "; // nodent: text is aligned to current indent
^\h*\i    out() << "> "; // indent: matched and added with \i
^\h*\j    out() << "< "; // dedent: matched with \j
\j        out() << "< "; // dedent: for each extra level dedented
%%
```
</div>

The `\h` pattern matches space and tabs, where tabs advance to the next column
that is a multiple of 8.  The tab multiplier can be changed by setting the
`−−tabs=N` option where `N` must be a positive integer that is a power of 2
and between 1 and 8 (1, 2, 4, 8).

To add a pattern that consumes line continuations without affecting the
indentation levels defined by `\i`, use a negative match, which is a new
RE/flex feature:

<div class="alt">
```cpp
(?^\\\n\h*)  // lines ending in \ will continue on the next line
```
</div>

The negative pattern `(?^\\\n\h+)` consumes input internally as if we are
repeately calling `input()` (or `yyinput()` with `−−flex`).  We used it here to
consume the line-ending `\` and the indent that followed it, as if this text
was not part of the input, which ensures that the current indent positions that
are defined by `\i` are not affected.

To scan input that continues on the next new line(s) while preserving the
current indent stop positions, use the RE/flex matcher `matcher().push_stops()`
and `matcher().pop_stops()`:

  RE/flex action            | Result
  ------------------------- | -------------------------------------------------
  `matcher().push_stops()`  | push indent stops on the stack then clear stops
  `matcher().pop_stops()`   | pop indent stops and make them current
  `matcher().clear_stops()` | clear current indent stops
  `matcher().stops()`       | reference to current `std::vector<size_t>` stops

For example, to continue scanning after a `/*` for multiple lines without
indentation matching and up to a `*/` you can save the current indent positions
and transition to a new start condition state to scan the content between `/*`
and `*/`:

<div class="alt">
```cpp
%o tabs=8
%x CONTINUE
%%
^\h+          out() << "| "; // nodent, text is aligned to current margin
^\h*\i        out() << "> "; // indent
^\h*\j        out() << "< "; // dedent
\j            out() << "< "; // dedent, for each extra indent level on the same line
"/*"          matcher().push_stops(); // save the indent margin/tab stops
              start(CONTINUE);        // continue w/o indent matching
<CONTINUE>{
"*/"          matcher().pop_stops();  // restore the indent margin/tab stops
              start(INITIAL);         // go back to the initial scanning state
.|\n          /* ignore */
}
```
</div>

The `matcher().stops()` method returns a reference to the current
`std::vector<size_t>` of indent stop positions, which may be modified by adding
and/or removing indent stop positions.  Make sure to keep the vector sorted.

See \ref reflex-states for more information about start condition states.

🔝 [Back to table of contents](#)

### Lookahead                                       {#reflex-pattern-lookahead}

A lookahead pattern `φ(?=ψ)` matches `φ` only when followed by pattern `ψ`.
The text matched by `ψ` is not consumed.

Boost matchers support lookahead `φ(?=ψ)` and lookbehind `φ(?<=ψ)` patterns
that may appear anywhere in a regex.  The RE/flex matcher supports lookahead at
the end of a pattern, similar to \ref reflex-pattern-trailing.

🔝 [Back to table of contents](#)

### Lookbehind                                     {#reflex-pattern-lookbehind}

A lookbehind pattern `φ(?<=ψ)` matches `φ` only when it also matches pattern
`ψ` at its end (that is, `.*(?<=ab)` matches anything that ends in `ab`).

The RE/flex matcher does not support lookbehind.  Lookbehind patterns should
not look too far behind, see \ref reflex-limitations.

🔝 [Back to table of contents](#)

### Trailing context                                 {#reflex-pattern-trailing}

Flex "trailing context" `φ/ψ` matches a pattern `φ` only when followed by the
lookahead pattern `φ`.  A trailing context `φ/ψ` has the same meaning as the
lookahead `φ(?=ψ)`, see \ref reflex-pattern-lookahead.

A trailing context can only be used in lexer specifications and should only
occur at the end of a pattern, not in the middle of a pattern.  There are some
important \ref reflex-limitations to consider that are historical and related
to the contruction of efficient FSMs for regular expressions.  The limitations
apply to trailing context and lookaheads that the RE/flex matcher implements.

🔝 [Back to table of contents](#)

### Unicode mode                                      {#reflex-pattern-unicode}

Use **reflex** option `−−unicode` (or `%%option unicode`) to globally enable
Unicode.  Use `(?u:φ)` to locally enable Unicode in a pattern `φ`.  Unicode
mode enables the following patterns to be used:

  Pattern            | Matches
  ------------------ | --------------------------------------------------------
  `.`                | matches any Unicode character (beware of \ref invalid-utf)
  `€` (UTF-8)        | matches wide character `€`, encoded in UTF-8
  `[€¥£]` (UTF-8)    | matches wide character `€`, `¥` or `£`, encoded in UTF-8
  `\X`               | matches any ISO-8859-1 or Unicode character
  `\R`               | matches a Unicode line break `\r\n` or `[\u{000A}-\u{000D}u{U+0085}\u{2028}\u{2029}]`
  `\s`               | matches a white space character with Unicode sub-property Zs
  `\l`               | matches a lower case letter with Unicode sub-property Ll
  `\u`               | matches an upper case letter with Unicode sub-property Lu
  `\w`               | matches a Unicode word character with property L, Nd, or Pc
  `\u{20AC}`         | matches Unicode character U+20AC
  `\p{C}`            | matches a character in category C
  `\p{^C}`,`\P{C}`   | matches any character except in category C

When converting regex patterns for use with a C++ regex library, use regex
matcher converter flag `reflex::convert_flag::unicode` to convert Unicode
patterns for use with the 8-bit based RE/flex, Boost.Regex, and std::regex
regex libraries, see \ref regex-convert for more details.

🔝 [Back to table of contents](#)

### Free space mode                                 {#reflex-pattern-freespace}

Free space mode can be useful to improve readability of patterns.  Free space
mode permits spacing between concatenations and alternations in patterns.  As
always, to match a single space use `" "` or `[ ]`.  Long patterns can continue
on the next line(s) when lines end with an escape `\`.

Free space mode requires lexer actions in \ref reflex-spec-rules of a lexer
specification to be placed in `{` and `}` blocks and other code in `%{` and
`%}` instead of indented.

To enable free space mode in **reflex** use the `−−freespace` option (or
`%%option freespace`).

When converting regex patterns for use with a C++ regex library, use regex
matcher converter flag `reflex::convert_flag::freespace` to convert the regex
or use `(?x:φ)` to locally enable free-space mode in a pattern `φ`.  See
\ref regex-convert for more details.

🔝 [Back to table of contents](#)

### Multi-line mode                                 {#reflex-pattern-multiline}

Multi-line mode makes the anchors `^` and `$` match the start and end of a
line, respectively.  Multi-line mode is the default mode in lexer
specifications.

When converting regex patterns for use with a C++ regex library, use regex
matcher converter flag `reflex::convert_flag::multiline` to convert the regex
or use `(?m:φ)` to locally enable multi-line mode in a pattern `φ`.  See
\ref regex-convert for more details.

🔝 [Back to table of contents](#)

### Dotall mode                                        {#reflex-pattern-dotall}

To enable dotall mode in **reflex** use the `-a` or `−−dotall` option (or
`%%option dotall`).

When converting regex patterns for use with a C++ regex library, use regex
matcher converter flag `reflex::convert_flag::dotall` to convert the regex
or use `(?s:φ)` to locally enable dotall mode in a pattern `φ`.  See
\ref regex-convert for more details.

🔝 [Back to table of contents](#)

### Case-insensitive mode                             {#reflex-pattern-anycase}

To enable case-insensitive mode in **reflex** use the `-i` or
`−−case-insensitive` option (or `%%option case-insensitive`).

When converting regex patterns for use with a C++ regex library, use regex
matcher converter flag `reflex::convert_flag::anycase` to convert the regex
or use `(?i:φ)` to locally enable case-insensitive mode in a pattern `φ`.  See
\ref regex-convert for more details.

🔝 [Back to table of contents](#)


The Lexer/yyFlexLexer class                                     {#reflex-lexer}
---------------------------

By default, **reflex** produces a Lexer class with a virtual lex scanner
function.  The name of this function as well as the Lexer class name and the
namespace can be set with options:

  Option      | RE/flex default name | Flex default name
  ----------- | -------------------- | ----------------------------------------
  `namespace` | *n/a*                | *n/a* 
  `lexer`     | `Lexer` class        | `yyFlexLexer` class
  `lex`       | `lex()` function     | `yylex()` function

To customize the Lexer class use these options and code injection.

To understand the impact of these options, consider the following lex
specification template:

<div class="alt">
```cpp
%option namespace=NAMESPACE
%option lexer=LEXER
%option lex=LEX

%class{
  MEMBERS
%}

%init{
  INIT
%}

%%

%{
  CODE
%}

REGEX ACTION

%%
```
</div>

This produces the following Lexer class with the template parts filled in:

```cpp
#include <reflex/abslexer.h>
namespace NAMESPACE {
  class LEXER : public reflex::AbstractLexer<reflex::Matcher> {
    MEMBERS
   public:
    LEXER(
        const reflex::Input& input = reflex::Input(),
        std::ostream&        os    = std::cout)
      :
        AbstractLexer(input, os)
    {
      INIT
    }
    static const int INITIAL = 0;
    virtual int LEX(void);
    int LEX(
        const reflex::Input& input,
        std::ostream        *os = NULL)
    {
      in(input);
      if (os)
        out(*os);
      return LEX();
    }
  };
  int NAMESPACE::LEXER::LEX()
  {
    static const reflex::Pattern PATTERN_INITIAL("(?m)(REGEX)");
    if (!has_matcher())
    {
      matcher(new Matcher(PATTERN_INITIAL, stdinit(), this));
    }
    CODE
    while (true)
    {
      switch (matcher().scan())
      {
        case 0:
          if (matcher().at_end())
          {
            return 0;
          }
          else
          {
            out().put(matcher().input());
          }
          break;
        case 1:
          ACTION
          break;
      }
    }
  }
}
```

The Lexer class produced with option `−−flex` is compatible with Flex (assuming
Flex with option `-+` for C++):

```cpp
#include <reflex/flexlexer.h>
namespace NAMESPACE {
  typedef reflex::FlexLexer<reflex::Matcher> FlexLexer;
  class LEXER : public FlexLexer {
    MEMBERS
   public:
    LEXER(
        const reflex::Input& input = reflex::Input(),
        std::ostream        *os    = NULL)
      :
        FlexLexer(input, os)
    {
      INIT
    }
    virtual int LEX(void);
    int LEX(
        const reflex::Input& input,
        std::ostream        *os = NULL)
    {
      in(input);
      if (os)
        out(*os);
      return LEX();
    }
  };
  int NAMESPACE::LEXER::LEX()
  {
    static const reflex::Pattern PATTERN_INITIAL("(?m)(REGEX)");
    if (!has_matcher())
    {
      matcher(new Matcher(PATTERN_INITIAL, stdinit(), this));
      YY_USER_INIT
    }
    CODE
    while (true)
    {
      switch (matcher().scan())
      {
        case 0:
          if (matcher().at_end())
          {
            return 0;
          }
          else
          {
            output(matcher().input());
          }
          YY_BREAK
        case 1:
          YY_USER_ACTION
          ACTION
          YY_BREAK
      }
    }
  }
}
```

To use a custom lexer class that inherits the generated base Lexer class, use
option `−−class=NAME` to declare the name of your custom lexer class (or option
`−−yyclass=NAME` to also enable `−−flex` compatibility with the `yyFlexLexer`
class).  For details, see \ref reflex-inherit.

🔝 [Back to table of contents](#)


Inheriting Lexer/yyFlexLexer                                  {#reflex-inherit}
----------------------------

To define a custom lexer class that inherits the generated Lexer or the
yyFlexLexer class, use option `−−class=NAME` or option `−−yyclass=NAME`,
respectively.  Note that `−−yyclass=NAME` also enables option `−−flex` and
therefore enables Flex specification syntax.

When a `−−class=NAME` or `−−yyclass=NAME` option is specified with the name of
your custom lexer class, **reflex** generates the `lex()` (or `yylex()`) method
code for your custom lexer class.  The custom lexer class should declare a
public `int lex()` method (or `int yylex()` method with option
`−−yyclass=NAME`).  Otherwise, C++ compilation of your custom class will fail.

For example, the following bare-bones custom Lexer class definition simply
inherits Lexer and declares a public `int lex()` method:
  
<div class="alt">
```cpp
%option class=MyLexer
%{
  class MyLexer : public Lexer {
   public:
    int lex();
  };
%}
```
</div>

The `int MyLexer::lex()` method code is generated by **reflex** for this lexer
specification.

Options `−−lexer=NAME` and `--lex=NAME` can be combined with `−−class=NAME` to
change the name of the inherited Lexer class and change the name of the `lex()`
method, respectively.

When using option `−−yyclass=NAME` the inherited lexer is `yyFlexLexer`.  The
custom lexer class should declare a public `yylex()` method similar to Flex.
For example:

<div class="alt">
```cpp
%option yyclass=MyLexer
%{
  class MyLexer : public yyFlexLexer {
   public:
    int yylex();
  };
%}
```
</div>

The `int MyLexer::yylex()` method code is generated by **reflex** for this
lexer specification.

🔝 [Back to table of contents](#)


Combining multiple lexers                                    {#reflex-multiple}
-------------------------

To combine multiple lexers in one application, use the `−−lexer=NAME` option.
This option renames the generated lexer class to avoid lexer class name
clashes.  Use this option in combination with option `−−header-file` to output
a header file with the lexer class declaration to include in your application
source code.

Use option `−−prefix=NAME` to output the generated code in file `lex.NAME.cpp`
instead of the standard `lex.yy.cpp` to avoid file name clashes.  This option
also affect the `−−flex` option by generating `xxFlexLexer` with a `xxlex()`
method when option `−−prefix=xx` is used.  The generated `#define` names (some
of which are added to support option `−−flex`) are prefixed to avoid macro name
clashes.

Alternatively to `−−lexer=NAME` you can use `−−namespace=NAME` to place the
generated lexer class in a C++ namespace to avoid lexer class name clashes.

🔝 [Back to table of contents](#)


Switching input sources                                         {#reflex-input}
-----------------------

To create a Lexer class instance that reads from a designated input source
instead of standard input, pass the input source as the first argument to its
constructor and use the second argument to optionally set an `std::ostream`
that is assigned to `out()` and is used by `echo()` (likewise, assigned to
`*yyout` and used by `ECHO` when option `−−flex` is used):

```cpp
Lexer lexer(input, std::cout);
```

likewise, with option `−−flex`:

```cpp
yyFlexLexer lexer(input, std::cout);
```

where `input` is a `reflex::Input` object.  The `reflex::Input` constructor
takes a `FILE*` descriptor, `std::istream`, a string `std::string` or
`const char*`, or a wide string `std::wstring` or `const wchar_t*`.

To switch input to another source while using the scanner, use `in(i)` with
`reflex::Input i`:

```cpp
// read from a file, this also decodes UTF-16/32 encodings automatically
FILE *fd = fopen("cow.txt", "r");
if (fd)
  lexer.in(fd);

// read from a stream (as is, can be ASCII or UTF-8)
std::istream i = std::ifstream("file", std::ifstream::in);
lexer.in(i);

// read from a string (as is, can be ASCII or UTF-8)
lexer.in("How now brown cow.");

// read from a wide string, encoding it to UTF-8 for matching
lexer.in(L"How now brown cow.");
```

Invoking `in(i)` also resets the lexer's matcher (internally with
`matcher.reset()`).  This clears the line and column counters, resets the
internal anchor and boundary flags for anchor and word boundary matching, and
resets the matcher to consume buffered input.

You can also assign new input with `in() = &i`, which does not reset the
lexer's matcher.  This means that when the end of the input (EOF) is reached,
and you want to switch to new input, then you should clear the EOF state first
with `lexer.matcher().set_end(false)` to reset EOF.  Or use
`lexer.matcher().reset()` to clear the state.

Switching input before the end of the input source is reached discards all
remaining input from that source.  To switch input without affecting the
current input source, switch matchers instead.  The matchers buffer the input
and manage the input state, in addition to pattern matching the input.

To switch to a matcher that scans from a new input source, use:

```cpp
... // scanning etc.
Matcher *oldmatcher = matcher();
Matcher *newmatcher = new_matcher(input);
matcher(newmatcher);
... // scan the new input
del_matcher(newmatcher);
matcher(oldmatcher);
... // continue scanning the old input
```

the same with the `−−flex` option becomes:

```cpp
... // scanning etc.
YY_BUFFER_STATE oldbuf = YY_CURRENT_BUFFER;
YY_BUFFER_STATE newbuf = yy_create_buffer(input, YY_BUF_SIZE);
yy_switch_to_buffer(newbuf);
... // scan the new input
yy_delete_buffer(newbuf);
yy_switch_to_buffer(oldbuf);
... // continue scanning the old input
```

This switches the scanner's input by switching to another matcher.  Note that
`matcher(m)` may be used by the virtual `wrap()` method (or `yywrap()` when
option `−−flex` is used) if you use input wrapping after EOF to set things up
for continued scanning.

Switching input sources (via either `matcher(m)` or `in(i)`) does not change
the current start condition state.

When the scanner reaches the end of the input, it will check the `int wrap()`
method to detetermine if scanning should continue.  If `wrap()` returns one (1)
the scanner terminates and returns zero to its caller.  If `wrap()` returns
zero (0) then the scanner continues.  In this case `wrap()` should set up a new
input source to scan.

For example, continuing reading from `std:in` after some other input source
reached EOF:

<div class="alt">
```cpp
%class{
  virtual int wrap() // note: yywrap() when option −−flex is used
  {
    in(std::in);
    return in().good() ? 0 : 1;
  }
%}
```
</div>

To implement a `wrap()` (and `yywrap()` when option `−−flex` is used) in a
derived lexer class with option `class=NAME` (or `yyclass=NAME`), override the
`wrap()` (or `yywrap()`) method as follows:

<div class="alt">
```cpp
%option class=Tokenizer

%{
  class Tokenizer : Lexer { // note: yyFlexLexer when option −−flex is used
   public:
    virtual int wrap() // note: yywrap() when option −−flex is used
    {
      in(std::in);
      return in().good() ? 0 : 1;
    }
  };
%}
```
</div>

You can override the `wrap()` method to set up a new input source when the
current input is exhausted.  Do not use `matcher().input(i)` to set a new input
source `i`, because that resets the internal matcher state.

With the `−−flex` options your can override the `yyFlexLexer::yywrap()` method
that returns an integer 0 (more input available) or 1 (we're done).

With the `−−flex` and `−−bison` options you should define a global `yywrap()`
function that returns an integer 0 (more input available) or 1 (we're done).  

A more typical scenario is to process an `include` directive in the source
input that should include the source of another file before continuing with the
current input.

For example, the following specification defines a lexer that processes
`#include` directives by switching matchers and using the stack of matchers to
permit nested `#include` directives up to a depth of 99 files:

<div class="alt">
```cpp
%top{
  #include <stdio.h>
%}

%class{

  int depth;

  void include_file()
  {
    depth++;
    if (depth > 99)
      exit(EXIT_FAILURE);           // max include depth exceeded
    char *q = strchr(text(), '"');  // get ..."filename"
    char *file = strdup(q + 1);     // get filename"
    file[strlen(file) - 1] = '\0';  // get filename
    FILE *fd = fopen(file, "r");
    free(file);
    if (!fd)
      exit(EXIT_FAILURE);           // cannot open file
    push_matcher(new_matcher(fd));  // push current matcher, use new matcher
  }

  bool end_of_file()
  {
    if (depth == 0)
      return true;                  // return true: no more input to read
    fclose(in());                   // close current input in() (a FILE*)
    pop_matcher();                  // delete current matcher, pop matcher
    depth--;
    return false;                   // return false: continue reading
  }

%}

%init{
  depth = 0;
%}

%%

^\h*#include\h*\".*?\"    include_file();
.|\n                      echo();
<<EOF>>                   if (end_of_file()) return 0;

%%
```
</div>

With option `−−flex`, the statement `push_matcher(new_matcher(fd))` above
becomes `yypush_buffer_state(yy_create_buffer(fd, YY_BUF_SIZE))` and
`pop_matcher()` becomes `yypop_buffer_state()`.

To set the current input as interactive, such as input from a console, use
`matcher().interactive()` (`yy_set_interactive(1)` with option `−−flex`).  This
disables buffering of the input and makes the scanner responsive.

To read from the input without pattern matching, use `matcher().input()` to
read one character at a time (8-bit, ASCII or UTF-8).  This function returns
EOF if the end of the input was reached.  But be careful, the Flex `yyinput()`
and `input()` functions return 0 instead of an `EOF` (-1)!

To put back one character unto the input stream, use `matcher().unput(c)`

For example, to crudily scan a C/C++ multiline comment we can use the rule:

<div class="alt">
```cpp
"/*"    {  /* skip multiline comments */
  int c;
  while ((c = yyinput()) != 0)
  {
    if (c == '\n')
      ++mylineno;
    else if (c == '*')
    {
      if ((c = yyinput()) == '/')
        break;
      unput(c);
    }
  }
}
```
</div>

Instead of the ugly solution above, a better alternative is to use a regex
`/\*.*?\*/` or perhaps use start condition states, see \ref reflex-states.

To grab the rest of the input as a string, use `matcher().rest()` which returns
a `const char*` string that points to an internal buffer.  Copy the string
before using the matcher again.

To read a number of bytes `n` into a string buffer `s[0..n-1]`, use
`matcher().in.get(s, n)`, which is the same as invoking the virtual method
`matcher().get(s, n)`.  This matcher method can be overriden by a derived
matcher class (to customize reading).

The Flex `YY_INPUT` macro is not supported by RE/flex.  It is recommended to
use `YY_BUFFER_STATE` (Flex), which is a `reflex::FlexLexer::Matcher` class in
RE/flex that holds the matcher state and the state of the current input,
including the line and column number positions (so unlike Flex, `yylineno` does
not have to be saved and restored when switching buffers).  See also section
\ref reflex-spec on the actions to use.

To implement a custom input handler you can use a proper object-oriented
approach: create a derived class of `reflex::Matcher` (or
`reflex::BoostPosixMatcher`) and in the derived class override the
`size_t reflex::Matcher::get(char *s, size_t n)` method for input handling.
This function is called with a string buffer `s` of size `n` bytes.  Fill the
string buffer `s` up to `n` bytes and return the number of bytes stored in `s`.
Return zero upon EOF.  Use **reflex** options `−−matcher=NAME` and
`−−pattern=reflex::Pattern` to use your new matcher class `NAME` (or leave out
`−−pattern` for Boost.Regex derived matchers).

The `FlexLexer` lexer class that is the base class of the `yyFlexLexer` lexer
class generated with **reflex** option `−−flex` defines a virtual `size_t
LexerInput(char*, size_t)` method.  This approach is compatible with Flex.  The
virtual method can be redefined in the generated `yyFlexLexer` lexer to consume
input from some source of text:

<div class="alt">
```cpp
%class{
  virtual size_t LexerInput(char *s, size_t n)
  {
    size_t k;
    // populate s[0..k-1] for some k with k <= n
    return k; // return number of bytes filled in s[]
  }
%}
```
</div>

The `LexerInput` method may be invoked multiple times by the matcher engine
and should eventually return zero to indicate the end of input is reached (e.g.
when at EOF).

🔝 [Back to table of contents](#)


Start condition states                                         {#reflex-states}
----------------------

Start conditions are used to group rules and selectively activate rules when
the start condition state becomes active.

A rule with a pattern that is prefixed with one ore more start conditions will
only be active when the scanner is in one of these start condition states.

For example:

<div class="alt">
```cpp
<A,B>pattern1    action1
<A>pattern2      action2
<B>pattern3      action3
```
</div>

When the scanner is in state `A` rules 1 and 2 are active. When the scanner
is in state `B` rules 1 and 3 are active.

Start conditions are declared in \ref reflex-spec-defs (the first section) of
the lexer specification using `%%state` or `%%xstate` (or `%%s` and `%%x` for
short) followed by a list of names called *start symbols*.  Start conditions
declared with `%%s` are *inclusive start conditions*.  Start conditions
declared with `%%x` are *exclusive start conditions*.

If a start condition is inclusive, then all rules without a start condition and
rules with the corresponding start condition will be active.

If a start condition is exclusive, only the rules with the corresponding start
condition will be active.

When declaring start symbol names it is recommended to use all upper case to
avoid name clashes with other Lexer class members.  For example, we cannot use
`text` as a start symbol name because `text()` is a Lexer method.  When option
`−−flex` is used, start symbol names are simple macros for compatibility.

The scanner is initially in the `INITIAL` start condition state.  The `INITIAL`
start condtion is inclusive: all rules without a start condition and those
prefixed with the `INITIAL` start condition are active when the scanner is in
the `INITIAL` start condition state.

The special start condition prefix `<*>` matches every start condition.
The prefix `<*>` is not needed for `<<EOF>>` rules, because unprefixed
`<<EOF>>` rules are always active, as a special case (`<<EOF>>` and this
special exception were introduced by Flex).

For example:

<div class="alt">
```cpp
%s A
%x B

%%

<A,B>pattern1    action1    // rule for states A and B
<A>pattern2      action2    // rule for state A
<B>pattern3      action3    // rule for state B
<*>pattern4      action4    // rule for states INITIAL, A and B
pattern5         action5    // rule for states INITIAL and A
<<EOF>>          action6    // rule for states INITIAL, A and B

%%
```
</div>

When the scanner is in state `INITIAL` rules 4, 5, and 6 are active.  When the
scanner is in state `A` rules 1, 2, 4, 5, and 6 are active.  When the scanner is
in state `B` rules 1, 3, 4, and 6 are active.  Note that `A` is inclusive
whereas `B` is exclusive.

To switch to a start condition state, use `start(START)` (or `BEGIN START` when
option `−−flex` is used).  To get the current state use `start()` (or
`YY_START` when option `−−flex` is used).  Switching start condition states in
your scanner allows you to create "mini-scanners" to scan portions of the input
that are syntactically different from the rest of the input, such as comments:

<div class="alt">
```cpp
%x COMMENT

%%

"/*"    start(COMMENT);    // with −−flex use: BEGIN COMMENT;
.|\n    echo();            // with −−flex use: ECHO;

<COMMENT>[^*]*         // eat anything that is not a '*'
<COMMENT>"*"+[^*/]*    // eat '*'s not followed by a '/'
<COMMENT>"*"+"/"       start(INITIAL);
<COMMENT><<EOF>>       std::cerr << "EOF in comment\n"; return 1;

%%
```
</div>

Start symbols are actually integer values, where `INITIAL` is 0.  This means
that you can store a start symbol value in a variable.  You can also push the
current start condition on a stack and transition to start condition `START`
with `push_state(START)`.  To transition to a start condition that is on the
top of the stack and pop it use `pop_state()`.  The `top_state()` returns the
start condition that is on the top of the stack:

<div class="alt">
```cpp
%x COMMENT

%%

"/*"    push_state(COMMENT);
.|\n    echo();

<COMMENT>[^*]*         // eat anything that is not a '*'
<COMMENT>"*"+[^*/]*    // eat '*'s not followed by a '/'
<COMMENT>"*"+"/"       pop_state();
<COMMENT><<EOF>>       std::cerr << "EOF in comment\n"; return 1;

%%
```
</div>

When many rules are prefixed by the same start conditions, you can simplify
the rules by placing them in a *start condition scope*:

<div class="alt">
```cpp
<COMMENT>{
[^*]*         // eat anything that is not a '*'
"*"+[^*/]*    // eat '*'s not followed by a '/'
"*"+"/"       start(INITIAL);
<<EOF>>       std::cerr << "EOF in comment\n"; return 1;
}
```
</div>

Contrary to some Flex manuals, rules cannot be indented in a start condition
scope.  Indentation is used to specify code blocks.  Code blocks can be
indented or placed within `%{` and `%}`.  These code blocks are executed when
the scanner is invoked.  A code block at the start of a condition scope is
executed when the scanner is invoked and if it is in the corresponding start
condition state:

<div class="alt">
```cpp
%s A
%x B

%%

%{
  // Code for all inclusive states (INITIAL and A)
%}
pattern    action
...
pattern    action

<B>{
%{
  // Code for state B
%}
pattern    action
...
pattern    action
}

<*>{
%{
  // Code for all inclusive and exclusive states (INITIAL, A and B)
%}
pattern    action
...
pattern    action
}

%%
```
</div>

Start condition scopes may be nested.

🔝 [Back to table of contents](#)


Interfacing with Bison/Yacc                                     {#reflex-bison}
---------------------------

The [Bison](dinosaur.compilertools.net/#bison) tools generate parsers that
invoke the global C function `yylex()` to get the next token.  Tokens are
integer values returned by `yylex()`.

To support Bison parsers use **reflex** option `−−bison`.  This option
generates a scanner with a global lexer object `YY_SCANNER` and a global
`YY_EXTERN_C int yylex()` function.  When the Bison parser is compiled in C and
the scanner is compiled in C++, you must set `YY_EXTERN_C` in the lex
specification to `extern "C"` to enable C linkage rules:

<div class="alt">
```cpp
%top{
  #include "y.tab.h"               /* include y.tab.h generated by bison */
  #define YY_EXTERN_C extern "C"   /* yylex() must use C linkage rules */
%}

%option noyywrap bison

%%

[0-9]+               yylval.num = strtol(text(), NULL, 10);
                     return CONST_NUMBER;
\"([^\\"]|\\")*\"    yylval.str = text();
                     return CONST_STRING;

%%
```
</div>

Note that `noyywrap` is used to remove the dependency on the global `yywrap()`
function that is not defined.

This example sets the global `yylval.num` to the integer scanned or
`yylval.str` to the string scanned.  It assumes that the yacc grammar
specification defines the tokens `CONST_NUMBER` and `CONST_STRING` and the type
`YYSTYPE` of `yylval`, which is a union.  For example:

<div class="alt">
```cpp
/* yacc grammar (.y file) */

%{
  extern int yylex(void);
%}

%union {         // YYSTYPE yylval is a union:
  int num;       // yylval.num
  char* str;     // yylval.str
}

%token <num> CONST_NUMBER
%token <str> CONST_STRING

%%
...  // grammar rules
%%
```
</div>

When option `−−flex` is used with `−−bison`, the `yytext`, `yyleng`,
and `yylineno` globals are accessible to the Bison/Yacc parser.  In fact, all
Flex actions and variables are globally accessible (outside \ref reflex-spec-rules
of the lexer specification) with the exception of `yy_push_state`,
`yy_pop_state`, and `yy_top_state` that are class methods.  Furthermore, `yyin`
and `yyout` are macros and cannot be (re)declared or accessed as global
variables, but these can be used as if they are variables to assign a new input
source and to set the output stream.  To avoid compilation errors when using
globals such as `yyin`, use **reflex** option `−−header-file` to generate a
header file `lex.yy.h` to include in your code.  Finally, in code outside of
\ref reflex-spec-rules you must use `yyinput()` instead of `input()`, use the
global action `yyunput()` instead of `unput()`, and use the global action
`yyoutput()` instead of `output()`.

See the generated `lex.yy.cpp` BISON section, which contains declarations
specific to Bison when the `−−bison` option is used.

There are two approaches for a Bison parser to work with a scanner.  Either the
yacc grammar specification for Bison should include the externs we need to
import from the scanner:

<div class="alt">
```cpp
/* yacc grammar (.y file) assuming C with externs defined by the scanner using YY_EXTERN_C */

%{
  extern int yylex(void);
  extern char *yytext;
  extern yy_size_t yyleng;
  extern int yylineno;
%}

%%
...  // grammar rules
%%
```
</div>

or a better approach is to generate a `lex.yy.h` header file with option
`--header-file` and use this header file in the yacc grammar specification:

<div class="alt">
```cpp
/* yacc grammar (.y file) for C++ */

%{
  #include "lex.yy.h"
%}

%%
...  // grammar rules
%%
```
</div>

The second option requires the generated parser to be compiled in C++, because
`lex.yy.h` contains C++ declarations.

@note The ugly Flex macro `YY_DECL` is not supported by RE/flex.  This macro is
needed with Flex to redeclare the `yylex()` function signature, for example to
take an additional `yylval` parameter that must be passed through from
`yyparse()` to `yylex()`.  Because the generated scanner uses a Lexer class for
scanning, the class can be extended with `%%class{` and `%}` to hold state
information and additional token-related values.  These values can then be
exchanged with the parser using getters and setters, which is preferred over
changing the `yylex()` function signature with `YY_DECL`.

🔝 [Back to table of contents](#)


### Bison and thread-safety                             {#reflex-bison-mt-safe}

Bison and Yacc are not thread-safe because the generated code uses and updates
global variables.  Yacc and Bison use the global variable `yylval` to exchange
token values.  By contrast, thread-safe reentrant Bison parsers pass the
`yylval` to the `yylex()` function as a parameter.  RE/flex supports all of
these Bison-specific features.

The following combinations of options are available to generate scanners for
Bison:

  Options                                       | Method                                    | Global functions and variables
  --------------------------------------------- | ----------------------------------------- | ------------------------------
  &nbsp;                                        | `int Lexer::lex()`                        | no global variables, but doesn't work with Bison
  `−−flex`                                      | `int yyFlexLexer::yylex()`                | no global variables, but doesn't work with Bison
  `−−bison`                                     | `int Lexer::lex()`                        | `Lexer YY_SCANNER`, `int yylex()`, `YYSTYPE yylval`
  `−−flex` `−−bison`                            | `int yyFlexLexer::yylex()`                | `yyFlexLexer YY_SCANNER`, `int yylex()`, `YYSTYPE yylval`, `char *yytext`, `yy_size_t yyleng`, `int yylineno`
  `−−bison` `−−reentrant`                       | `int Lexer::lex()`                        | `int yylex(yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)` 
  `−−flex` `−−bison` `−−reentrant`              | `int yyFlexLexer::lex()`                  | `int yylex(yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)`
  `−−bison-bridge`                              | `int Lexer::lex(YYSTYPE& yylval)`         | `int yylex(YYSTYPE*, yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)`
  `−−flex` `−−bison-bridge`                     | `int yyFlexLexer::yylex(YYSTYPE& yylval)` | `int yylex(YYSTYPE*, yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)` 
  `−−bison-locations`                           | `int Lexer::lex(YYSTYPE& yylval)`         | `Lexer YY_SCANNER`, `int yylex(YYSTYPE*, YYLTYPE*)` 
  `−−flex` `−−bison-locations`                  | `int yyFlexLexer::yylex(YYSTYPE& yylval)` | `yyFlexLexer YY_SCANNER`, `int yylex(YYSTYPE*, YYLTYPE*)` 
  `−−bison-locations` `−−bison-bridge`          | `int Lexer::lex(YYSTYPE& yylval)`         | `int yylex(YYSTYPE*, YYLTYPE*, yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)` 
  `−−flex` `−−bison-locations` `−−bison-bridge` | `int yyFlexLexer::yylex(YYSTYPE& yylval)` | `int yylex(YYSTYPE*, YYLTYPE*, yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)` 

Option `−−prefix` may be used with option `−−flex` to change the prefix of the
generated `yyFlexLexer` and `yylex`.  This option can be combined with option
`−−bison` to also change the prefix of the generated `yytext`, `yyleng`, and
`yylineno`.

The following sections explain the `−−bison` with `−−reentrant`,
`−−bison-bridge` and `−−bison-locations` options for **reflex**.

🔝 [Back to table of contents](#)


### Bison-bridge                                         {#reflex-bison-bridge}

The **reflex** option `−−bison-bridge` expects a Bison "pure parser" that is
declared as follows in a Yacc specification:

<div class="alt">
```cpp
/* yacc grammar (.y file) for C++ */

%{
  #include "lex.yy.h"
  #define YYPARSE_PARAM scanner
  #define YYLEX_PARAM   scanner
%}

%pure-parser

%%
...  // grammar rules
%%
```
</div>

With the `−−bison-bridge` option of **reflex**, the `yyscan_t` argument type of
`yylex()` is a `void*` type that passes the scanner object to this global
function (as defined by `YYPARSE_PARAM` and `YYLEX_PARAM`).  The function then
invokes this scanner's lex function.  This option also passes the `yylval`
value to the lex function, which is a reference to an `YYSTYPE` value.

Wtih the `−−bison-bridge` option two additional functions are generated that
should be used to create a new scanner and delete the scanner in your program:

<div class="alt">
```cpp
yyscan_t scanner = nullptr;
yylex_init(&scanner);                // create a new scanner
...
int token = yylex(&yylval, scanner); // scan with bison-bridge
...
yylex_destroy(scanner);              // delete a scanner
scanner = nullptr;
```
</div>

🔝 [Back to table of contents](#)


### Bison-locations                                   {#reflex-bison-locations}

The option `−−bison-locations` expects a Bison parser with the locations
feature enabled.  This feature provides line and column numbers of the matched
text for error reporting.  For example:

<div class="alt">
```cpp
/* yacc grammar (.y file) for C++ */

%{
  #include "lex.yy.h"
  void yyerror(const char*);
%}

%locations

%union {         // YYSTYPE yylval is a union:
  int num;       // type of yylval.num is int
  char* str;     // type of yylval.str is char*
}

%{
  /* reflex option --bison-locations makes yylex() take yylval and yylloc */
  extern int yylex(YYSTYPE*, YYLTYPE*);
  #define YYLEX_PARAM &yylval, &yylloc
%}

/* add &yylval and &yyloc parameters to yylex() with a trick: use YYLEX_PARAM */
%lex-param { void *YYLEX_PARAM }

%token <num> CONST_NUMBER
%token <str> CONST_STRING

%%
...  // grammar rules
%%

void yyerror(const char *msg)
{
  fprintf(stderr, "%s at %d,%d to line %d,%d\n",
      msg,
      yylloc.first_line,
      yylloc.first_column,
      yylloc.last_line,
      yylloc.last_column);
}
```
</div>

The `yylval` value is passed to the lex function.  The `yylloc` structure is
automatically set by the RE/flex scanner, so you do not need to define a
`YY_USER_ACTION` macro as you have to with Flex.

Note that with the `−−bison-location` option, `yylex()` takes an additional
`YYLTYPE` argument that a Bison parser provides.

🔝 [Back to table of contents](#)


### Bison-bridge & locations                   {#reflex-bison-bridge-locations}

Here is a final example that combines options `−−bison-locations` and
`−−bison-bridge`,  The Bison parser should be a Bison pure-parser with
locations enabled:

<div class="alt">
```cpp
/* yacc grammar (.y file) */

%{
  #include "lex.yy.h"
  void yyerror(YYLTYPE*, yyscan_t, const char*);
  #define YYPARSE_PARAM scanner
  #define YYLEX_PARAM   scanner
%}

%locations
%pure-parser
%lex-param { void *scanner }
%parse-param { void *scanner }

%union {         // YYSTYPE yylval is a union:
  int num;       // yylval.num
  char* str;     // yylval.str
}

%token <num> CONST_NUMBER
%token <str> CONST_STRING

%%
...  // grammar rules
%%

void yyerror(YYLTYPE *yylloc, yyscan_t scanner, const char *msg)
{
  fprintf(stderr, "%s at %d,%d to line %d,%d\n",
    msg,
    yylloc.first_line,
    yylloc.first_column,
    yylloc.last_line,
    yylloc.last_column);
}
```
</div>

@note when Bison `%locations` with `%define api.pure full` is used, `yyerror`
has the signature `void yyerror(YYLTYPE *locp, char const *msg)`.  This
function signature is required to obtain the location information with Bison
pure-parsers.

@note `yylval` is not a pointer argument but is always passed by reference and
can be used as such in the scanner's rules.

@note Because `YYSTYPE` is declared by the parser, do not forget to add a
`#include "y.tab.h"` to the top of the specification of your lexer:

<div class="alt">
```cpp
%top{
  #include "y.tab.h"    /* include y.tab.h generated by bison */
%}
```
</div>

With the `−−bison-bridge` and `−−bison-location` options two additional
functions are generated that should be used to create a new scanner and delete
the scanner in your program:

<div class="alt">
```cpp
yyscan_t scanner = nullptr;
yylex_init(&scanner);      // create a new scanner
...
int token = yylex(&yylval, &yylloc, scanner); // scan with bison-bridge and bison-locations
...
yylex_destroy(scanner);    // delete a scanner
scanner = nullptr;
```
</div>

🔝 [Back to table of contents](#)


### Reentrant Bison                                         {#reflex-reentrant}

When **reflex** is used as a Flex replacement with option `−−flex` and
`−−bison`, option `-R` or `−−reentrant` can be used to generate a reentrant
scanner.  This is only useful if you use both options `−−flex` and `−−bison`,
for example when migrating an existing project from Flex to **reflex**, because
**reflex** by defaul generates is MT-safe scanners.  See also \ref
reflex-bison.

When using reentrant scanners, your code should create a `yyscan_t` scanner
object with `yylex_init(&scanner)` and destroy it with
`yylex_destroy(scanner)`.  Reentrant Flex functions take the scanner object as
an extra last argument.

With the `−−reentrant` option two additional functions are generated that should
be used to create a new scanner and delete the scanner in your program:

<div class="alt">
```cpp
yyscan_t scanner = nullptr;
yylex_init(&scanner);       // create a new scanner
...
int token = yylex(scanner); // reentrant scan
...
yylex_destroy(scanner);     // delete a scanner
scanner = nullptr;
```
</div>

Within a rules section you should refer to the scanner with `yyscanner`.

The following additional functions are available in a reentrant Flex scanner,
which are useful outside of the rules section (you can still use the usual
Flex functions in the rules section):

  Reentrant Flex action       | Result
  --------------------------- | -----------------------------------------------
  `yyget_text(s)`             | 0-terminated text match
  `yyget_leng(s)`             | size of the match in bytes
  `yyget_lineno(s)`           | line number of match (>=1)
  `yyget_in(s)`               | get `reflex::Input` object
  `yyset_in(i, s)`            | set `reflex::Input` object
  `yyget_out(s)`              | get `std::ostream` object
  `yyset_out(o, s)`           | set output to `std::ostream o`
  `yyget_debug(s)`            | reflex option `-d` sets `n=1`
  `yyset_debug(n, s)`         | reflex option `-d` sets `n=1`
  `yyrestart(i, s)`           | set input to `reflex::Input i`
  `yyinput(s)`                | get next 8-bit char from input
  `yyunput(c, s)`             | put back 8-bit char `c`
  `yyoutput(c, s)`            | output char `c`
  `yy_create_buffer(i, n, s)` | new matcher `reflex::Input i`
  `yy_delete_buffer(m, s)`    | delete matcher `m`
  `yypush_buffer_state(m, s)` | push current matcher, use `m`
  `yypop_buffer_state(s)`     | pop matcher and delete current
  `yy_switch_to_buffer(m, s)` | use matcher `m`
  `yyget_extra(s)`            | get user-defined extra parameter
  `yyset_extra(x, s)`         | set user-defined extra parameter

With respect to the last two functions, a scanner object has a `YY_EXTRA_TYPE
yyextra` value that is user-definable.  You can define the type in a lexer
specification with the `extra-type` option:

<div class="alt">
```cpp
%option flex bison reentrant
%option extra-type struct extra
struct extra { ... }; // type of the data to include in a FlexLexer
```
</div>

This is a crude mechanism originating in Flex' C legacy to add extra
user-defined values to a scanner class.  Because **reflex** is C++, you should
instead define a derived class that extends the `Lexer` or `FlexLexer` class,
see \ref #reflex-inherit.

🔝 [Back to table of contents](#)


POSIX versus Perl matching                                 {#reflex-posix-perl}
--------------------------

The **reflex** scanner generator gives you a choice of matchers to use in the
generated scanner, where the default is the POSIX RE/flex matcher engine.
Other options are the Boost.Regex matcher in POSIX mode or in Perl mode.

To use a matcher for the generated scanner, use one of these three choices:

  Option          | Matcher class used  | Mode  | Engine      
  --------------- | ------------------- | ----- | ----------------------------- 
  `-m reflex`     | `Matcher`           | POSIX | RE/flex lib (default choice)
  `-m boost`      | `BoostPosixMatcher` | POSIX | Boost.Regex 
  `-m boost-perl` | `BoostPerlMatcher`  | Perl  | Boost.Regex 

The POSIX matchers look for the *longest possible match* among the given set of
alternative patterns.  Perl matchers look for the *first match* among the given
set of alternative patterns.

POSIX is generally preferred for scanners, since it is easier to arrange rules
that may have partially overlapping patterns.  Since we are looking for the
longest match anyway, it does not matter which rule comes first.  The order
does not matter as long as the length of the matches differ.  When matches are
of the same length because multiple patterns match, then the first rule is
selected.

Consider for example the following specification if a lexer with rules that are
intended to match keywords and identifiers in some input text:

<div class="alt">
```cpp
%%

int                     out() << "int keyword\n;
interface               out() << "interface keyword\n;
float                   out() << "float keyword\n;
[A-Za-z][A-Za-z0-9]*    out() << "identifier\n";

%%
```
</div>

When the input to the scanner is the text `integer` then a POSIX matcher
selects the last rule, which is what we want because it is an identifier.

By contrast, a Perl matcher selects the first rule because it matches the first
part `int` of `integer`.  This is NOT what we want.  The same problem occurs
when the text `interface` is encountered on the input, which we want to
recognize as a separate keyword and not match against `int`.  Switching the
rules for `int` and `interface` fixes that problem.  But note that we cannot do
the same to fix matching `integer` as an identifier: when moving the last rule
up to the top we cannot match `int` any longer!

@note To prevent a Perl matcher from matching a keyword when an identifier
starts with the name of that keyword, we could use a lookahead pattern such as
`int(?=[^A-Za-z0-9_])` which is written in a lexer specification as
`int/[^A-Za-z0-9_]` with the `/` lookahead meta symbol.

Basically, a Perl matcher works in an *operational* mode by working the regex
pattern as a sequence of *operations* for matching, usually using backtracking
to find a matching pattern.

A POSIX matcher on the other hand is *declarative* and have a deeper foundation
in formal language theory.  An advantage of POSIX matchers is that regular
expressions can be compiled to deterministic finite state machines for
efficient matching.

@note POSIX matching still require moving the `int` matching rule before the
identifier matching rule.  Otherwise an `int` on the input will be matched by
the identifier rule.

Perl matchers generally support lazy quantifiers and group captures, while most
POSIX matchers do not (Boost.Regex in POSIX mode does not support lazy
quantifiers).  The RE/flex POSIX matcher supports lazy quantifiers, but not
group captures.  The added support for lazy quantifiers and word boundary
anchors in RE/flex matching offers a reasonably new and useful feature for
scanners that require POSIX mode matching.

🔝 [Back to table of contents](#)


Examples                                                     {#reflex-examples}
--------

Some lexer specification examples to generate scanners with RE/flex.

🔝 [Back to table of contents](#)

### Example 1

The following Flex specification counts the lines, words, and characters on the
input.  We use `yyleng` match text length to count 8-bit characters (bytes).

To build this example with RE/flex, use **reflex** option `−−flex` to generate
Flex-compatible "yy" variables and functions.  This generates a C++ scanner
class `yyFlexLexer` that is compatible with the Flex scanner class (assuming
Flex with option `-+` for C++).

<div class="alt">
```cpp
%{
  #include <stdio.h>
  int ch = 0, wd = 0, nl = 0;
%}

%option noyywrap
%option main

nl      \r?\n
wd      [^ \t\r\n]+

%%

{nl}    ch += yyleng; ++nl;
{wd}    ch += yyleng; ++wd;
.       ++ch;
<<EOF>> printf("%8d%8d%8d\n", nl, wd, ch); yyterminate();

%%
```
</div>

To generate a scanner with a global `yylex()` function similar to Flex in C
mode (i.e. without Flex option `-+`), use **reflex** option `−−bison` with the
specification shown above.  This option when combined with `−−flex` produces the
global "yy" functions and variables.  This means that you can use RE/flex
scanners with Bison (Yacc) and with any other C code, assuming everything is
compiled together with a C++ compiler.

🔝 [Back to table of contents](#)

### Example 2

An improved implementation drops the use of global variables in favor of Lexer
class member variables.  We also want to count Unicode letters with the `wd`
counter instead of ASCII letters, which are single bytes while Unicode UTF-8
encodings vary in size.  So we add the Unicode option and use `\w` to match
Unicode word characters.  Note that `.` (dot) matches Unicode, so the match
length may be longer than one character that must be counted.  We drop the
`−−flex` option and use RE/flex Lexer methods instead of the Flex "yy"
functions:

<div class="alt">
```cpp
%top{
  #include <iostream>
  #include <iomanip>
  using namespace std;
%}

%class{
  int ch, wd, nl;
%}

%init{
  ch = wd = nl = 0;
%}

%option unicode
%option main
%option full

nl      \r?\n
wd      (\w|\p{Punctuation})+

%%

{nl}    ch += size(); ++nl;
{wd}    ch += size(); ++wd;
.       ch += size();
<<EOF>> out() << setw(8) << nl << setw(8) << wd << setw(8) << ch << endl;
        return 0;

%%
```
</div>

This simple word count program differs slightly from the Unix wc utility,
because the wc utility counts words delimited by wide character spaces
(`iswspace`) whereas this program counts words made up from word characters
combined with punctuation.

🔝 [Back to table of contents](#)

### Example 3

The following RE/flex specification filters tags from XML documents and verifies
whether or not the tags are properly balanced.  Note that this example uses the
lazy repetitions to keep the patterns simple.  The XML document scanned should
not include invalid XML characters such as `/`, `<`, or `>` in attributes
(otherwise the tags will not match properly).  The `dotall` option allows `.`
(dot) to match newline in all patterns, similar to the `(?s)` modifier in
regexes.

<div class="alt">
```cpp
%top{
  #include <stdio.h>
%}

%class{
  int level;
%}

%init{
  level = 0;
%}

%o matcher=reflex dotall main
%x ATTRIBUTES

name                    [A-Za-z_:\x80-\xFF][-.0-9A-Za-z_:\x80-\xFF]*
pi                      <\?{name}
comment                 <!--.*?-->
open                    <{name}
close                   <\/{name}>
cdata                   <!\[CDATA\[.*?]]>
string                  \".*?\"|'.*?'

%%

{comment}               |
{cdata}                 // skip comments and CDATA sections

{pi}                    level++;
                        start(ATTRIBUTES);

{open}                  printf("%*s%s\n", level++, "", text() + 1);
                        start(ATTRIBUTES);

{close}                 matcher().less(size() - 1);
                        printf("%*s%s\n", --level, "", text() + 2);

<<EOF>>                 printf("Tags are %sbalanced\n", level ? "im" : "");
                        return 0;

<ATTRIBUTES>"/>"        --level;
                        start(INITIAL);

<ATTRIBUTES>">"         start(INITIAL);

<ATTRIBUTES>{name}      |
<ATTRIBUTES>{string}    // skip attribute names and strings

<*>.                    // skip anything else

%%
```
</div>

Note thay we restrict XML tag names to valid characters, including all UTF-8
sequences that run in the range `\x80`-`\xFF` per 8-bit character.  This
matches all Unicode characters U+0080 to U+10FFFF.

The `ATTRIBUTES` state is used to scan attributes and their quoted values
separately from the `INITIAL` state.  The `INITIAL` state permits quotes to
freely occur in character data, whereas the `ATTRIBUTES` state matches quoted
attribute values.

We use `matcher().less(size() - 1)` to remove the ending `>` from the match in
`text()`.  The `>` will be matched again, this time by the `<*>.` rule that
ignores it.  We could also have used a lookahead pattern `"</"{name}/">"` where
`X/Y` means look ahead for `Y` after `X`.

🔝 [Back to table of contents](#)

### Example 4

This example Flex specification scans non-Unicode C/C++ source code.  It uses
free space mode to enhance readability.

<div class="alt">
```cpp
%{
  #include <stdio.h>
%}

%o flex freespace main

directive       ^ \h* # (. | \\ \r? \n)+
name            [\u\l_] \w*
udec            0 | [1-9] \d*
uhex            0 [Xx] [[:xdigit:]]+
uoct            0 [0-7]+
int             [-+]? ({udec} | {uhex}) \
                  ([Ll]{0,2} [Uu]? | [Uu] [Ll]{0,2})
float           [-+] \d* (\d | \.\d | \d\.) \d* \
                  ([Ee][-+]? \d+)? \
                  [FfLl]?
char            L? ' (\\. | [^\\\n'])* '
string          L? \" (\\. | \\\r?\n | [^\\\n"])* \"

%%

\s+
"//" .*? \n
"/*" (.|\n)*? "*/"
{directive}     { printf("DIRECTIVE %s\n", yytext); }
{name}          { printf("NAME      %s\n", yytext); }
{int}           { printf("INT       %s\n", yytext); }
{float}         { printf("FLOAT     %s\n", yytext); }
{char}          { printf("CHAR      %s\n", yytext); }
{string}        { printf("STRING    %s\n", yytext); }
[[:punct:]]     { printf("PUNCT     %s\n", yytext); }
.               { printf("*** ERROR '%s' at line %d\n", yytext, yylineno); }

%%
```
</div>

Free space mode permits spacing between concatenations and alternations.  To
match a single space, use `" "` or `[ ]`.  Long patterns can continue on the
next line(s) when lines ends with an escape `\`.

In free space mode you MUST place actions in `{` and `}` blocks and other code
in `%{` and `%}`.

When used with option `unicode`, the scanner automatically recognizes and scans
Unicode identifier names.  Note that we can use `matcher().columno()` in the
error message.  The `matcher()` object associated with the Lexer offers several
other methods that Flex does not support.

🔝 [Back to table of contents](#)


Limitations                                               {#reflex-limitations}
-----------

The RE/flex matcher engine uses an efficient FSM.  There are known limitations
to FSM matching that apply to Lex/Flex and therefore also apply to the
**reflex** scanner generator and to the RE/flex matcher engine:

- Lookaheads (trailing contexts) must appear at the end of a pattern when using
  the RE/flex matcher, so `a/b` (which is the same as lookahead `a(?=b)`) is
  permitted, but `(a/b)c` and `a(?=b)c` are not.
- Lookaheads cannot be properly matched when the ending of the first part of
  the pattern matches the beginning of the second part, such as `zx*/xy*`,
  where the `x*` matches the `x` at the beginning of the lookahead pattern.
- Anchors and boundaries must appear at the start or at the end of a pattern.
  The begin of buffer/line anchors `\A` and `^`, end of buffer/line anchors
  `\z` and `$` and the word boundary anchors must start or end a pattern.  For
  example, `\<cow\>` is permitted, but `.*\Bboy` is not.

Current **reflex** tool limitations that may be removed in future versions:

- The `REJECT` action is not supported.
- Flex translations `%T` are not supported.

Boost.Regex library limitations:

- Lookbehinds `φ(?<=ψ)` with the Boost.Regex matcher engine should not look too
  far behind, because the input is shifted through a buffer which means that
  prior text is no longer available for matching.  The lookbehind pattern `ψ`
  should not match text that is longer than what pattern `φ` can match.
- Boost.Regex may fail to find the longest match when greedy repetition
  patterns such as `.*` are used.  Under certain conditions greedy repetitions
  may behave as lazy repetitions.  For example, the Boost.Regex engine may
  return the short match `abc` when the regex `a.*c` is applied to `abc abc`,
  instead of returning the full match `abc abc`.  The problem is caused by the
  limitations of Boost.Regex `match_partial` matching algorithm.  To work
  around this limitation, we suggest to make the repetition pattern as specific
  as possible and not overlap with the pattern that follows the repetition.
  *The easiest solution is to read the entire input* using **reflex** option
  `-B` (batch input).  For a stand-alone `BoostMatcher`, use the `buffer()`
  method.  We consider this Boost.Regex partial match behavior a bug, not a
  restriction, because *as long as backtracking on a repetition pattern is
  possible given some partial text, Boost.Regex should flag the result as a
  partial match instead of a full match.*

🔝 [Back to table of contents](#)


The RE/flex regex library                                              {#regex}
=========================

The RE/flex regex library consists of a set of C++ templates and classes that
encapsulate regex engines in a standard API for scanning, tokenizing,
searching, and splitting of strings, wide strings, files, and streams.

🔝 [Back to table of contents](#)


Boost matcher classes                                            {#regex-boost}
---------------------

The RE/flex regex library is a class hierarchy that has at the root an abstract
class `reflex::AbstractMatcher`.  Pattern types may differ between for matchers
so the `reflex::PatternMatcher` template class takes a pattern type and creates
a class that is complete except for the implementation of the `reflex::match()`
virtual method that requires a regex engine, such as Boost.Regex or the RE/flex
engine.

The `reflex::BoostMatcher` inherits `reflex::PatternMatcher<boost::regex>`, and
in turn the `reflex::BoostPerlMatcher` and `reflex::BoostPosixMatcher` are both
derived from `reflex::BoostMatcher`:

  ![](classreflex_1_1_boost_matcher__inherit__graph.png)

An instance of `reflex::BoostPerlMatcher` is initialized with flag `match_perl`
and the flag `match_not_dot_newline`, these are `boost::regex_constants` flags.
These flags are the only difference with the plain `reflex::BoostMatcher`.

An instance of `reflex::BoostPosixMatcher` creates a POSIX matcher.  This means
that lazy quantifiers are not supported and the *leftmost longest rule* applies
to pattern matching.  This instance is initialized with the flags `match_posix`
and `match_not_dot_newline`.

Boost.Regex is a powerful library.  The RE/flex regex API enhances this library
with operations to match, search, scan, and split data from a given input.  The
input can be a file, a string, or a stream.  Files that are UTF-8/16/32-encoded
are automatically decoded.  Further, streams can be of unlimited length because
internal buffering is used by the RE/flex regex API enhancements to efficiently
apply Boost.Regex pattern matching to streaming data.  This enhancement permits
pattern matching of interactive input from the console, such that searching and
scanning interactive input for matches will return these matches immediately.

@note The `reflex::BoostMatcher` extends the capabilities of Boost.Regex, which
does not natively support streaming input:

@note The Boost.Regex library requires the target text to be loaded into memory
for pattern matching.  It is possible to match a target text incrementally with
the `match_partial` flag and `boost::regex_iterator`.  Incremental matching can
be used to support matching on (possibly infinite) streams.  To use this method
correctly, a buffer should be created that is large enough to hold the text for
each match.  The buffer must adjust with a growing size of the matched text, to
ensure that long matches that do not fit the buffer are not discared.

@note Boost.Regex `Boost.IOStreams` with `regex_filter` loads the entire stream
into memory which does not permit pattern matching of streaming and interactive
input data.

A `reflex::BoostMatcher` (or `reflex::BoostPerlMatcher`) engine is created from
a `boost::regex` object, or string regex, and some given input for normal (Perl
mode) matching:

```cpp
#include <reflex/boostmatcher.h>

reflex::BoostMatcher matcher( boost::regex or string, reflex::Input [, "options"] )
```

A `reflex::BoostPosixMatcher` engine is created from a `boost::regex` object,
or string regex, and some given input for POSIX mode matching:

```cpp
#include <reflex/boostmatcher.h>

reflex::BoostPosixMatcher matcher( boost::regex or string, reflex::Input [, "options"] )
```

For input you can specify a string, a wide string, a file, or a stream object.

Use option `"N"` to permit empty matches (nullable results).

You can convert an expressive regex of the form defined in \ref reflex-patterns
to a regex that the boost::regex engine can handle:

```cpp
#include <reflex/boostmatcher.h>

static const std::string regex = reflex::BoostMatcher::convert( string, [ flags ]);

reflex::BoostMatcher matcher( regex, reflex::Input [, "options"] )
```

The converter is specific to the matcher selected, i.e.
`reflex::BoostMatcher::convert`, `reflex::BoostPerlMatcher::convert`, and
`reflex::BoostPosixMatcher::convert`.  The converters also translates Unicode
`\p` character classes to UTF-8 patterns, converts bracket character classes
containing Unicode, and groups UTF-8 multi-byte sequences in the regex string.

The converter throws a `reflex::regex_error` exception if conversion fails,
for example when the regex syntax is invalid.

See \ref reflex-patterns for more details on regex patterns.

See \ref regex-input for more details on the `reflex::Input` class.

See \ref regex-methods for more details on pattern matching methods.

See \ref regex-convert for more details on regex converters.

🔝 [Back to table of contents](#)


std::regex matcher classes                                         {#regex-std}
--------------------------

The `reflex::StdMatcher` class inherits `reflex::PatternMatcher<std::regex>` as
a base.  The `reflex::StdEcmaMatcher` and `reflex::StdPosixMatcher` are derived
classes from `reflex::StdMatcher`:

  ![](classreflex_1_1_std_matcher__inherit__graph.png)

An instance of `reflex::StdEcmaMatcher` is initialized with regex syntax option
`std::regex::ECMAScript`.  This is also the default std::regex syntax.

An instance of `reflex::StdPosixMatcher` creates a POSIX AWK-based matcher.  So
that lazy quantifiers are not supported and the *leftmost longest rule* applies
to pattern matching.  This instance is initialized with the regex syntax option
`std::regex::awk`.

The C++11 std::regex library does not support `match_partial` that is needed to
match patterns on real streams with an adaptive internal buffer that grows when
longer matches are made when more input becomes available.  Therefore all input
is buffered with the C++11 std::regex class matchers.

With respect to performance, as of this time of writing, std::regex matching is
much slower than Boost.Regex, slower by a factor 10 or more.

The std::regex syntax is more limited than Boost.Regex and RE/flex regex.  Also
the matching behavior differs and cannot be controlled with mode modifiers:

- `.` (dot) matches anything except `\0` (NUL);
- `\177` is erroneously interpreted as a backreference, `\0177` does not match;
- `\x7f` is not supported in POSIX mode;
- `\cX` is not supported in POSIX mode;
- `\Q..\E` is not supported;
- no mode modifiers `(?imsux:φ)`;
- no `\A`, `\z`, `\<` and `\>` anchors;
- no `\b` and `\B` anchors in POSIX mode;
- no non-capturing groups `(?:φ)` in POSIX mode;
- empty regex patterns and matcher option `"N"` (nullable) may cause issues
  (std::regex `match_not_null` is buggy);
- `interactive()` is not supported.

To avoid these limitations on the std::regex syntax imposed, you can convert an
expressive regex of the form defined in section \ref reflex-patterns to a regex
that the std::regex engine can handle:

```cpp
#include <reflex/stdmatcher.h>

static const std::string regex = reflex::StdMatcher::convert( string, [ flags ]);

reflex::StdMatcher matcher( regex, reflex::Input [, "options"] )
```

The converter is specific to the matcher selected, i.e.
`reflex::StdMatcher::convert`, `reflex::StdEcmaMatcher::convert`, and
`reflex::StdPosixMatcher::convert`.  The converters also translates Unicode
`\p` character classes to UTF-8 patterns, converts bracket character classes
containing Unicode, and groups UTF-8 multi-byte sequences in the regex string.

The converter throws a `reflex::regex_error` exception if conversion fails,
for example when the regex syntax is invalid.

See \ref reflex-patterns for more details on regex patterns.

See \ref regex-input for more details on the `reflex::Input` class.

See \ref regex-methods for more details on pattern matching methods.

See \ref regex-convert for more details on regex converters.

🔝 [Back to table of contents](#)


The RE/flex matcher class                                      {#regex-matcher}
-------------------------

The RE/flex framework includes a POSIX regex matching library `reflex::Matcher`
that inherits the API from `reflex::PatternMatcher<reflex::Pattern>`:

  ![](classreflex_1_1_matcher__inherit__graph.png)

where the RE/flex `reflex::Pattern` class represents a regex pattern.  Patterns
as regex texts are internally compiled into deterministic finite state machines
by the `reflex::Pattern` class.  The machines are used by the `reflex::Matcher`
for fast matching of regex patterns on some given input.  The `reflex::Matcher`
can be much faster than the Boost.Regex matchers.

A `reflex::Matcher` engine is constructed from a `reflex::Pattern` object, or a
string regex, and some given input:

```cpp
#include <reflex/matcher.h>

reflex::Matcher matcher( reflex::Pattern or string, reflex::Input [, "options"] )
```

The regex is specified as a string or a `reflex::Pattern` object, see \ref
regex-pattern below.

Use option `"N"` to permit empty matches (nullable results).  Option `"T=8"`
sets the tab size to 8 for \ref reflex-pattern-dents matching.

For input you can specify a string, a wide string, a file, or a stream object.

A regex string with Unicode patterns can be converted for Unicode matching as
follows:

```cpp
#include <reflex/matcher.h>

static const std::string regex = reflex::Matcher::convert( string, [ flags ]);

reflex::Matcher matcher( regex, reflex::Input [, "options"] )
```

The converter is specific to the matcher and translates Unicode `\p` character
classes to UTF-8 patterns, converts bracket character classes containing
Unicode, and groups UTF-8 multi-byte sequences in the regex string.

See \ref reflex-patterns for more details on regex patterns.

See \ref regex-input for more details on the `reflex::Input` class.

See \ref regex-methods for more details on pattern matching methods.

See \ref regex-convert for more details on regex converters.

🔝 [Back to table of contents](#)


The RE/flex pattern class                                      {#regex-pattern}
-------------------------

The `reflex::Pattern` class is used by the `reflex::matcher` for pattern
matching.  The `reflex::Pattern` class converts a regex pattern to an efficient
FSM and takes a regex string and options to construct the FSM internally:

```cpp
#include <reflex/matcher.h>

[static] reflex:Pattern pattern(string [, "options"] )
```

It is recommended to create a static instance of the pattern if the regex
string is fixed.  This avoids repeated FSM construction.

The following options are combined in a string and passed to the
`reflex::Pattern` contructor:

  Option        | Effect
  ------------- | -------------------------------------------------------------
  `b`           | bracket lists are parsed without converting escapes
  `e=c;`        | redefine the escape character
  `f=file.cpp;` | save finite state machine code to file.cpp
  `f=file.gv;`  | save deterministic finite state machine to file.gv
  `i`           | case-insensitive matching, same as `(?i)X`
  `l`           | Lex-style trailing context with `/`, same as `(?l)X`
  `m`           | multiline mode, same as `(?m)X`
  `n=name;`     | use `reflex_code_name` for the machine (instead of FSM)
  `q`           | Lex-style quotations "..." equals `\Q...\E`, same as `(?q)X`
  `r`           | throw regex syntax error exceptions (not just fatal errors)
  `s`           | dot matches all (aka. single line mode), same as `(?s)X`
  `x`           | inline comments, same as `(?x)X`
  `w`           | display regex syntax errors before raising them as exceptions

The compilation of a `reflex::Pattern` object into a FSM may throw an exception
when the regex string has problems:

```cpp
try
{
  reflex::Pattern pattern(argv[1]);
  ...
  // code that uses the pattern object
  ...
}
catch (reflex::regex_error& e)
{
  std::cerr << e.what();
  switch (e)
  {
    case reflex::regex_error::mismatched_parens:    std::cerr << "mismatched ( )"; break;
    case reflex::regex_error::mismatched_braces:    std::cerr << "mismatched { }"; break;
    case reflex::regex_error::mismatched_brackets:  std::cerr << "mismatched [ ]"; break;
    case reflex::regex_error::mismatched_quotation: std::cerr << "mismatched \Q...\E quotation"; break;
    case reflex::regex_error::empty_expression:     std::cerr << "regex (sub)expression should not be empty"; break;
    case reflex::regex_error::empty_class:          std::cerr << "class [...] is empty, e.g. [a&&[b]]"; break;
    case reflex::regex_error::invalid_class:        std::cerr << "invalid character class name"; break;
    case reflex::regex_error::invalid_class_range:  std::cerr << "invalid class range, e.g. [Z-A]"; break;
    case reflex::regex_error::invalid_escape:       std::cerr << "invalid escape character"; break;
    case reflex::regex_error::invalid_anchor:       std::cerr << "invalid anchor"; break;
    case reflex::regex_error::invalid_repeat:       std::cerr << "invalid repeat range, e.g. {10,1}"; break;
    case reflex::regex_error::invalid_quantifier:   std::cerr << "invalid lazy/possessive quantifier"; break;
    case reflex::regex_error::invalid_modifier:     std::cerr << "invalid (?ismux:) modifier"; break;
    case reflex::regex_error::invalid_syntax:       std::cerr << "invalid regex syntax"; break;
    case reflex::regex_error::exceeds_limits:       std::cerr << "exceeds complexity limits: {n,m} range too large"; break;
  }
}
```

🔝 [Back to table of contents](#)


Regex converters                                               {#regex-convert}
----------------

To work around limitations of regex libraries and to support Unicode matching,
RE/flex offers converters to translate expressive regex syntax forms (with
Unicode patterns defined in section \ref reflex-patterns) to regex strings
that the selected regex engines can handle.

The converters translate `\p` Unicode classes, translate character
class set operations such as `[a-z−−[aeiou]]`, convert escapes such as `\X`,
and enforce `(?imsux:φ)` mode modifiers to a regex string that the underlying
regex library understands and can use.

Each converter is specific to the regex engine.  Use a converter for the
matcher of your choice:

- `std::string reflex::BoostMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts `regex` for use with Boost.Regex;
- `std::string reflex::BoostPerlMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts `regex` for use with Boost.Regex in Perl mode;
- `std::string reflex::BoostPosixMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts `regex` for use with Boost.Regex in POSIX mode;
- `std::string reflex::StdMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts `regex` for use with C++ std::regex;
- `std::string reflex::StdEcmaMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts `regex` for use with C++ std::regex in ECMA mode;
- `std::string reflex::StdPosixMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts `regex` for use with C++ std::regex in POSIX mode;
- `std::string reflex::Matcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts `regex` for use with the RE/flex POSIX regex library;

where `flags` is optional.  When specified, it can be a combination of the
following `reflex::convert_flag` flags:

  Flag        | Effect
  ----------- | ---------------------------------------------------------------
  `unicode`   | `.`, `\s`, `\w`, `\l`, `\u`, `\S`, `\W`, `\L`, `\U` match Unicode, same as `(?u)`
  `recap`     | remove capturing groups, add capturing groups to the top level
  `lex`       | convert Lex/Flex regular expression syntax
  `u4`        | convert `\uXXXX` (shorthand of `\u{XXXX}`)
  `anycase`   | convert regex to ignore case, same as `(?i)`
  `multiline` | regex has multiline anchors `^` and `$`, same as `(?m)`
  `dotall`    | convert `.` (dot) to match all, same as `(?s)`
  `freespace` | convert regex by removing spacing, same as `(?x)`

For example:

```cpp
#include <reflex/matcher.h> // reflex::Matcher, reflex::Input, reflex::Pattern

// use a Matcher to check if sentence is in Greek:
static const reflex::Pattern pattern(reflex::Matcher::convert("[\\p{Greek}\\p{Zs}\\pP]+", reflex::convert_flag::unicode));
if (reflex::Matcher(pattern, sentence).matches())
  std::cout << "This is Greek" << std::endl;
```

For example, the `unicode` flag and `dotall` flag can be used with a `.`
regex pattern to count wide characters in some `example` input:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

// construct a Boost.Regex matcher to count wide characters:
std::string regex = reflex::BoostMatcher::convert(".", reflex::convert_flag::unicode | reflex::convert_flag::dotall);
reflex::BoostMatcher boostmatcher(regex, example);
size_t n = std::distance(boostmatcher.scan.begin(), boostmatcher.scan.end());
```

You can also use the regex `"\X"` to match any wide character without using
these flags.

A converter throws a `reflex::regex_error` exception if conversion fails, for
example when the regex syntax is invalid:

```cpp
std::string regex;
try
{
  regex = reflex::BoostMatcher::convert(argv[1], reflex::convert_flag::unicode));
}
catch (reflex::regex_error& e)
{
  std::cerr << e.what();
  switch (e)
  {
    case reflex::regex_error::mismatched_parens:    std::cerr << "mismatched ( )"; break;
    case reflex::regex_error::mismatched_braces:    std::cerr << "mismatched { }"; break;
    case reflex::regex_error::mismatched_brackets:  std::cerr << "mismatched [ ]"; break;
    case reflex::regex_error::mismatched_quotation: std::cerr << "mismatched \Q...\E quotation"; break;
    case reflex::regex_error::empty_expression:     std::cerr << "regex (sub)expression should not be empty"; break;
    case reflex::regex_error::empty_class:          std::cerr << "class [...] is empty, e.g. [a&&[b]]"; break;
    case reflex::regex_error::invalid_class:        std::cerr << "invalid character class name"; break;
    case reflex::regex_error::invalid_class_range:  std::cerr << "invalid class range, e.g. [Z-A]"; break;
    case reflex::regex_error::invalid_escape:       std::cerr << "invalid escape character"; break;
    case reflex::regex_error::invalid_anchor:       std::cerr << "invalid anchor"; break;
    case reflex::regex_error::invalid_repeat:       std::cerr << "invalid repeat range, e.g. {10,1}"; break;
    case reflex::regex_error::invalid_quantifier:   std::cerr << "invalid lazy/possessive quantifier"; break;
    case reflex::regex_error::invalid_modifier:     std::cerr << "invalid (?ismux:) modifier"; break;
    case reflex::regex_error::invalid_syntax:       std::cerr << "invalid regex syntax"; break;
  }
}
```

🔝 [Back to table of contents](#)


Methods and iterators                                          {#regex-methods}
---------------------

The RE/flex abstract matcher provides four operations for matching with an
instance of a regex engine:

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `matches()` | returns nonzero if the input from begin to end matches
  `find()`    | search input and return nonzero if a match was found
  `scan()`    | scan input and return nonzero if input at current position matches
  `split()`   | return nonzero for a split of the input at the next match

These methods return a nonzero *"accept"* value for a match, meaning the
`size_t accept()` value that corresponds to a group capture (or one if no
groups are used).  The methods are repeatable, where the last three return
additional matches.

The `find`, `scan`, and `split` methods are also implemented as input iterators
that apply filtering tokenization, and splitting:

  Iterator range                  | Acts as a | Iterates over
  ------------------------------- | --------- | -------------------------------
  `find.begin()`...`find.end()`   | filter    | all matches
  `scan.begin()`...`scan.end()`   | tokenizer | continuous matches
  `split.begin()`...`split.end()` | splitter  | text between matches

🔝 [Back to table of contents](#)

### matches                                            {#regex-methods-matches}

The `matches()` method returns a nonzero "accept" value (the `size_t accept()`
group capture index value or the value 1 if no groups are used) if the given
input from begin to the end matches the specified pattern.

For example, to match a UUID string:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

std::string uuid = "123e4567-e89b-12d3-a456-426655440000";

if (reflex::BoostMatcher("[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}", uuid).matches())
  std::cout << uuid << " is a string in UUID format" << std::endl;
```

When executed this code prints:

    123e4567-e89b-12d3-a456-426655440000 is a string in UUID format

The `matches()` method returns the group capture index that can be used as a
selector.  For example:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

const char *food = "hotdog";

switch (reflex::BoostMatcher("(.*cat.*)|(.*dog.*)", food).matches())
{
  case 0: std::cout << food << " has not cat or dog" << std::endl;
          break;
  case 1: std::cout << food << " has a cat" << std::endl;
          break;
  case 2: std::cout << food << " has a dog" << std::endl;
          break;
}
```

See also \ref regex-methods-props.

🔝 [Back to table of contents](#)

### find                                                  {#regex-methods-find}

The `find()` method and `find.begin()`...`find.end()` iterator range are used
to search for a match in the given input.

The `find()` method returns a nonzero "accept" value (the `size_t accept()`
group capture index value or the value 1 if no groups are used) for a match
and zero otherwise.

For example, to find all words in a string:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
while (matcher.find() != 0)
  std::cout << matcher.text() << std::endl;
```

When executed this code prints:

    How
    now
    brown
    cow

The iterator range `find.begin()`...`find.end()` serves as an input filter.

For example, in C++11 we can use a range-based loop to loop over matches using
the `find` iterator:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

for (auto& match : reflex::BoostMatcher("\\w+", "How now brown cow.").find)
  std::cout << match.text() << std::endl;
```

Iterators can be used with STL algorithms and other iterator functions.  For
example to count words in a string:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
std::cout << std::distance(matcher.find.begin(), boostmatcher.find.end()) << std::endl;
```

When executed this code prints:

    4

The `find()` method returns the group capture index that can be used as a
selector.

See also \ref regex-methods-props.

🔝 [Back to table of contents](#)

### scan                                                  {#regex-methods-scan}

The `scan()` method and `scan.begin()`...`scan.end()` iterator range are
similar to `find` but generate continuous matches in the given input.

The `scan()` method returns a nonzero "accept" value (the `size_t accept()`
group capture index value or the value 1 if no groups are used) for a match
and zero otherwise.

For example, to scan for words, spacing, and punctuation in a sentence:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

const char *tokens[4] = { "n/a", "word", "space", "other" };
reflex::BoostMatcher matcher("(\\w+)|(\\s+)|(.)", "How now brown cow.");
size_t accept;
while ((accept = matcher.scan()) != 0)
  std::cout << tokens[accept] << std::endl;
```

When executed this code prints:

    word
    space
    word
    space
    word
    space
    word
    other

The iterator range `scan.begin()`...`scan.end()` serves as an input tokenizer
and produces continuous matches.

For example, tokenizing a string into a vector of numeric tokens:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

reflex::BoostMatcher matcher("(\\w+)|(\\s+)|(.)", "How now brown cow.");
std::vector<size_t> tokens(matcher.scan.begin(), matcher.scan.end());
std::copy(tokens.begin(), tokens.end(), std::ostream_iterator<size_t>(std::cout, " "));
```

When executed the code prints:

    1 2 1 2 1 2 1 3

If the pattern does not match the input immediately succeeding a previous
match, then the `scan()` method returns false and the iterator returns
`scan.end()`.  To determine if all input was scanned and end of input was
reached, use the `at_end()` method, see \ref regex-methods-props.

See also \ref regex-methods-props.

🔝 [Back to table of contents](#)

### split                                                {#regex-methods-split}

The `split()` method and `split.begin()`...`split.end()` iterator range return
text between matches in the given input.

The `split()` method returns a nonzero "accept" value (the `size_t accept()`
group capture index value or the value 1 if no groups are used) of the
matching text (that is, the text that comes after the split part) and zero
otherwise.

When matches are adjacent then empty text is returned.  Also the start of input
and end of input return text that may be empty.

For example, to split text into words by matching non-words:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

reflex::BoostMatcher matcher("\\W+", "How now brown cow.");
while (matcher.split() != 0)
  std::cout << "'" << matcher.text() << "'" << std::endl;
```

When executed this code prints:

    'How'
    'now'
    'brown'
    'cow'
    ''

This produces five text splits where the last text is empty because the period
at the end of the sentence matches the pattern and empty input remains.

The iterator range `split.begin()`...`split.end()` serves as an input splitter.

For example, to display the contents of a text file while normalizing spacing:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

reflex::BoostMatcher matcher("\\s+", fopen("abstract.txt", "r"));
if (matcher.in.file() != NULL)
{
  std::copy(matcher.split.begin(), matcher.split.end(), std::ostream_iterator<std::string>(std::cout, " "));
  fclose(matcher.in.file());
}
```

@note The "accept" value of the last split is `0xFFFF` (65535) to indicate that
no text after the split was matched.  The same value is returned by `accept()`.

See also \ref regex-methods-props.

🔝 [Back to table of contents](#)

### Properties of a match                                {#regex-methods-props}

To obtain properties of a match, use the following methods:

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `accept()`  | returns group capture index (or zero if not captured/matched)
  `text()`    | returns `const char*` to 0-terminated text match (ends in `\0`)
  `str()`     | returns `std::string` text match (preserves `\0`s)
  `wstr()`    | returns `std::wstring` wide text match (converted from UTF-8)
  `chr()`     | returns first 8-bit character of the text match (`str()[0]`)
  `wchr()`    | returns first wide character of the text match (`wstr()[0]`)
  `pair()`    | returns `std::pair<size_t,std::string>(accept(),str())`
  `wpair()`   | returns `std::pair<size_t,std::wstring>(accept(),wstr())`
  `size()`    | returns the length of the text match in bytes
  `wsize()`   | returns the length of the match in number of wide characters
  `begin()`   | returns `const char*` to non-0-terminated text match begin
  `end()`     | returns `const char*` to non-0-terminated text match end
  `rest()`    | returns `const char*` to 0-terminated rest of input
  `more()`    | tells the matcher to append the next match (adjacent matches)
  `less(n)`   | cuts `text()` to `n` bytes and repositions the matcher
  `lineno()`  | returns line number of the match, starting with line 1
  `columno()` | returns column number of the match, starting with 0
  `first()`   | returns input position of the first character of the match
  `last()`    | returns input position of the last + 1 character of the match
  `at_bol()`  | true if matcher reached the begin of a new line `\n`
  `at_bob()`  | true if matcher is at the start of input, no matches consumed
  `at_end()`  | true if matcher is at the end of input
  `[0]`       | operator returns `std::pair<const char*,size_t>(begin(),size())`
  `[n]`       | operator returns n'th capture `std::pair<const char*,size_t>`

Note that `begin()`, `operator[0]`, and `operator[n]` return non-0-terminated
strings.  You must use `end()` with `begin()` to determine the span of the
match.  Use the size of the capture to determine the end of the captured match.

All methods take constant time to execute except for `str()`, `wstr()`,
`pair()`, `wpair()`, `wsize()`, `lineno()` and `columno()` that require a pass
over the (matched) text.

The `chr()` and `wchr()` methods are much more efficient than `str()[0]` (or
`text()[0]`) and `wstr()[0]`, respectively.

In addition, the following type casts of matcher objects and iterators can be
used for convenience:

- Casting to `size_t` gives the matcher's `accept()` index.
- Casting to `std::string` is the same as invoking `str()`
- Casting to `std::wstring` is the same as invoking `wstr()`.
- Casting to `std::pair<size_t,std::string>` is the same as `pair()`.
- Casting to `std::pair<size_t,std::wstring>` is the same as `wpair()`.

The following example prints some of the properties of each match:

```cpp
#include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
while (matcher.find() != 0)
  std::cout <<
    "accept: " << matcher.accept() <<
    "text:   " << matcher.text() <<
    "size:   " << matcher.size() <<
    "line:   " << matcher.lineno() <<
    "column: " << matcher.columno() <<
    "first:  " << matcher.first() <<
    "last:   " << matcher.last() << std::endl;
```

When executed this code prints:

    accept: 1 text: How size: 3 line: 1 column: 0 first: 0 last: 3
    accept: 1 text: now size: 3 line: 1 column: 4 first: 4 last: 7
    accept: 1 text: brown size: 5 line: 1 column: 8 first: 8 last: 13
    accept: 1 text: cow size: 3 line: 1 column: 14 first: 14 last: 17

🔝 [Back to table of contents](#)

### Public data members                                        {#regex-members}

Four public data members of a matcher object are accesible:

  Variable | Usage
  -------- | ------------------------------------------------------------------
  `in`     | the `reflex::Input` object used by the matcher
  `find`   | the `reflex::AbstractMatcher::Operation` functor for searching
  `scan`   | the `reflex::AbstractMatcher::Operation` functor for scanning
  `split`  | the `reflex::AbstractMatcher::Operation` functor for splitting

Normally only the `in` variable should be used which holds the current input
object of the matcher.  See \ref regex-input for details.

The functors provide `begin()` and `end()` methods that return iterators and
hold the necessary state information for the iterators.  A functor invocation
essentially invokes the corresponding method listed in \ref regex-methods.

🔝 [Back to table of contents](#)

### Pattern methods                                    {#regex-methods-pattern}

To change a matcher's pattern or check if a pattern was assigned, you can use
the following methods:

  Method          | Result
  --------------- | -----------------------------------------------------------
  `pattern(p)`    | set pattern to `p` (string regex or `reflex::Pattern`)
  `has_pattern()` | true if the matcher has a pattern assigned to it
  `own_pattern()` | true if the matcher has a pattern to manage and delete
  `pattern()`     | get the pattern object, `reflex::Pattern` or `boost::regex`

The first method returns a reference to the matcher, so multiple method
invocations can be chained together.

🔝 [Back to table of contents](#)

### Input methods                                        {#regex-methods-input}

To assign a new input source to a matcher or set the input to buffered or
interactive, you can use the following methods:

  Method          | Result
  --------------- | -----------------------------------------------------------
  `input(i)`      | set input to `reflex::Input i` (string, stream, or `FILE*`)
  `buffer()`      | buffer all input at once, returns true if successful
  `buffer(n)`     | set the adaptive buffer size to `n` bytes to buffer input
  `interactive()` | sets buffer size to 1 for console-based (TTY) input
  `flush()`       | flush the remaining input from the internal buffer
  `reset()`       | resets the matcher, restarting it from the remaining input
  `reset(o)`      | resets the matcher with new options string `o` ("A?N?T?")

The first method returns a reference to the matcher, so multiple method
invocations can be chained together.

Four special methods can be used to read the input stream provided to a matcher
directly, even when you use the matcher's search and match methods:

  Method     | Result
  ---------- | ----------------------------------------------------------------
  `input()`  | returns next 8-bit char from the input, matcher then skips it
  `winput()` | returns next wide character from the input, matcher skips it
  `unput(c)` | put char `c` back unto the stream, matcher then takes it
  `peek()`   | returns next 8-bit char from the input without consuming it

The `input()`, `winput()`, and `peek()` methods return a non-negative character
code and EOF (-1) when the end of input is reached.

A matcher reads from the specified input source using its virtual method
`size_t get(char *s, size_t n)` that simply returns `in.get(s, n)`, that is,
the result of the `reflex::Input::get(s, n)` method of the `reflex::Input`
object.  This method can be overriden by a derived matcher class to customize
reading.

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `get(s, n)` | fill `s[0..n-1]` with next input, returns number of bytes read
  `wrap()`    | returns false (can be overriden to wrap input after EOF)

When a matcher reaches the end of input, it invokes the virtual method `wrap()`
to check if more input is available.  This method returns false by default, but
this behavior can be changed by overring `wrap()` to set a new input source and
return `true`, for example:

```cpp
class WrappedInputMatcher : public reflex::Matcher {
 public:
  WrappedInputMatcher() : reflex::Matcher(), source_select(0)
  { }
 private:
  virtual bool wrap()
  {
    // read a string, a file, and a string:
    switch (source_select++)
    {
      case 0: in = "Hello World!";
              return true;
      case 1: in = fopen("hello.txt", "r");
              return in.file() != NULL;
      case 2: fclose(in.file());
              in = "Goodbye!";
              return true;
    }
    return false;
  }
  int source_select;
};
```

Note that the constructor in this example does not specify a pattern and input.
To set a pattern for the matcher after its instantiation use the `pattern(p)`
method.  In this case the input does not need to be specified, which allows us
to immediately force reading the sources of input that we assigned in our
`wrap()` method.

For details of the `reflex::Input` class, see \ref regex-input.

🔝 [Back to table of contents](#)


The Input class                                                  {#regex-input}
---------------

A matcher may accept several types of input, but can only read from one input
source at a time.  Input to a matcher is represented by a single
`reflex::Input` class instance that the matcher uses internally.

🔝 [Back to table of contents](#)

### Assigning input                                       {#regex-input-assign}

An input object can be constructed by specifying a string, a file, or a stream
to read from.  You can also reassign input to read from new input.

More specifically, you can pass a `std::string`, `char*`, `std::wstring`,
`wchar_t*`, `FILE*`, or a `std::istream` to the constructor.

A `FILE*` file descriptor is a special case.  The input object handles various
file encodings.  If a UTF Byte Order Mark (BOM) is detected then the UTF input
will be normalized to UTF-8.  When no UTF BOM is detected then the input is
considered plain ASCII, binary, or UTF-8 and passed through unconverted.  To
override the file encoding when no UTF BOM was present, and normalize Latin-1,
ISO-8859-1, CP-1252, CP 434, CP 850, EBCDIC, and other encodings to UTF-8, see
\ref regex-input-file.

🔝 [Back to table of contents](#)

### Input strings                                        {#regex-input-strings}

An input object constructed from an 8-bit string `char*` or `std::string` just
passes the string to the matcher engine.  The string should contain UTF-8 when
Unicode patterns are used.

An input object constructed from a wide string `wchar_t*` or `std::wstring`
translates the wide string to UTF-8 for matching, which effectively normalizes
the input for matching with Unicode patterns.  This conversion is illustrated
below.  The copyright symbol `©` with Unicode U+00A9 is matched against its
UTF-8 sequence `C2 A9` of `©`:

```cpp
if (reflex::Matcher("©", L"©").matches())
  std::cout << "copyright symbol matches\n";
```

To ensure that Unicode patterns in UTF-8 strings are grouped properly, use \ref
regex-convert, for example as follows:

```cpp
static reflex::Pattern CR(reflex::Matcher::convert("(?u:\u{00A9})"));
if (reflex::Matcher(CR, L"©").matches())
  std::cout << "copyright symbol matches\n";
```

Here we made the converted pattern static to avoid repeated conversion and
construction overheads.

@note Strings cannot contain the terminating `\0` (NUL) character.  To match
raw binary content that contains zeros, use a `std::istringstream`.

🔝 [Back to table of contents](#)

### Input streams                                        {#regex-input-streams}

An input object constructed from a `std::istream` (or a derived class) just
passes the text to the matcher engine.  The stream should contain UTF-8 when
Unicode patterns are used.

🔝 [Back to table of contents](#)

### FILE encodings                                          {#regex-input-file}

File content specified with a `FILE*` file descriptor can be encoded in ASCII,
binary, UTF-8/16/32, ISO-8859-1, CP-1250 to CP-1258, CP 434, CP 850, or EBCDIC.

A [UTF Byte Order Mark (BOM)](www.unicode.org/faq/utf_bom.html) is detected in
the content of a file scanned by the matcher, which enables UTF-8 normalization
of the input automatically.

Otherwise, if no file encoding is explicitly specified, the matcher expects
UTF-8, ASCII, or plain binary.  However, other file formats can be
automatically decoded and translated to UTF-8 on the fly for matching.

The current file encoding used by a matcher is obtained with the
`reflex::Input::file_encoding()` method, which returns an
`reflex::Input::file_encoding` constant:

  Constant                                | File encoding
  --------------------------------------- | -----------------------------------
  `reflex::Input::file_encoding::plain`   | plain octets, ASCII/binary/UTF-8
  `reflex::Input::file_encoding::utf8`    | UTF-8 (BOM detected)
  `reflex::Input::file_encoding::utf16be` | UTF-16 big endian (BOM detected)
  `reflex::Input::file_encoding::utf16le` | UTF-16 little endian (BOM detected)
  `reflex::Input::file_encoding::utf32be` | UTF-32 big endian (BOM detected)
  `reflex::Input::file_encoding::utf32le` | UTF-32 little endian (BOM detected)
  `reflex::Input::file_encoding::latin`   | ASCII and Latin-1, ISO-8859-1
  `reflex::Input::file_encoding::cp437`   | CP 437
  `reflex::Input::file_encoding::cp850`   | CP 850 (updated to CP 858)
  `reflex::Input::file_encoding::ebcdic`  | EBCDIC
  `reflex::Input::file_encoding::cp1250`  | CP-1250
  `reflex::Input::file_encoding::cp1251`  | CP-1251
  `reflex::Input::file_encoding::cp1252`  | CP-1252
  `reflex::Input::file_encoding::cp1253`  | CP-1253
  `reflex::Input::file_encoding::cp1254`  | CP-1254
  `reflex::Input::file_encoding::cp1255`  | CP-1255
  `reflex::Input::file_encoding::cp1256`  | CP-1256
  `reflex::Input::file_encoding::cp1257`  | CP-1257
  `reflex::Input::file_encoding::cp1258`  | CP-1258
  `reflex::Input::file_encoding::custom`  | User-defined custom code page

To set the file encoding when assigning a file to read, use
`reflex::Input(file, enc)` with one of the encoding constants shown in the
table to construct the input object.

For example, use `reflex::Input::file_encoding::latin` to override the encoding
when the file contains ISO-8859-1.  This way you can match its content using
Unicode patterns (matcher engines internally normalizes ISO-8859-1 to UTF-8):

```cpp
reflex::Input input(stdin, reflex::Input::file_encoding::latin);
reflex::Matcher matcher(pattern, input);
```

This sets the standard input encoding to ISO-8859-1, but only if no UTF BOM was
detected on the standard input, because the UTF encoding of a `FILE*` that
starts with a UTF BOM cannot be overruled.

To define a custom code page to translate files, define a code page table with
256 entries that maps each 8-bit input character to a 16-bit Unicode character
(UCS-2).  Then use `reflex::Input::file_encoding::custom` with a pointer to
your code page to construct an input object.  For example:

```cpp
const unsigned short CP[256] = {
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
   32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
   48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
   64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
   80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
   96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
};
reflex::Input input(stdin, reflex::Input::file_encoding::custom, CP);
reflex::Matcher matcher(pattern, input);
```

This example translates all constrol characters and characters above 127 to
spaces before matching.

🔝 [Back to table of contents](#)

### Input properties                                  {#regex-input-properties}

To obtain the properties of an input source use the following methods:

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `size()`    | size of the input in total bytes (encoded) or zero when unknown
  `good()`    | input is available to read (no error and not EOF)
  `eof()`     | end of input (but use only `at_end()` with matchers!)
  `cstring()` | the current `const char*` (of a `std::string`) or NULL
  `wstring()` | the current `const wchar_t*` (of a `std::wstring`) or NULL
  `file()`    | the current `FILE*` file descriptor or NULL
  `istream()` | a `std::istream*` pointer to the current stream object or NULL

🔝 [Back to table of contents](#)


Examples                                                      {#regex-examples}
--------

Some examples to demonstrate the concepts discussed.

🔝 [Back to table of contents](#)

### Example 1

This example illustrates the `find` and `split` methods and iterators with a
RE/flex `reflex::Matcher` and a `reflex::BoostMatcher`:

```cpp
#include <reflex/matcher.h>
#include <reflex/boostmatcher.h>

using namespace reflex;

// some random text (with extra spacing)
std::string example("Monty\n Python's    Flying  Circus");

// construct a fixed pattern that is case insensitive
static Pattern pattern("monty|python", "i");

// construct a matcher to search the example text
Matcher matcher(pattern, example);
while (matcher.find() != 0)
  std::cout << matcher.text()
            << " at " << matcher.lineno() << "," << matcher.columno()
            << " spans " << matcher.first() << ".." << matcher.last()
            << std::endl;

// construct a Boost.Regex matcher to count the words (all non-spaces "\S+")
BoostMatcher boostmatcher("\\S+", example);
boostmatcher.buffer(); // because Boost.Regex partial_match is broken!
std::cout << std::distance(boostmatcher.find.begin(), boostmatcher.find.end())
          << " words"
          << std::endl;

// reuse the BoostMatcher with a C++11 range-based loop to split the example text on space "\s+":
for (auto& split : boostmatcher.pattern("\\s+").input(example).split)
  std::cout << split.text() << " ";
std::cout << std::endl;

// reuse the BoostMatcher to split on whitespace using an iterator, sort into a set, and print
boostmatcher.input(example).buffer();
std::set<std::string> words(boostmatcher.split.begin(), boostmatcher.split.end());
std::copy(words.begin(), words.end(), std::ostream_iterator<std::string>(std::cout, " "));
std::cout << std::endl;
```

When executed this code prints:

    Monty at 1,0 spans 0..5
    Python at 2,1 spans 7..13
    4 words
    Monty Python's Flying Circus
    Circus Flying Monty Python's


🔝 [Back to table of contents](#)

### Example 2

This example shows how a URL can be matched by using two patterns: one pattern
to extract the host:port/path parts and another pattern to extract the query
string key-value pairs in a loop.

```cpp
#include <reflex/boostmatcher.h>
#include <iostream>

using namespace reflex;

const char *URL = "...";

// match URL host:port/path using group captures for these
BoostMatcher re("https?://([^:/]*):?(\\d*)/?([^?#]*)", URL);

if (re.scan())
{
  // found a partial match at start, now check if we have a host
  if (re[1].first != NULL)
  {
    std::string host(re[1].first, re[1].second);
    std::cout << "host: " << host << std::endl;

    // check of we have a port
    if (re[2].first != NULL && re[2].second != 0)
    {
      std::string port(re[2].first, re[2].second);
      std::cout << "port: " << port << std::endl;
    }

    // check of we have a path
    if (re[3].first != NULL && re[3].second != 0)
    {
      std::string path(re[3].first, re[3].second);
      std::cout << "path: " << path << std::endl;
    }
  }

  // check if we have a query string
  if (re.input() == '?')
  {
    // now switch patterns to match the query string
    re.pattern("([^=&]*)=?([^&]*)&?");
    while (re.scan())
      std::cout <<
        "query key: " << std::string(re[1].first, re[1].second) <<
        ", value: " << std::string(re[2].first, re[2].second) << std::endl;
  }
  else if (!re.at_end())
  {
    // not a query string and not the end, we expect an # anchor
    std::cout << "anchor: " << re.rest() << std::endl;
  }
}
else
{
  std::cout << "Error, not a http/s URL: " << re.rest() << std::endl;
}
```

🔝 [Back to table of contents](#)

### Example 3

This example shows how input can be reassigned in each iteration of a loop that
matches wide strings against a word pattern `\w+`:

```cpp
#include <reflex/boostmatcher.h>

using namespace reflex;

// four words
const wchar_t *words[] = { L"Monty", L"Python's", L"Flying", L"Circus" };

// construct a Boost.Regex matcher for words, given empty input initially
BoostMatcher wordmatcher("\\w+", Input());

// check if each string in words[] is a word
for (int i = 0; i < 4; i++)
  if (wordmatcher.input(words[i]).matches())
    std::cout << wordmatcher.text() << ", ";
```

When executed this code prints:

    Monty, Flying, Circus, 

🔝 [Back to table of contents](#)

### Example 4

This example counts the number of words, lines, and chars from the `std::cin`
stream:

```cpp
#include <reflex/stdmatcher.h>

using namespace reflex;

// construct a std::regex matcher like the wc command (a word is a series of nonspaces)
StdMatcher word("\\S+", std::cin);

size_t words = std::distance(word.find.begin(), word.find.end());
size_t lines = word.lineno() - 1;
size_t chars = word.last();

std::cout << lines << " " << words << " " << chars << std::endl;
```

🔝 [Back to table of contents](#)

### Example 5

This example tokenizes a string by grouping the subpatterns in a regex and by
using the group index of the capture obtained with `accept()`:

```cpp
#include <reflex/matcher.h>

using namespace reflex;

static Pattern pattern =
  "(\\w*cat\\w*)|" // 1st group = token 1
  "(\\w*dog\\w*)|" // 2nd group = token 2
  "(\\w+)|"        // 3rd group = token 3
  "(.)";           // 4th group = token 4

Matcher tokenizer(pattern, "cats love hotdogs!");

for (auto& token : tokenizer.scan)
  std::cout << "Token = " << token.accept()
            << ": matched '" << token.text() << "'"
            << " with '" << token.pattern()[token.accept()] << "'\n";
assert(tokenizer.at_end());
```

When executed this code prints:

    Token = 1: matched 'cats' with '(\\w*cat\\w*)'
    Token = 4: matched ' ' with '(.)'
    Token = 3: matched 'love' with '(\\w+)'
    Token = 4: matched ' ' with '(.)'
    Token = 2: matched 'hotdogs' with '(\\w*dog\\w*)'
    Token = 4: matched '!' with '(.)'

🔝 [Back to table of contents](#)

### Example 6

This example reads a file with embedded credit card numbers to extract.  The
numbers are sorted into five sets for each type of major credit card:

```cpp
#include <reflex/matcher.h>

using namespace reflex;

static Pattern card_patterns =
  "(?# MasterCard)(5[1-5]\\d{14})|"                   // 1st group = MC
  "(?# Visa)(4\\d{12}(?:\\d{3})?)|"                   // 2nd group = VISA
  "(?# AMEX)(3[47]\\d{13})|"                          // 3rd group = AMEX
  "(?# Discover)(6011\\d{14})|"                       // 4th group = Discover
  "(?# Diners Club)((?:30[0-5]|36\\d|38\\d)\\d{11})"; // 5th group = Diners

static const char *card_data =
  "mark 5212345678901234\n"
  "vinny 4123456789012\n"
  "victor 4123456789012345\n"
  "amy 371234567890123\n"
  "dirk 601112345678901234\n"
  "doc 38812345678901 end\n";

std::set<std::string> cards[5];

for (auto& match : Matcher(card_patterns, card_file).find)
  cards[match.accept() - 1].insert(match.text());

for (int i = 0; i < 5; ++i)
  for (std::set<std::string>::const_iterator j = cards[i].begin(); j != cards[i].end(); ++j)
    std::cout << i << ": " << *j << std::endl;
```

When executed this code prints:

    0: 5212345678901234
    1: 4123456789012
    1: 4123456789012345
    2: 371234567890123
    3: 601112345678901234
    4: 38812345678901

🔝 [Back to table of contents](#)

### Example 7

The RE/flex matcher engine `reflex::Matcher` only recognizes group captures at
the top level of the regex (i.e. among the top-level alternations), because it
uses an efficient FSM for matching.

By contrast, the Boost.Regex matcher can capture groups within a regex, but the
matcher is slower.

```cpp
#include <reflex/boostmatcher.h>

// a BoostMatcher to find 'TODO' lines on stdin and capture their content to display
reflex::BoostMatcher matcher("TODO ([^\\n]+)", stdin);
matcher.buffer(); // because Boost.Regex partial_match is broken!
while (matcher.find())
  std::cout
    << matcher.lineno() << ": "
    << std::string(matcher[1].first, matcher[1].second)
    << std::endl;
```

🔝 [Back to table of contents](#)

### Example 8

This is a more advanced example, in which we will use the
`reflex::BoostMatcher` class to decompose URLs into parts: the host, port,
path, optional ?-query string key=value pairs, and an optional #-anchor.

To do so, we change the pattern of the matcher to partially match each of the
URL's parts and also use `input()` to check the input character:

```cpp
#include <reflex/boostmatcher.h>
#include <iostream>

using namespace reflex;

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "Usage: url 'URL'" << std::endl;
    exit(EXIT_SUCCESS);
  }

  BoostMatcher re("https?://([^:/]*):?(\\d*)/?([^?#]*)", argv[1]);

  if (re.scan())
  {
    // found a partial match at start, now check if we have a host
    if (re[1].first != NULL)
    {
      std::string host(re[1].first, re[1].second);
      std::cout << "host: " << host << std::endl;

      // check of we have a port
      if (re[2].first != NULL && re[2].second != 0)
      {
        std::string port(re[2].first, re[2].second);
        std::cout << "port: " << port << std::endl;
      }

      // check of we have a path
      if (re[3].first != NULL && re[3].second != 0)
      {
        std::string path(re[3].first, re[3].second);
        std::cout << "path: " << path << std::endl;
      }
    }

    // check if we have a query string
    if (re.input() == '?')
    {
      // now switch patterns to match the rest of the input
      // i.e. a query string or an anchor
#if 0
      // 1st method: a pattern to split query strings at '&'
      re.pattern("&");
      while (re.split())
        std::cout << "query: " << re << std::endl;
#else
      // 2nd method: a pattern to capture key-value pairs between the '&'
      re.pattern("([^=&]*)=?([^&]*)&?");
      while (re.scan())
        std::cout <<
          "query key: " << std::string(re[1].first, re[1].second) <<
          ", value: " << std::string(re[2].first, re[2].second) << std::endl;
#endif
    }
    else if (!re.at_end())
    {
      // not a query string and not the end, we expect an # anchor
      std::cout << "anchor: " << re.rest() << std::endl;
    }
  }
  else
  {
    std::cout << "Error, not a http/s URL: " << re.rest() << std::endl;
  }

  return EXIT_SUCCESS;
}
```

Note that there are two ways to splite the query string into key-value pairs
and each method is shown in the two `#if` branches.

When executing

    ./url 'https://localhost:8080/test/me?name=reflex&license=BSD-3'

this code prints:

    host: localhost
    port: 8080
    path: test/me
    query key: name, value: reflex
    query key: license, value: BSD-3

🔝 [Back to table of contents](#)

### Example 9

This example shows how a `FILE*` file descriptor is used as input.  The file
encoding is obtained from the UTF BOM, when present in the file.  Note that the
file's state is accessed through the matcher's member variable `in`:

```cpp
#include <reflex/boostmatcher.h>

using namespace reflex;

BoostMatcher matcher("\\s+", fopen("filename", "r"));

if (matcher.in.file() != NULL && matcher.in.good())
{
  switch (matcher.in.file_encoding())
  {
    case Input::file_encoding::plain:   std::cout << "plain ASCII/binary/UTF-8"; break;
    case Input::file_encoding::utf8:    std::cout << "UTF-8 with BOM";           break;
    case Input::file_encoding::utf16be: std::cout << "UTF-16 big endian";        break;
    case Input::file_encoding::utf16le: std::cout << "UTF-16 little endian";     break;
    case Input::file_encoding::utf32be: std::cout << "UTF-32 big endian";        break;
    case Input::file_encoding::utf32le: std::cout << "UTF-32 little endian";     break;
    case Input::file_encoding::latin:   std::cout << "ASCII+Latin-1/ISO-8859-1"; break;
    case Input::file_encoding::cp437:   std::cout << "CP 437";                   break;
    case Input::file_encoding::cp850:   std::cout << "CP 850";                   break;
    case Input::file_encoding::ebcdic:  std::cout << "EBCDIC";                   break;
    case Input::file_encoding::cp1250:  std::cout << "CP-1250";                  break;
    case Input::file_encoding::cp1251:  std::cout << "CP-1251";                  break;
    case Input::file_encoding::cp1252:  std::cout << "CP-1252";                  break;
    case Input::file_encoding::cp1253:  std::cout << "CP-1253";                  break;
    case Input::file_encoding::cp1254:  std::cout << "CP-1254";                  break;
    case Input::file_encoding::cp1255:  std::cout << "CP-1255";                  break;
    case Input::file_encoding::cp1256:  std::cout << "CP-1256";                  break;
    case Input::file_encoding::cp1257:  std::cout << "CP-1257";                  break;
    case Input::file_encoding::cp1258:  std::cout << "CP-1258";                  break;
  }
  std::cout << " of " << matcher.in.size() << " converted bytes to read\n";
  matcher.buffer(); // because Boost.Regex partial_match is broken!
  if (matcher.split() != 0)
    std::cout << "Starts with: " << matcher.text() << std::endl;
  std::cout << "Rest of the file is: " << matcher.rest();
  fclose(matcher.in.file());
}
```

The default encoding is `reflex::Input::file_encoding::plain` when no UTF BOM
is detected at the start of the input file.  The encodings
`reflex::Input::file_encoding::latin`, `reflex::Input::file_encoding::cp1252`,
`reflex::Input::file_encoding::cp437`, `reflex::Input::file_encoding::cp850`,
`reflex::Input::file_encoding::ebcdic` are never detected automatically,
because plain encoding is implicitly assumed to be the default encoding.  To
convert these files, set the file encoding format explicitly in your code.  For
example, if you expect the source file to contain ISO-8859-1 8-bit characters
(ASCII and the latin-1 supplement) then set the default file encoding to
`reflex::Input::file_encoding::latin` as follows:

```cpp
reflex::Input input(fopen("filename", "r"), reflex::Input::file_encoding::latin);
if (input.file() != NULL)
{
  reflex::BoostMatcher matcher("\\s+", input);
  matcher.buffer(); // because Boost.Regex partial_match is broken!
  if (matcher.split() != 0)
    std::cout << "Starts with: " << matcher.text() << std::endl;
  std::cout << "Rest of the file is: " << matcher.rest();
  fclose(input.file());
}
```

This sets the file encoding to ISO-8859-1, but only if no UTF BOM was detected
in the file.  Files with a UTF BOM are always decoded as UTF, which cannot be
overruled.

🔝 [Back to table of contents](#)


Tips, tricks and gotchas                                              {#tricks}
========================

Invalid UTF encodings                                            {#invalid-utf}
---------------------

It may be tempting to write a pattern with `.` (dot) as a wildcard in a lexer
specification, but beware that in Unicode mode with option `−−unicode` (global
`%%option unicode` or local `(?u:φ)`) the dot matches any code point, including
code points outside of the valid Unicode character range and invalid overlong
UTF-8.  The reason for this design decision is that a lexer should support a
"catch all else" rule to report errors in the input:

<div class="alt">
```cpp
.    std::cerr << "lexical error, full stop!" << std::endl; exit(EXIT_FAILURE);
```
</div>

If dot in Unicode mode would be restricted to match valid Unicode only, then
the action above will never be triggered when invalid input is encountered.
Because all non-dot regex patterns are valid Unicode in RE/flex, it would be
impossible to write a "catch all else" rule that catches input format errors!

The dot in Unicode mode is self-synchronizing and consumes text up to to the
next ASCII or Unicode character.

To reject invalid UTF-8 input in regex patterns, make sure to avoid `.` (dot)
and use `\p{Unicode}` or `\X` instead, and reserve dot to catch anything,
including invalid UTF-8 and UTF-16 encodings.

Invalid UTF-16 is detected automatically by the `reflex::Input` class and
replaced with the `REFLEX_NONCHAR` code point U+200000 that lies outside the
valid Unicode range.  This code point is never matched by non-dot regex
patterns and is easy to detect by a regex pattern with a dot.

Note that character classes written as bracket lists may produce invalid
Unicode ranges when used improperly.  This is not a problem for matching, but
may prevent rejecting surrogate halves that are invalid Unicode.  For example,
`[\u{00}-\u{10FFFF}]` obviously includes the invalid range of surrogate halves
`[\u{D800}-\u{DFFF}]`.  You can always remove surrogate halves from any
character class by intersecting the class with `[\p{Unicode}]`, that is
`[...&&[\p{Unicode}]]`.  Furthermore, character class negation with `^` results
in classes that are within range U+0000 to U+10FFFF and excludes surrogate
halves.

🔝 [Back to contents](#)


Scanning ISO-8859-1 (latin-1) files with a Unicode scanner        {#iso-8859-1}
----------------------------------------------------------

Scanning files encoded in ISO-8859-1 by a Unicode scanner that expects UTF-8
will cause the scanner to misbehave or throw errors.

Many text files are still encoded in ISO-8859-1 (also called latin-1).  To set
up your scanner to safely scan ISO-8859-1 content when your scanner rules use
Unicode (with the `−−unicode` option and your patterns that use UTF-8
encodings), set the default file encoding to `latin`:

```cpp
reflex::Input input(stdin, reflex::Input::file_encoding::latin);
Lexer lexer(input);
lexer.lex();
```

This scans files from standard input that are encoded in ISO-8859-1, unless the
file has a [UTF Byte Order Mark (BOM)](www.unicode.org/faq/utf_bom.html).  When
a BOM is detected the scanner switches to UTF scanning.

See \ref regex-input-file to set file encodings.

🔝 [Back to contents](#)


Lazy repetitions                                                        {#lazy}
----------------

Repetitions (`*`, `+`, and `{n,m}`) and the optional pattern (`?`) are greedy,
unless marked with an extra `?` to make them lazy.  Lazy repetitions are
useless when the regex pattern after the lazy repetitions permits empty input.
For example, `.*?a?` only matches one `a` or nothing at all, because `a?`
permits an empty match.

🔝 [Back to table of contents](#)


Lazy optional patterns and trigraphs                               {#trigraphs}
------------------------------------

This C/C++ trigraph problem work-around does not apply to lexer specifications
that the **reflex** command converts while preventing trigraphs.

Trigraphs in C/C++ strings are special tripple-character sequences, beginning
with two question marks and followed by a character that is translated.  For
example, `"x??(y|z)"` is translated to `"x[y|z)"`.

Fortunately, most C++ compilers ignore trigraphs unless in standard-conforming
modes, such as `-ansi` and `-std=c++98`.

When using the lazy optional pattern `φ??` in a regex C/C++ string for pattern
matching with one of the RE/flex matchers for example, use `φ?\?` instead,
which the C/C++ compiler translates to `φ??`.

Otherwise, lazy optional pattern constructs will appear broken.

🔝 [Back to table of contents](#)


Repeately switching to the same input                              {#switching}
-------------------------------------

The state of the input object `reflex::Input` changes as the scanner's matcher
consumes more input.  If you switch to the same input again (e.g. with `in(i)`
or `switch_stream(i)` for input source `i`), a portion of that input may end up
being discarded as part of the matcher's internal buffer is flushed when input
is assigned.  Therefore, the following code will not work because stdin is
flushed repeately:

```cpp
Lexer lexer(stdin);       // a lexer that reads stdin
lexer.in(stdin);          // this is OK, nothing read yet
while (lexer.lex(stdin))  // oops, assigning stdin again and again
  std::cout << "we're not getting anywhere?" << std::endl;
```

If you need to read a file or stream again, you must rewind it to the location
in the file to start reading.  Beware that `FILE*` input is checked against an
UTF BOM at the start of a file, which means that you cannot reliably move to a
location in the file to start reading when files are encoded in UTF-8 or
UTF-16.

🔝 [Back to table of contents](#)


Where is FlexLexer.h?                                              {#flexlexer}
---------------------

RE/flex uses its own header file `reflex/flexlexer.h` for compatibility with
Flex, instead of `FlexLexer.h`.  The latter is specific to Flex and cannot be
used with RE/flex.  You should not have to include `FlexLexer.h` but if you do,
use:

```cpp
#include <reflex/flexlexer.h>
```

The `FlexLexer` class defined in `reflex/flexlexer.h` is the base class of the
generated `yyFlexLexer` class.

🔝 [Back to table of contents](#)


Interactive input with GNU readline                                 {#readline}
-----------------------------------

Option `-I` for interactive input generates a scanner that uses `fgetc()` to
read input from a `FILE*` descriptor (stdin by default).  Interactive input is
made more user-friendly with the GNU readline library that provides basic line
editing and a history mechanism.

To use `readline()` in your lexer, call `readline()` in your Lexer's
constructor and in the `wrap()` method as follows:

<div class="alt">
```cpp
%top{
  #include <stdlib.h>
  #include <stdio.h>
  #include <readline/readline.h>
  #include <readline/history.h>
%}

%class{
  const char *prompt;
  // we use wrap() to read the next line
  virtual int wrap() {
    if (line)
    {
      free((void*)line);
      line = readline(prompt);
      if (line != NULL)
      {
        if (*line)
          add_history(line);
        linen.assign(line).push_back('\n');
        in(linen);
      }
    }
    // wrap() == 0 means OK: wrapped after EOF
    return line == NULL ? 1 : 0;
  }
  // the line returned by readline() without \n
  char *line;
  // the line with \n appended
  std::string linen;
%}

%init{
  prompt = NULL;
  line = readline(prompt);
  if (line != NULL)
  {
    if (*line)
      add_history(line);
    linen.assign(line).push_back('\n');
  }
  in(linen);
%}
```
</div>

With option `−−flex` you will need to replace `wrap()` by `yywrap()`.

The rules can be matched as usual, where `\n` matches the end of a line, for
example:

<div class="alt">
```cpp
%%

.+  echo(); // ECHO the entire line
\n  echo(); // ECHO end of the line

%%
```
</div>

🔝 [Back to table of contents](#)


Getting RE/flex                                                     {#download}
===============

Download RE/flex from [SourceForge](https://sourceforge.net/projects/re-flex)
or visit the RE/flex [GitHub repository](https://github.com/Genivia/RE-flex).

🔝 [Back to table of contents](#)


License and copyright                                                {#license}
=====================

RE/flex software is released under the BSD-3 license.  All parts of the
software have reasonable copyright terms permitting free redistribution.  This
includes the ability to reuse all or parts of the RE/flex source tree.

> Copyright (c) 2016, Robert van Engelen, Genivia Inc. All rights reserved.   
>                                                                             
> Redistribution and use in source and binary forms, with or without          
> modification, are permitted provided that the following conditions are met: 
>                                                                             
>   (1) Redistributions of source code must retain the above copyright notice,
>       this list of conditions and the following disclaimer.                 
>                                                                             
>   (2) Redistributions in binary form must reproduce the above copyright     
>       notice, this list of conditions and the following disclaimer in the   
>       documentation and/or other materials provided with the distribution.  
>                                                                             
>   (3) The name of the author may not be used to endorse or promote products 
>       derived from this software without specific prior written permission. 
>                                                                             
> THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
> WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        
> MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO  
> EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      
> SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
> PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
> OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,    
> WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR     
> OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF      
> ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                  

The Free Software Foundation maintains a
[BSD-3 License Wiki](http://directory.fsf.org/wiki/License:BSD_3Clause).

🔝 [Back to table of contents](#)

Copyright (c) 2017, Robert van Engelen, Genivia Inc. All rights reserved.
