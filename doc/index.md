
RE/flex User Guide                                                  {#mainpage}
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
- optional "free space mode" improves readability of lex specifications;
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
  efficient deterministic finite state machines (FSMs) that are contructed from
  regex patterns with POSIX mode matching extended to support lazy quantifiers,
  word boundary anchors, Unicode UTF-8, and much more;

- a *flexible regex framework* that combines the above with a collection of C++
  class templates that are easy to use and that offer a rich API for searching,
  matching, scanning, and splitting of input strings, files and streaming data.
  This framework is flexible and can be extended to include other regex pattern
  matchers that can operate seamlessly with the RE/flex scanner generator.

⇢ [Back to contents](#)


Yet another scanner generator                                         {#intro1}
-----------------------------

Lex, Flex and variants are powerful *scanner generators* that generate scanners
(a.k.a. *lexical analyzers* and *lexers*) from *lex specifications*.  These lex
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
lex specification:

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

Consider the lex specification at the start of this section.  Suppose the input
to tokenize is `iflag = 1`.  In POSIX mode we return `ASCII_IDENTIFIER` for the
name `iflag`, `OP_ASSIGN` for `=`, and `NUMBER` for `1`.  In Perl mode, we find
that `iflag` matches `if` and the rest of the name is not consumed, which gives
`KEYWORD_IF` for `if`, `ASCII_IDENTIFIER` for `lag`, `OP_ASSIGN` for `=`, and a
`NUMBER` for `1`.  Perl mode matching greedely returns leftmost matches.

Using Perl mode in a scanner requires all overlapping patterns to be defined in
a lex specification with longer patterns defined first to avoid partial matches
of tokens as demonstrated.  By contrast, POSIX mode is *declarative* and allows
you to define the patterns in the specification in any order.  Perhaps the only
ordering constraint on patterns is for patterns that match the same input, such
such as matching the keyword `if` in the example:  `KEYWORD_IF` must be matched
before `ASCII_IDENTIFIER`.

For this reason, RE/flex scanners use a regex library in POSIX mode by default.

In summary, the advantages that RE/flex has to offer include:

- RE/flex is fully compatible with Flex and Bison, by using the `−−flex` and/or
  `−−bison` options.  This eliminates a learning curve to use RE/flex.

- The RE/flex scanner generator takes a lex specification that is compatible to
  [Flex](dinosaur.compilertools.net/#flex), with additional options to select a
  matcher engine and to specify names and options for C++ class generation.
  
- The RE/flex scanner generator option `−−bison` generates a scanner compatible
  with [Bison](dinosaur.compilertools.net/#bison).  RE/flex also supports Bison
  bridge (pure reentrant and MT-safe) parsers.

- The regular expression syntax of patterns in lex specifications is restricted
  to POSIX ERE.  By contrast, the RE/flex scanner generator is regex-centric by
  design and offers a rich pattern syntax including lazy quantifiers.

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

⇢ [Back to contents](#)


A regex library for pattern matching in C++                           {#intro2}
-------------------------------------------

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
  `matches()` | true if the input from begin to end matches the regex pattern
  `find()`    | search input and return true if a match was found
  `scan()`    | return true if input at current position matches partially
  `split()`   | split input at the next match

These methods are repeatable, where the last three return additional matches.

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
while (matcher.find() == true)
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
while (matcher.split() == true)
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
of time as we will see later.

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
while (matcher.find() == true)
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
while (matcher.find() == true)
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
  `accept()`  | returns group capture index or zero if not captured/matched
  `text()`    | returns `\0`-terminated `const char*` string match
  `wtext()`   | returns `std::wstring` wide string match (converted from UTF-8)
  `pair()`    | returns `std::pair<size_t,std::string>(accept(), text())`
  `wpair()`   | returns `std::pair<size_t,std::wstring>(accept(), wtext())`
  `size()`    | returns the length of the match in bytes
  `wsize()`   | returns the length of the match in number of wide characters
  `rest()`    | returns `\0`-terminated `const char*` of the rest of the input
  `more()`    | tells the matcher to append the next match (adjacent matches)
  `less(n)`   | cuts `text()` to `n` bytes and repositions the matcher
  `lineno()`  | returns line number of the match, starting with line 1
  `columno()` | returns column number of the match, starting with 0
  `first()`   | returns position of the first character of the match
  `last()`    | returns position of the last + 1 character of the match
  `at_bol()`  | true if matcher reached the begin of a new line `\n`
  `at_bob()`  | true if matcher is at the start of input, no matches consumed
  `at_end()`  | true if matcher is at the end of input
  `[0]`       | operator returns `std::pair<const char*,size_t>(text(),size())`
  `[n]`       | operator returns n'th capture `std::pair<const char*,size_t>`

The first five methods in the table return values that can also be obtained via
casting the matcher object to the return types of these methods.

The `operator[]` takes a group capture number `n` and returns the group capture
match in a pair with a `const char*` pointer to the group-matching text and the
size of the matched text in bytes.  Because the pointer points to a string that
is not `\0`-terminated, you should use the size to determine the matching part.
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
the `text()` string value we recommend to instantiate a `std::string`.

@warning The `operator[]` method returns a pair with the match info of the n'th
group, which is a string `const char*` (or NULL) and size in bytes.  The string
*should not be used* after matching continues.

@note When using the `reflex::Matcher` class, the `accept()` method returns the
accepted pattern among the alternations in the regex that are specified only at
the top level in the regex.  For example, the regex `"(a(b)c)|([A-Z])"` has two
groups, because only the outer top-level groups are recognized.  Because groups
are specified at the top level only, the grouping parenthesis are optional.  We
can simplify the regex to `"a(b)c|[A-Z]"` and still capture the two patterns.

Three special methods can be used to manipulate the input stream directly:

  Method     | Result
  ---------- | ----------------------------------------------------------------
  `input()`  | returns next char 0..255 from the input, matcher then skips it
  `winput()` | returns the next wide character from the input, matcher skips it
  `unput(c)` | put char `c` back unto the stream, matcher then takes it
  `peek()`   | returns the next char 0..255 from the input without consuming it

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

while (matcher.find() == true)
  std::cout << "Found " << matcher.text() << std::endl;

// use the same matcher with a new source (an Input object):
std::ifstream ifs("cows.txt", std::ifstream::in);
source = ifs;           // Input source is reassignable
matcher.input(source);  // can use ifs as parameter also

while (matcher.find() == true)
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
while (matcher.find() == true)
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
while (matcher.find() == true)
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

⇢ [Back to contents](#)


The RE/flex scanner generator                                         {#reflex}
=============================

The RE/flex scanner generator tool takes a lex specification and generates a
regex-based C++ lexer class that is saved in lex.yy.cpp, or saved to the file
specified by the `-o` command-line option.  This file is then compiled and
linked with a regex-library to produce a scanner.  A scanner can be a
stand-alone application or part of a larger program such as a compiler:

@dot
digraph build {
  ranksep=.25;
  node     [ shape=box, fontname=Helvetica, fontsize=11 ];
  spec     [ label="lex specification\n(.l, .lex)", peripheries=0 ];
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

⇢ [Back to contents](#)


The reflex command line tool                                  {#reflex-command}
----------------------------

The **reflex** command generates a C++ scanner class in a source code file
given a lex specification.  The **reflex** command accepts `−−flex` and
`−−bison` options for compatibility.  These options allow **reflex** to be used
as a replacement of the classic Flex and Lex tools:

    $ reflex −−flex −−bison lexspec.l

The first option `−−flex` specifies that `lexspec.l` is a classic Lex
specification with `yytext` or `YYText()` and the usual "yy" variables and
functions.

The second option `−−bison` generates a scanner class and auxiliary code for
compatibility with [Bison](dinosaur.compilertools.net/#bison) parsers.  See
\ref reflex-bison for more details.

The source code output is intentionally structured in sections that are clean,
readable, and reusable.

⇢ [Back to contents](#)


Lex specifications                                               {#reflex-spec}
------------------

A lex specification consists of three sections that are divided by `%%`
delimiters:

<div class="alt">
```cpp
Definitions
%%
Rules
%%
User code
```
</div>

The Definitions section is used to define named regex patterns, to set options
for the scanner, and for including C++ declarations.

The Rules section is the main workhorse of the scanner and consists of patterns
and actions, where patterns may use named regex patterns that are defined in
the definitions section.  The actions are executed when patterns match.  For
example, the following lex specification replaces all occurrences of `cow` by
`chick`:

<div class="alt">
```cpp
%%
cow      out() << "chick";
%%
```
</div>

The default rule is to echo any input character that is read from input that
does not match a rule in the rules section, so all other text is faithfully
reproduced.

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

Note that we defined a named pattern `cow` in the definitions section to match
the start and end of (capitalized) "cow" word with the regex `\<[Cc]ow\>`.  We
use `{cow}` in our rule for matching.  The matched text first character is
emitted with `text()[0]` and we simply add "hick" to complete our chick.

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

To create a stand-alone scanner, we add `main` to the user code section:

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
will not return until the entire input is processed.

### The definitions section

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

Names must be defined before being referenced.  Names are expanded as macros in
regex patterns.  For example, `{digit}+` is expanded into `[0-9]+`.

@note This is not entirely accurate: when a name is expanded to its right-hand
side `X`, `X` is placed in a non-capturing group `(?:X)` to preserve its
structure.  For example, `{number}` expands to `(?:{digit}+)` which in turn
expands to `(?:(?:[0-9])+)`.

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

The Definition section may also contain one or more options with `%%option` (or
`%%o` for short).  For example:

<div class="alt">
```cpp
%option dotall main
%o matcher=boost
```
</div>

Multiple options can be grouped on the same line as is shown above.  See
\ref reflex-options for a list of available options.

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

To inject Lexer class member declarations, place the declarations within
`%%class{` and `%}`.  The `%%class{` and `%}` should be placed at the start of
a new line.

Likewise, to inject Lexer class constructor code, for example to initialize
members, place the code within `%%init{` and `%}`.  The `%%init{` and `%}`
should be placed at the start of a new line.

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

To modularize lex specifications use `%%include` (or `%%i` for short) to
include files into the definitions section of a lex specification.  For
example:

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

### The rules section

Each rule in the Rules section consists of a pattern-action pair.  For example,
the following defines an action for a pattern:

<div class="alt">
```cpp
{number}    out() << "number " << text() << std::endl;
```
</div>

To add action code that spans multiple lines, indent the code or place the code
within a `{` and `}` code block.  When local variables are declared in an
action then the code should always be placed in a code block.

Actions in the Rules section can use predefined RE/flex variables and
functions.  With **reflex** option `−−flex`, the variables and functions are
the classic Flex actions shown in the second column of this table:

  RE/flex action        | Flex action          | Result
  --------------------- | -------------------- | ------------------------------
  `text()`              | `YYText()`, `yytext` | `\0`-terminated text match
  `size()`              | `YYLeng()`, `yyleng` | size of the match in bytes
  `wsize()`             | *n/a*                | number of wide chars matched
  `lineno()`            | `yylineno`           | line number of match (>=1)
  `columno()`           | *n/a*                | column number of match (>=0)
  `echo()`              | `ECHO`               | `out().write(text(), size())`
  `start()`             | `YY_START`           | get current start condition
  `start(n)`            | `BEGIN n`            | set start condition to `n`
  `push_state(n)`       | `yy_push_state(n)`   | push current state, start `n`
  `pop_state()`         | `yy_pop_state()`     | pop state and make it current
  `top_state()`         | `yy_top_state()`     | get top state start condition
  `in(i)`               | `yyin = &i`          | set input to `reflex::Input i`
  `in()`                | `*yyin`              | get `reflex::Input` object
  `out(s)`              | `yyout = &s`         | set output to `std::ostream s`
  `out()`               | `*yyout`             | get `std::ostream` object
  `out().write(s, n)`   | `LexerOutput(s, n)`  | output chars `s[0..n-1]`
  `out().put(c)`        | `output(c)`          | output char `c`
  `matcher().accept()`  | `yy_act`             | number of the matched rule
  `matcher().text()`    | `YYText()`, `yytext` | same as `text()`
  `matcher().wtext()`   | *n/a*                | `std::wstring` match
  `matcher().size()`    | `YYLeng()`, `yyleng` | same as `size()`
  `matcher().wsize()`   | *n/a*                | size of wide string match
  `matcher().input()`   | `yyinput()`          | get next char from input
  `matcher().winput()`  | *n/a*                | get wide character from input
  `matcher().unput(c)`  | `unput(c)`           | put back char `c`
  `matcher().peek()`    | *n/a*                | peek at next char on input
  `matcher().more()`    | `yymore()`           | concat next match to the match
  `matcher().less(n)`   | `yyless(n)`          | shrink match's length to `n`
  `matcher().first()`   | *n/a*                | first pos of match in input
  `matcher().last()`    | *n/a*                | last pos+1 of match in input
  `matcher().rest()`    | *n/a*                | get rest of input until end
  `matcher().at_bob()`  | *n/a*                | true if at the begin of input
  `matcher().at_end()`  | *n/a*                | true if at the end of input
  `matcher().at_bol()`  | `YY_AT_BOL()`        | true if at begin of a newline
  `set_debug(n)`        | `set_debug(n)`       | reflex option `-d` sets `n=1`
  `debug()`             | `debug()`            | nonzero when debugging

Note that Flex `switch_streams(i, o)` is the same as invoking the `in(i)` and
`out(o)` methods.  Flex `yyrestart(i)` is the same as setting the input
`yyin=&i` or invoking `in(i)` to set input to a file, stream, or string.
However, when the end of input (EOF) was reached, you should clear the EOF
state with `matcher().set_end(false)` or reset the matcher state with
`matcher().reset()`.  A reset clears the line and column counters, resets the
internal anchor and boundary flags for anchor and word boundary matching, and
resets to standard buffered input mode.

@warning Do not invoke `matcher()` before `lex()` (or `yylex()` with option
`−−flex`) is invoked!  A matcher is not initially assigned to the lexer when it
is constructed.

Use **reflex** options `−−flex` and `−−bison` to enable global Flex actions and
variables.  This makes Flex actions and variables globally accessible outside
of the rules section, with the exception of `yy_push_state`, `yy_pop_state`,
`yy_top_state`.  Outside the rules section you must use the global action
`yyinput` instead of `input`, global action `yyunput` instead of `unput`, and
global action `yyoutput` instead of `output`.  Because `yyin` and `yyout` are
macros they cannot be (re)declared or accessed as global variables, but they
can be used as if these were variables.  To avoid compilation errors, use
**reflex** option `−−header-file` to generate a header file `lex.yy.h` to
include in your code to use the global use Flex actions and variables.
See \ref reflex-bison for more details on the `−−bison` options to use.

From the first entries in the table shown above you may have guessed correctly
that `text()` is just a shorthand for `matcher().text()`, since `matcher()` is
the matcher object associated with the generated Lexer class.  The same
shorthands apply to `size()`, `wsize()`, `lineno()` and `columno()`.

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

The Flex `yy_scan_string(string)` and `yy_scan_bytes(bytes, len)` functions are
also supported with **reflex** option `−−flex` and are simply creating setting
the matcher's `in(i)` input to a string.

The generated scanner reads from the standard input by default or from an input
source specified as a `reflex::Input` object, such as a string, wide string,
file, or a stream.

See \ref reflex-input for more details on managing the input to a scanner.

### The user code section

To inject code at the end of the generated scanner, such as a `main`
function, we can use the User Code section.  All of the code in the User Code
section is copied to the generated scanner.

Below is a User Code section example with `main` that invokes the lexer to read
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
  fclose(fd);
  return 0;
}
```
</div>

The above uses a `FILE` descriptor to read input from, which has the advantage
of automatically decoding UTF-8/16/32 input.  Other permissible input sources
are `std::istream`, `std::string`, `std::wstring`, `char*`, and `wchar_t*`.

⇢ [Back to contents](#)


Patterns                                                      {#reflex-patterns}
--------

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
  `\0177`   | matches an 8-bit character with octal value `177` (see below)
  `\x7f`    | matches an 8-bit character with hexadecimal value `7f`
  `\x{7f}`  | matches an 8-bit character with hexadecimal value `7f`
  `\p{C}`   | matches a character in class C (see below)
  `\Q..\E`  | matches the quoted content between `\Q` and `\E` literally
  `[abc]`   | matches one of `a`, `b`, or `c` (character class in bracket list)
  `[0-9]`   | matches a digit `0` to `9` (character class range)
  `[^0-9]`  | matches anything but a digit (negative list)
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
  `φψ`      | matches `φ` followed by `ψ` (concatenation)
  `φ⎮ψ`     | matches `φ` or matches `ψ` (alternation)
  `(φ)`     | matches `φ` as a group (capture)
  `(?:φ)`   | matches `φ` without group capture
  `(?=φ)`   | matches `φ` without consuming it (lookahead)
  `(?^φ)`   | matches `φ` and ignore it to continue matching (ignore, RE/flex only)
  `^φ`      | matches `φ` at the start of input or start of a line (multi-line mode)
  `φ$`      | matches `φ` at the end of input or end of a line (multi-line mode)
  `\Aφ`     | matches `φ` at the start of input
  `φ\z`     | matches `φ` at the end of input
  `\bφ`     | matches `φ` starting at a word boundary
  `φ\b`     | matches `φ` ending at a word boundary
  `\Bφ`     | matches `φ` starting at a non-word boundary
  `φ\B`     | matches `φ` ending at a non-word boundary
  `\<φ`     | matches `φ` that starts as a word
  `\>φ`     | matches `φ` that starts as a non-word
  `φ\<`     | matches `φ` that ends as a non-word
  `φ\>`     | matches `φ` that ends as a word
  `(?i:φ)`  | *case insensitive mode*: matches `φ` ignoring case
  `(?m:φ)`  | *multi-line mode*: `^` and `$` in `φ` match begin and end of a line
  `(?s:φ)`  | *dotall mode*: `.` (dot) in `φ` matches newline
  `(?x:φ)`  | *free space mode*: ignore all whitespace and comments in `φ`
  `(?#:..)` | *commenting*: all of `..` is skipped as a comment

Character classes in bracket lists are sets and these sets can be inverted,
subtracted, intersected, or merged as follows:

  Pattern           | Matches
  ----------------- | ---------------------------------------------------------
  `[^a-zA-Z]`       | matches a non-letter (character class inversion)
  `[a-z−−[aeiou]]`  | matches a consonant (character class subtraction)
  `[a-z&&[^aeiou]]` | matches a consonant (character class intersection)
  `[a-z⎮⎮[A-Z]]`    | matches a letter (character class union)

The character sets can be Unicode character sets.  Use **reflex** option
`−−unicode` or regex matcher converter flag `reflex::convert_flag::unicode`.

In addition, the following patterns can be used in lex specifications for
the **reflex** scanner generator:

  Pattern           | Matches
  ----------------- | ---------------------------------------------------------
  `x` (UTF-8)       | matches wide character `x` encoded in UTF-8, requires the `−−unicode` option
  `\u{2318}`        | matches Unicode character U+2318, requires the `−−unicode` option
  `\p{C}`           | matches a character in class C, ASCII or Unicode with the `−−unicode` option
  `\177`            | matches an 8-bit character with octal value `177`
  `".."`            | matches the quoted content literally
  `φ/ψ`             | matches `φ` if followed by `ψ` (trailing context, same as lookahead)
  `[a-z]{-}[aeiou]` | matches a consonant, same as `[a-z−−[aeiou]]`
  `[a-z]{+}[A-Z]`   | matches a letter, same as `[a-z⎮⎮[A-Z]]`
  `<S>φ`            | matches `φ` only if state `S` is enabled
  `<S1,S2,S3>φ`     | matches `φ` only if state `S1`, `S2`, or state `S3` is enabled
  `<*>φ`            | matches `φ` in any state
  `<<EOF>>`         | matches EOF in any state
  `<S><<EOF>>`      | matches EOF only if state `S` is enabled

The order of precedence for composing larger patterns from sub-patterns is as
follows, from high to low precedence:

1. Characters, character classes, bracket expressions, escapes, quotation
2. Grouping `(φ)`, `(?:φ)`, `(?=φ)`, and inline modifiers `(?imsx:φ)`
3. Quantifiers `?`, `*`, `+`, `{n,m}`
4. Concatenation (including lookahead)
5. Anchoring `^`, `$`, `\<`, `\>`, `\b`, `\B`, `\A`, `\z` 
6. Alternation `|`
7. Global modifiers `(?imsx)φ`

Note that the characters `.` (dot), `\`, `?`, `*`, `+`, `|`, `(`, `)`, `[`,
`]`, `{`, `}`, `^`, and `$` are meta-characters and should be escaped to match.
Lex specifications also include the `"` and `/` as meta-characters and these
should be escaped to match.

Bracket lists cannot be empty, so `[]` and `[^]` are invalid.  In fact, the
first character after the bracket is always part of the list.  So `[][]` is a
list that matches a `]` and a `[`, `[^][]` is a list that matches anything but
`]` and `[`, and `[-^]` is a list that matches a `-` and a `^`.

Multi-line mode is the default mode in lex specifications.  Free space mode can
be useful to improve readability.  To enable free space mode in **reflex** use
the `−−freespace` option.  Be warned that this requires all actions in lex
specifications to be placed within `{` and `}` blocks and other code to be
placed in `%{` and `%}` blocks.

Patterns ending in an escape `\` in lex specifications continue on the next
line.  This permits layout out long patterns, for example in free space mode.

@note We need the "regex escapes to be escaped" in C++ literal strings, so we
need an additional backslash for every backslash in a regex.

@note The lazy quantifier `?` for optional patterns `φ??` and repetitions `φ*?`
`φ+?` is not supported by Boost.Regex in POSIX mode.  In general, POSIX
matchers do not support lazy quantifiers due to POSIX limitations that are
rooted in the theory of formal languages.

@warning Trigraphs in C++ strings are special three character sequences,
beginning with two question marks and followed by one other character.  Avoid
`??` at all cost in C++ strings.  Instead, use at least one escaped question
mark, such as `?\?`, which the compiler will translate to `??`.  Otherwise,
lazy optional pattern constructs will appear broken.  Fortunately, most C++
compilers will warn about trigraph translation before causing trouble.

### Character categories

The 7-bit ASCII character categories are:

  Category     | List form      | Matches
  ------------ | -------------- | ---------------------------------------------
  `\p{Space}`  | `[[:space:]]`  | matches a white space character `[ \t\n\v\f\r]` same as `\s`
  `\p{Xdigit}` | `[[:xdigit:]]` | matches a hex digit `[0-9A-Fa-f]`
  `\p{Cntrl}`  | `[[:cntrl:]]`  | matches a control character `[\x00-\0x1f\x7f]`
  `\p{Print}`  | `[[:print:]]`  | matches a printable character `[\x20-\x7e]`
  `\p{Alnum}`  | `[[:alnum:]]`  | matches a alphanumeric character `[0-9A-Za-z]`
  `\p{Alpha}`  | `[[:alpha:]]`  | matches a letter `[A-Za-z]`
  `\p{Blank}`  | `[[:blank:]]`  | matches a blank `[ \t]` same as `\h`
  `\p{Digit}`  | `[[:digit:]]`  | matches a digit `[0-9]` same as `\d`
  `\p{Graph}`  | `[[:graph:]]`  | matches a visible character `[\x21-\x7e]`
  `\p{Lower}`  | `[[:lower:]]`  | matches a lower case letter `[a-z]` same as `\l`
  `\p{Punct}`  | `[[:punct:]]`  | matches a punctuation character `[\x21-\x2f\x3a-\x40\x5b-\x60\x7b-\x7e]`
  `\p{Upper}`  | `[[:upper:]]`  | matches an upper case letter `[A-Z]` same as `\u`
  `\p{Word}`   | `[[:word:]]`   | matches a word character `[0-9A-Za-z_]` same as `\w`
  `\d`         | `[[:digit:]]`  | matches a digit `[0-9]`
  `\D`         | `[^[:digit:]]` | matches a non-digit `[^0-9]`
  `\h`         | `[[:blank:]]`  | matches a blank character `[ \t]`
  `\H`         | `[^[:blank:]]` | matches a non-blank character `[^ \t]`
  `\s`         | `[[:space:]]`  | matches a white space character `[ \t\n\v\f\r]`
  `\S`         | `[^[:space:]]` | matches a non-white space `[^ \t\n\v\f\r]`
  `\l`         | `[[:lower:]]`  | matches a lower case letter `[a-z]`
  `\L`         | `[^[:lower:]]` | matches a non-lower case letter `[^a-z]`
  `\u`         | `[[:upper:]]`  | matches an upper case letter `[A-Z]`
  `\U`         | `[^[:upper:]]` | matches a nonupper case letter `[^A-Z]`
  `\w`         | `[[:word:]]`   | matches a word character `[0-9A-Za-z_]`
  `\W`         | `[^[:word:]]`  | matches a non-word character `[^0-9A-Za-z_]`

The following Unicode character categories are enabled with the **reflex**
`−−unicode` option and with the regex matcher converter flag
`reflex::convert_flag::unicode` when using a regex library:

  Category                               | Matches
  -------------------------------------- | ------------------------------------
  `.`                                    | matches any Unicode character
  `\X`                                   | matches any Unicode character with or without the `−−unicode` option
  `\s`, `\p{Zs}`                         | matches a Unicode white space character
  `\l`, `\p{Ll}`                         | matches a lower case letter with Unicode sub-property Ll
  `\u`, `\p{Lu}`                         | matches an upper case letter with Unicode sub-property Lu
  `\w`, `\p{Word}`                       | matches a Unicode word character with property L, Nd, or Pc
  `\p{Unicode}`                          | matches any character (Unicode U+0000 to U+10FFFF)
  `\p{ASCII}`                            | matches an ASCII character U+0000 to U+007F)
  `\p{Non_ASCII_Unicode}`                | matches a non-ASCII Unicode character U+0080 to U+10FFFF)
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

You can also use the `\P{C}` form that is identical to `\p{^C}`, which is the
inverted character class `C`.

### Indent and dedent matching

Automatic indent and dedent matching is a new feature in RE/flex and is enabled
when the RE/flex matcher engine is used (not the Boost.Regex matcher).

  Pattern | Matches
  ------- | -------------------------------------------------------------------
  `\i`    | indent: matches the next indent position
  `\j`    | dedent: matches the previous indent position

These patterns should be used in combination with the start of a line anchor
`^` followed by a pattern that represents the spacing for indentations.  The
pattern may include any characters that are considered part of indentation
margins, but should exclude `\n`.  For example:

<div class="alt">
```cpp
%o tabs=8
%%
^\h+      out() << "| "; // nodent: text is aligned to current indent margin
^\h*\i    out() << "> "; // indent: matched with \i
^\h*\j    out() << "< "; // dedent: matched with \j
\j        out() << "< "; // dedent: for each extra level dedented
%%
```
</div>

The `\h` pattern matches space and tabs, where tabs advance to the next column
that is a multiple of 8.  The tab multiplier can be changed by setting the
`−−tabs=N` option where `N` must be a positive integer that is a power of 2.

To add a pattern that consumes line continuations without affecting the
indentation levels, use a negative match, which is a new RE/flex feature:

<div class="alt">
```cpp
(?^\\\n\h*)  // lines ending in \ will continue on the next line
```
</div>

The negative pattern `(?^\\\n\h+)` consumes input internally as if we are
repeately calling `input()` (or `yyinput()` with `−−flex`).  We used it here to
consume the line-ending `\` and the indent that followed it, as if this text
was not part of the input, which ensures that the current indent positions are
not affected.

To scan input that continues on the next new line(s) while preserving the
current indent positions, use the RE/flex matcher `matcher().push_stops()` and
`matcher().pop_stops()`.  For example, to continue scanning after a `/*` for
multiple lines without indentation matching and up to a `*/` you can save the
current indent positions and transition to a new start condition state to scan
the content between `/*` and `*/`:

<div class="alt">
```cpp
%o tabs=8
%x CONTINUE
%%
^\h+          out() << "| "; // text is aligned to current indent margin
^\h*\i        out() << "> "; // indent
^\h*\j        out() << "< "; // dedent
\j            out() << "< "; // dedent, for each extra level dedented
"/*"          matcher().push_stops(); // save the indent margin/tab stops
              start(CONTINUE);        // continue w/o indent matching
<CONTINUE>{
"*/"          matcher().pop_stops();  // restore the indent margin/tab stops
              start(INITIAL);         // go back to the initial scanning state
.|\n          /* ignore */
}

```
</div>

See \ref reflex-states for more information about start condition states.

The Lexer class                                                 {#reflex-lexer}
---------------

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
        const reflex::Input& input = stdin,
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
        std::ostream&        os = std::cout)
    {
      in(input);
      out(os);
      return LEX();
    }
  };
  int NAMESPACE::LEXER::LEX()
  {
    static const reflex::Pattern PATTERN_INITIAL("(?m)(REGEX)");
    if (!has_matcher())
    {
      matcher(new Matcher(PATTERN_INITIAL, in(), this));
      start(INITIAL);
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
        const reflex::Input& input = stdin,
        std::ostream&        os    = std::cout)
      :
        FlexLexer(input, os)
    {
      INIT
    }
    virtual int LEX(void);
    int LEX(
        const reflex::Input& input,
        std::ostream        *os = nullptr)
    {
      in(input);
      out(os ? *os : std::cout);
      return LEX();
    }
  };
  int NAMESPACE::LEXER::LEX()
  {
    static const reflex::Pattern PATTERN_INITIAL("(?m)(REGEX)");
    if (!has_matcher())
    {
      matcher(new Matcher(PATTERN_INITIAL, in(), this));
      start(INITIAL);
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

To use a new Lexer class that is derived from the generated base Lexer class,
use option option `−−class=CLASS`.

When using this option, you must also explicitly define a `CLASS` class that is
derived from the base lexer class.  For example, the simplest derived Lexer
class would be:
  
<div class="alt">
```cpp
%option class=CLASS
%option lexer=LEXER
%option lex=LEX

%{
  class CLASS : public LEXER {
   public:
    int LEX();
  };
%}
```
</div>

The base Lexer class `LEXER` is the same class as was shown earlier, except
that the `LEX()` function is now a pure virtual method and the Lexer class is
abstract.  The `CLASS::LEX()` scanner function is generated by **reflex**.

You can omit the `lexer` and `lex` options shown above, which means that the
default Lexer class name and lex scanner function name are used.

⇢ [Back to contents](#)


Command-line options                                          {#reflex-options}
--------------------

To control the output of the **reflex** scanner generator, use command-line
options.  These options can also be specified in the lex specification with
`%%option` (or `%%o` for short):

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
%graphs-file=mygraph
```
</div>

Options that affect the regular expressions such as `%%option unicode` and
`%%option dotall` MUST be specified before the regular expressions are defined
and used in the specification.

The **reflex** command-line options are listed below.

### `−+`, `−−flex`

This generates a `yyFlexLexer` scanner class that is compatible with the
Flex-generated `yyFlexLexer` scanner class (assuming Flex with option `−+` for
C++).  The generated `yyFlexLexer` class has the usual `yytext` and other "yy"
variables and functions, as defined by the lex specification standard.  Without
this option, RE/flex actions should be used that are lexer class methods such
as `echo()` and `reflex::AbstractMatcher` class methods such as
`matcher.more()`, see \ref reflex-spec for more details.

### `−−bison`

This generates a scanner that works with Bison parsers, by defining global
(non-MT-safe and non-reentrant) "yy" variables and functions.  See
\ref reflex-bison for more details.  Use option `−−noyywrap` to remove the
dependency on the global `yywrap()` function.

### `−−bison-bridge`

This generates a scanner that works with Bison pure (MT-safe and reentrant)
parsers using a Bison bridge for one ore more scanner objects.  See \ref
reflex-bison for more details.

### `−−bison-locations`

This generates a scanner that works with Bison with locations enabled.  See
\ref reflex-bison for more details.

### `-R`, `−−reentrant`

This generates additional Flex-compatible `yylex()` reentrant scanner
functions.  RE/flex scanners are always reentrant, assuming that `%%class`
variables are used instead of global variables in the scanner's user code.
This is a Flex-compatibility option and should only be used with options
`−−flex` and `−−bison`.  See \ref reflex-bison.

### `−−main`

This generates a `main` function to create a stand-alone scanner that scans
data from standard input (using `stdin`).

### `−−nostdinit`

This initializes input to `std::cin` instead of using `stdin`.  Automatic
UTF decoding is not supported.  Use `stdin` for automatic UTF BOM detection and
decoding.

### `-u`, `−−unicode`

This makes `.` (dot), `\s`, `\w`, `\l`, and `\u` match Unicode and also groups
UTF-8 sequences in the regex, such that each UTF-8 encoded character in a regex
is properly matched as one wide character.  Note that `\S` and `\W` are *not
affected* by this switch.

### `-x`, `−−freespace`

This switches the **reflex** scanner generator to *free space mode*.  Regular
expressions in free space mode may contain spacing to improve readability.
Spacing within regular expressions is ignored, so use `" "` or `[ ]` to match a
space and `\h` to match a space or a tab character.  Actions in free space mode
MUST be placed in `{` and `}` blocks and all other code must be placed in `%{`
and `%}` blocks.  Patterns ending in an escape `\` continue on the next line.

### `-a`, `−−dotall`

This makes dot (`.`) in patterns match newline.  Normally dot matches a single
character except a newline (`\n` ASCII 0x0A).

### `-i`, `−−case_insensitive`

This ignore case in patterns.  Patterns match lower and upper case letters.
This switch only applies to ASCII letters.

### `-B`, `−−batch`

This generates a batch input scanner that reads the entire input all at once
when possible.  This scanner is fast, but consumes more memory depending on the
input data size.

### `-I`, `−−interactive`, `−−always-interactive`

This generates an interactive scanner and permits console input by sacrificing
speed.  By contrast, the default buffered input strategy is more efficient.

### `-d`, `−−debug`

This enables debug mode in the generated scanner.  Running the scanner produces
debug messages on `std::cerr` standard error and the `debug()` function returns
nonzero.  To temporarily turn off debug messages, use `set_debug(0)` in your
action code.  To turn debug messages back on, use `set_debug(1)`.  The
`set_debug()` and `debug()` functions are virtual, so you can override their
behavior in a derived lexer class.  Also enables assertions that check for
errors.

### `-s`, `−−nodefault`

This suppresses the default rule that ECHOs all unmatched text when no rule
matches.  With the `−−flex` option, the scanner reports "scanner jammed" when
no rule matches.  Without the `−−flex` option, unmatched input is silently
ignored.

### `-o FILE`, `−−outfile=FILE`

This saves the scanner to FILE instead of yy.lex.cpp.

### `-t`, `−−stdout`

This writes the scanner to stdout instead of to the yy.lex.cpp file.

### `-h`, `−−help`

This displays helpful information about **reflex**.

### `-V`, `−−version`

This displays the current **reflex** release version.

### `-v`, `−−verbose`

This displays a summary of scanner statistics.

### `-w`, `−−nowarn`

This disables warnings.

### `-P NAME`, `−−prefix=NAME`

This specifies NAME as a prefix for the generated `yyFlexLexer` class to
replace the default `yy` prefix.  This option is only useful with the `−−flex`
option and can be used in combination with `−−bison`.

### `-L`, `−−noline`

This suppresses the #line directives in the generated scanner code.

### `−−namespace=NAME`

This places the generated scanner class in the C++ namespace NAME scope.  Which
means `NAME::Lexer` (and `NAME::yyFlexLexer` when option `−−flex` is used).

### `−−lexer=NAME`

This defines the NAME of the generated scanner class and replaces the default
name `Lexer` (and replaces `yyFlexLexer` when option `−−flex` is used).

### `−−lex=NAME`

This defines the NAME of the generated scanner function to replace the function
name `lex()` (and `yylex()` when option `−−flex` is used).

### `−−class=NAME`

This defines the NAME of the scanner class that should be derived from the
generated base `Lexer` class.  Use this option when defining your own scanner
class named NAME.  You can declare a custom lexer class in the first section of
the lex specification.  Because the custom lexer class is user-defined,
**reflex** generates the implementation of the `lex()` scanner function for
this specified class.

### `−−yyclass=NAME`

This option combines options `−−flex` and `−−class=NAME`.

### `-m reflex`, `−−matcher=reflex`

This generates a scanner that uses the RE/flex `reflex::Matcher` class with a
POSIX matcher engine.  This is the default matcher for scanning.  This option
is best for Flex compatibility.  The matcher supports Unicode, lazy
quantifiers, word boundaries, indent/nodent/dedent matching, and supports FSM
output for visualization with Graphviz.

### `-m boost`, `−−matcher=boost`

This generates a scanner that uses the `reflex::BoostPosixMatcher` class with a
Boost.Regex POSIX matcher engine for scanning.  The matcher supports Unicode
and word boundary anchors, but not lazy quantifiers.  No Graphviz output.

### `-m boost-perl`, `−−matcher=boost-perl`

This generates a scanner that uses the `reflex::BoostPerlMatcher` class with a
Boost.Regex normal (Perl) matcher engine for scanning.  The matching behavior
differs from the POSIX *leftmost longest rule* and results in the first
matching rule to be applied instead of the rule that produces the longest
match.  The matcher supports lazy quantifiers and word boundary anchors.  No
Graphviz output.

### `-f`, `−−full`

(RE/flex matcher only).  This option adds the FSM to the generated code as a
static opcode table, thus generating the scanner in full.  FSM construction
overhead is eliminated when the scanner is initialized, resulting in a scanner
that starts scanning the input immediately.  This option has no effect when
option `−−fast` is used.

### `-F`, `−−fast`

(RE/flex matcher only).  This option adds the FSM to the generated code as
optimized native C++ code.  FSM construction overhead is eliminated when the
scanner is initialized, resulting in a scanner that starts scanning the input
immediately.  The generated code takes more space compared to the `−−full`
option.

### `−−graphs-file[=FILE]`

(RE/flex matcher only).  This generates a Graphviz file FILE.gv, where FILE is
optional.  When FILE is omitted the **reflex** command generates the file
reflex.S.gv for each start condition state S defined in the lex specification.
This includes reflex.INITIAL.gv for the INITIAL start condition state.  This
option can be used to visualize the RE/flex matcher's finite state machine with
the [Graphviz dot](http://www.graphviz.org) tool.  For example:

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

### `−−tables-file[=FILE]`

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

### `−−header-file[=FILE]`

This generates a C++ header file FILE.h that declares the lexer class, in
addition to the generated lexer class code, where FILE is optional.  When FILE
is omitted the **reflex** command generates lex.yy.h.

### `−−regexp-file[=FILE]`

This generates a text file FILE.txt that contains the scanner's regular
expression patterns, where FILE is optional.  When FILE is omitted the
**reflex** command generates reflex.S.txt for each start condition state S.
The regular expression patterns are converted from the lex specification and
translated into valid C++ strings that can be used with a regex library for
pattern matching.

### `−−pattern=NAME`

This defines a custom pattern class NAME for the custom matcher specified with
option `-m`.

### `−−include=FILE`

This defines a custom include FILE.h to include for the custom matcher
specified with option `-m`.

### `−−tabs=N`

This sets the tab size to N, where N > 0 must be a power of 2.  The tab size is
used internally to determine the column position in indent `\i` and dedent `\j`
matching.  It has no effect otherwise.

### `−−yywrap` and `−−noyywrap`

Option `−−yywrap` generates a scanner that calls the global `int yywrap()`
function when EOF is reached.  Only applicable when `−−flex` is used for
compatibility.  The same applies when options `−−flex`  and `−−bison` are used
together.  Use `−−noyywrap` to disable the dependence on this global function.
This option has no effect for C++ lexer classes, which have a virtual
`int wrap()` (or `yywrap()` with option `−−flex`) method.

### `−−lineno`, `−−yymore`, `−−batch`

These options are enabled by default and have no effect.

⇢ [Back to contents](#)


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

Consider for example the following lex specification with rules that are
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
`int(?=[^A-Za-z0-9_])` which is written in a lex specification as
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

⇢ [Back to contents](#)


Switching input sources                                         {#reflex-input}
-----------------------

To create a Lexer class instance that reads from a designated input source
instead of standard input, pass the input source as the first argument to its
constructor and use the second argument to optionally set an `std::ostream`:

```cpp
Lexer lexer(input, std::cout);
```

the same with the `−−flex` option becomes:

```cpp
yyFlexLexer lexer(input, std::cout);
```

where `input` is a `reflex::Input` object.  The `reflex::Input` constructor
takes a `FILE*` descriptor, `std::istream`, a string `std::string` or `char*`,
or a wide string `std::wstring` or `wchar_t*`.

To switch input to another source while using the scanner, use `in(input)`:

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

When the end of the input (EOF) is reached, and you want to switch to new
input, then you should clear the EOF state first with
`lexer.matcher().set_end(false)` to reset EOF.  This allows more input to be
scanned with `lexer.in(i)`.  You can also use `lexer.matcher().reset()` to
clear the state, which clears the line and column counters, resets the internal
anchor and boundary flags for anchor and word boundary matching, and resets to
standard buffered input mode.

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

Switching input sources (via either `matcher(m)` or `in(input)`) does not
change the current start condition state.

When the scanner reaches the end of the input, it will check the `int wrap()`
method to detetermine if scanning should continue.  If `wrap()` returns one (1)
the scanner terminates and returns zero to its caller.  If `wrap()` returns
zero (0) then the scanner continues.  In this case `wrap()` should set up a new
input source to scan.

To implement a `wrap()` (and `yywrap()` when option `−−flex` is used), create a
derived Lexer class using option `class=NAME` (or `yyclass=NAME`) and override
the `wrap()` (or `yywrap()`) method:

<div class="alt">
```cpp
%option class=Tokenizer

%{
  class Tokenizer : Lexer {
   public:
    virtual bool wrap()
    {
      in(std::in);
      return in().good(); // true if stdin is still OK (not EOF or error)
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

For example, the following lex specification processes `#include` directives by
switching matchers and using the stack of matchers to permit nested `#include`
directives up to a depth of 99 files:

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
matcher class (to customize reading.

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

The `LexerInput` method may be invoked multiple times until it returns zero
indicating EOF.

⇢ [Back to contents](#)


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

Start conditions are declared in the definitions section (the first section) of
the lex specification using `%%state` or `%%xstate` (or `%%s` and `%%x` for
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

⇢ [Back to contents](#)


Interfacing with Bison and Yacc                                 {#reflex-bison}
-------------------------------

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
Flex actions and variables are globally accessible (outside the rules section
of the lex specification) with the exception of `yy_push_state`,
`yy_pop_state`, and `yy_top_state` that are class methods.  Furthermore, `yyin`
and `yyout` are macros and cannot be (re)declared or accessed as global
variables, but these can be used as if they are variables to assign a new input
source and to set the output stream.  To avoid compilation errors when using
globals such as `yyin`, use **reflex** option `−−header-file` to generate a
header file `lex.yy.h` to include in your code.  Finally, in code outside of
the rules section you must use `yyinput` instead of `input`, use the global
action `yyunput` instead of `unput`, and use the global action `yyoutput`
instead of `output`.

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
/* yacc grammar (.y file) assuming C++ */

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

The ugly Flex macro `YY_DECL` is not supported by RE/flex.  This macro is
needed with Flex to redeclare the `yylex()` function signature, for example to
take an additional `yylval` parameter that must be passed through from
`yyparse()` to `yylex()`.  Because the generated scanner uses a Lexer class for
scanning, the class can be extended with `%%class{` and `%}` to hold state
information and additional token-related values.  These values can then be
exchanged with the parser using getters and setters, which is preferred over
changing the `yylex()` function signature.

Yacc and Bison use the global variable `yylval` to exchange token values.
Reentrant Bison parsers pass the `yylval` to the `yylex()` function as a
parameter.  RE/flex supports all of these Bison-specific features with
command-line options.

To change the generated code for use with Bison, use the following options:

  Options                                       | Method                                    | Global functions and variables
  --------------------------------------------- | ----------------------------------------- | ------------------------------
  &nbsp;                                        | `int Lexer::lex()`                        | &nbsp;                          
  `−−flex`                                      | `int yyFlexLexer::yylex()`                | &nbsp;                          
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

The **reflex** option `−−bison-bridge` expects a Bison "pure parser":

<div class="alt">
```cpp
/* yacc grammar (.y file) assuming C++ */

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

For the `−−bison-bridge` option, the `yyscan_t` argument type of
`yylex()` is a `void*` type that passes the scanner object to this global
function (as defined by `YYPARSE_PARAM` and `YYLEX_PARAM`).  The function then
invokes this scanner's lex function.  This option also passes the `yylval`
value to the lex function, which is a reference to an `YYSTYPE` value.

For the `−−reentrant` and `−−bison-bridge` options two additional functions are
generated that can be used to create a new scanner and delete the scanner:

<div class="alt">
```cpp
yyscan_t scanner = nullptr;
yylex_init(&scanner);      // create a new scanner
...
// three possibilities, depending on options:
int token = yylex(scanner);                   // reentrant scan
int token = yylex(&yylval, scanner);          // reentrant scan with bison-bridge
int token = yylex(&yylval, &yylloc, scanner); // reentrant scan with bison-bridge and bison-locations
...
yylex_destroy(scanner);    // delete a scanner
scanner = nullptr;
```
</div>

The option `−−bison-locations` expects a Bison parser with the locations
feature enabled.  This feature provides line and column numbers of the matched
text for error reporting.  For example:

<div class="alt">
```cpp
/* yacc grammar (.y file) assuming C++ */

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
  /* add this only to patch old bison versions */
  extern int yylex(YYSTYPE*, YYLTYPE*);
  #define YYLEX_PARAM &yylval, &yylloc
%}

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

And a final example that combines options `−−bison-locations` and
`−−bison-bridge`, which expects a Bison pure-parser with locations enabled:

<div class="alt">
```cpp
/* yacc grammar (.y file) */

%{
  #include "lex.yy.h"
  #define YYPARSE_PARAM scanner
  #define YYLEX_PARAM   scanner
%}

%pure-parser
%locations

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

When Bison `%locations` with `%define api.pure full` is used, `yyerror` has the
signature `void yyerror(YYLTYPE *locp, char const *msg)`.  This function
signature is required to obtain the location information with Bison
pure-parsers.

@note `yylval` is not a pointer argument but is always passed by reference and
can be used as such in the scanner's rules.

@note Because `YYSTYPE` is declared by the parser, do not forget to add a
`#include "y.tab.h"` to the top of your lex specification:

<div class="alt">
```cpp
%top{
  #include "y.tab.h"    /* include y.tab.h generated by bison */
%}
```
</div>

⇢ [Back to contents](#)


Examples                                                     {#reflex-examples}
--------

Some lex specification examples to generate scanners with RE/flex.

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
lex specification above.  This option when combined with `−−flex` produces the
global "yy" functions and variables.  This means that you can use RE/flex
scanners with Bison (Yacc) and with any other C code, assuming everything is
compiled together with a C++ compiler.

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

### Example 4

This example lex specification in Flex syntax scans C/C++ source code.  It uses
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
Unicode identifier names.  Note that we can use `matcher().columno` in the
error message.  The `matcher()` object associated with the Lexer offers several
other methods that Flex does not support.

⇢ [Back to contents](#)


Limitations                                               {#reflex-limitations}
-----------

The RE/flex matcher engine uses an efficient FSM.  There are known limitations
to FSM matching that apply to Lex/Flex and therefore also apply to RE/flex:

- Lookaheads (trailing contexts) must appear at the end of a pattern, so `a/b`
  and `a(?=b)` are permitted, but `(a/b)c` and `a(?=b)c` are not.
- Lookaheads cannot be properly matched when the ending of the first part of
  the pattern matches the beginning of the second part, such as `zx*/xy*`,
  where the `x*` matches the `x` at the beginning of the lookahead pattern.
- Anchors must appear at the start or at the end of a pattern.  The begin of
  buffer/line anchors `\A` and `^`, end of buffer/line anchors `\z` and `$` and
  the word boundary anchors must start or end a pattern.  For example,
  `\<cow\>` is permitted, but `.*\Bboy` is not.

Current **reflex** tool limitations that may be removed in future versions:

- The `REJECT` action is not supported.
- Translations `%T` are not supported.

Boost.Regex issues to be resolved in future versions (this is not affecting the
default use of the RE/flex regex engine by the **reflex** command-line tool):

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

⇢ [Back to contents](#)


The RE/flex regex library                                              {#regex}
=========================

The RE/flex regex library consists of a set of C++ templates and classes that
encapsulate regex engines in a standard API for scanning, tokenizing,
searching, and splitting of strings, wide strings, files, and streams.

⇢ [Back to contents](#)


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

⇢ [Back to contents](#)


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
- no mode modifiers `(?imsx:φ)`;
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

⇢ [Back to contents](#)


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
sets the tab size to 8 for indent `\i` and dedent `\j` matching.

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

⇢ [Back to contents](#)


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
    case reflex::regex_error::invalid_modifier:     std::cerr << "invalid (?ismx:) modifier"; break;
    case reflex::regex_error::invalid_syntax:       std::cerr << "invalid regex syntax"; break;
    case reflex::regex_error::exceeds_limits:       std::cerr << "exceeds complexity limits: {n,m} range too large"; break;
  }
}
```

⇢ [Back to contents](#)


Regex converters                                               {#regex-convert}
----------------

To work around limitations of regex libraries and to support Unicode matching,
RE/flex offers converters to translate expressive regex syntax forms (with
Unicode patterns defined in section \ref reflex-patterns) to regex strings
that the selected regex engines can handle.

The converters translate `\p` Unicode classes, translate character
class set operations such as `[a-z−−[aeiou]]`, convert escapes such as `\X`,
and enforce `(?imsx:φ)` mode modifiers to a regex string that the underlying
regex library understands and can use.

Each converter is specific to the regex engine.  Use a converter for the
matcher of your choice:

- `std::string reflex::BoostMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
- `std::string reflex::BoostPerlMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
- `std::string reflex::BoostPosixMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
- `std::string reflex::StdMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
- `std::string reflex::StdEcmaMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
- `std::string reflex::StdPosixMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
- `std::string reflex::Matcher::convert(const std::string& regex, reflex::convert_flag_type flags)`

where `flags` is optional.  When specified, it can be a combination of the
following `reflex::convert_flag` flags:

  Flag        | Effect
  ----------- | ---------------------------------------------------------------
  `unicode`   | `.`, `\s`, `\w`, `\l`, `\u`, `\S`, `\W`, `\L`, `\U` match Unicode
  `recap`     | remove capturing groups, add capturing groups to the top level
  `lex`       | convert Lex/Flex regular expression syntax
  `u4`        | convert `\uXXXX` and UTF-16 surrogate pairs
  `anycase`   | convert regex to ignore case, same as `(?i)`
  `multiline` | regex has multiline anchors `^` and `$`, same as `(?m)`
  `dotall`    | convert `.` (dot) to match all, same as `(?s)`
  `freespace` | convert regex by removing spacing, same as `(?x)`

For example:

```cpp
#include <reflex/matcher.h> // reflex::Matcher, reflex::Input, reflex::Pattern

// use a Matcher to check if sentence is in Greek:
static const reflex::Pattern pattern(reflex::Matcher::convert("[\\p{Greek}\\p{Zs}\\pP]+"));
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
  regex = reflex::BoostMatcher::convert(argv[1]));
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
    case reflex::regex_error::invalid_modifier:     std::cerr << "invalid (?ismx:) modifier"; break;
    case reflex::regex_error::invalid_syntax:       std::cerr << "invalid regex syntax"; break;
  }
}
```

⇢ [Back to contents](#)


Methods and iterators                                          {#regex-methods}
---------------------

The RE/flex abstract matcher offers four operations for matching with the regex
engines that are derived from this base abstract class:

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `matches()` | true if the input from begin to end matches the regex pattern
  `find()`    | search input and return true if a match was found
  `scan()`    | scan input and return true if input at current position matches
  `split()`   | split input at the next match

The `find`, `scan`, and `split` methods are also implemented as input iterators
that apply filtering tokenization, and splitting:

  Iterator range                  | Acts as a | Iterates over
  ------------------------------- | --------- | -------------------------------
  `find.begin()`...`find.end()`   | filter    | all matches
  `scan.begin()`...`scan.end()`   | tokenizer | continuous matches
  `split.begin()`...`split.end()` | splitter  | text between matches

To obtain details of a match use the following methods:

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `accept()`  | returns group capture index or zero if not captured/matched
  `text()`    | returns `\0`-terminated `const char*` string match
  `wtext()`   | returns `std::wstring` wide string match (converted from UTF-8)
  `pair()`    | returns `std::pair<size_t,std::string>(accept(), text())`
  `wpair()`   | returns `std::pair<size_t,std::wstring>(accept(), wtext())`
  `size()`    | returns the length of the text match in bytes
  `wsize()`   | returns the length of the match in number of wide characters
  `rest()`    | returns `\0`-terminated `const char*` of the rest of the input
  `more()`    | tells the matcher to append the next match (adjacent matches)
  `less(n)`   | cuts `text()` to `n` bytes and repositions the matcher
  `lineno()`  | returns line number of the match, starting with line 1
  `columno()` | returns column number of the match, starting with 0
  `first()`   | returns position of the first character of the match
  `last()`    | returns position of the last + 1 character of the match
  `at_bol()`  | true if matcher reached the begin of a new line `\n`
  `at_bob()`  | true if matcher is at the start of input, no matches consumed
  `at_end()`  | true if matcher is at the end of input
  `[0]`       | operator returns `std::pair<const char*,size_t>(text(),size())`
  `[n]`       | operator returns n'th capture `std::pair<const char*,size_t>`

Note: the `wtext()` and `wsize()` methods are more expensive to invoke and take
more than constant time to compute, whereas `text()` and `size()` take constant
time.

In addition, type casts of matcher objects and iterators are available:

- Casting to `size_t` gives the matcher's `accept()` index.
- Casting to `std::string` is the same as storing `text()` in a string with
  `std::string(text(), size())`.
- Casting to `std::wstring` is the same as invoking `wtext()`.
- Casting to a `std::pair<size_t,std::string>` is the same as invoking the
  `pair()` method.
- Casting to a `std::pair<size_t,std::wstring>` is the same as invoking the
  `wpair()` method.

Four special methods can be used to manipulate the input stream directly:

  Method     | Result
  ---------- | ----------------------------------------------------------------
  `input()`  | returns next char 0..255 from the input, matcher then skips it
  `winput()` | returns the next wide character from the input, matcher skips it
  `unput(c)` | put char `c` back unto the stream, matcher then takes it
  `peek()`   | returns the next char 0..255 from the input without consuming it

These methods operate on the `in` public instance variable of a matcher:

  Variable | Usage
  -------- | ------------------------------------------------------------------
  `in`     | the `reflex::Input` object used by the matcher
  `find`   | the `reflex::AbstractMatcher::Operation` functor for searching
  `scan`   | the `reflex::AbstractMatcher::Operation` functor for scanning
  `split`  | the `reflex::AbstractMatcher::Operation` functor for splitting

To initialize a matcher for interactive use, to assign a new input source or to
change its pattern, you can use the following methods:

  Method          | Result
  --------------- | -----------------------------------------------------------
  `input(i)`      | set input to `reflex::Input i` (string, stream, or `FILE*`)
  `pattern(p)`    | set pattern to `p` (string regex or `reflex::Pattern`)
  `has_pattern()` | true if the matcher has a pattern assigned to it
  `own_pattern()` | true if the matcher has a pattern to manage and delete
  `pattern()`     | get the pattern object, `reflex::Pattern` or `boost::regex`
  `buffer()`      | buffer all input at once, returns true if successful
  `buffer(n)`     | set the adaptive buffer size to `n` bytes to buffer input
  `interactive()` | sets buffer size to 1 for console-based (TTY) input
  `flush()`       | flush the remaining input from the internal buffer
  `reset()`       | resets the matcher, restarting it from the remaining input
  `reset(o)`      | resets the matcher with new options string `o` ("A?N?T?")

The first two methods return a reference to the matcher, so multiple method
invocations can be chained together.

A matcher reads from the specified input source using its virtual method
`size_t get(char *s, size_t n)` that simply returns `in.get(s, n)`.  This
method can be overriden by a derived matcher class to customize reading.

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `get(s, n)` | fill `s[0..n-1]` with next input, returns number of bytes added
  `wrap()`    | returns false (can be overriden to wrap input)

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
We can set a pattern for the matcher after its instantiation, by using the
`pattern(p)` method.  In this case the input does not need to be specified to
immediately force reading the sources of input that we assigned in our
`wrap()`.

⇢ [Back to contents](#)


The Input class                                                  {#regex-input}
---------------

A matcher may accept several types of input, but can only read from one input
source at a time.  Input to a matcher is represented by a single
`reflex::Input` class instance that the matcher uses internally.

An input object can be assigned `std::string` and `char*` strings, wide strings
`std::wstring` and `wchar_t*`, a `FILE*`, or a `std::istream`.

Wide strings are internally converted to UTF-8 for matching, which effectively
normalizes the input for matching.

Conversion from wide string to UTF-8 is shown in the example below.  The
copyright symbol `©` with Unicode U+00A9 is matched against its UTF-8 sequence
`C2 A9`:

```cpp
if (reflex::Matcher("\xc2\xa9", L"©").matches())
  std::cout << "copyright symbol matches\n";
```

To obtain the properties of an input object use the following methods:

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `size()`    | size of the input in (UTF-encoded) bytes, or zero when unknown
  `good()`    | input is available to read (no error and not EOF)
  `eof()`     | end of input (but use only `at_end()` with matchers!)
  `cstring()` | the current `const char*` (of a `std::string`) or NULL
  `wstring()` | the current `const wchar_t*` (of a `std::wstring`) or NULL
  `file()`    | the current `FILE*` file descriptor or NULL
  `istream()` | a `std::istream*` pointer to the current stream object or NULL

File content specified with a `FILE*` file descriptor can be encoded in ASCII,
binary, or UTF-8/16/32 formats.  UTF-16/32 file content is automatically
decoded and converted to UTF-8 (i.e. streaming) to normalize the input for
matching.

A [UTF byte order mark (BOM)](www.unicode.org/faq/utf_bom.html) is detected in
the content of a file by the matcher, which enables UTF-8 normalization of the
input automatically.

The file encoding is obtained with the `file_encoding()` method of a
`reflex::Input` object and it returns one of:

  Encoding constant       | Effect when set
  ----------------------- | ---------------------------------------------------
  `Input::Const::plain`   | plain octets with ASCII, binary, or UTF-8
  `Input::Const::utf16be` | UCS-2/UTF-16 big endian
  `Input::Const::utf16le` | UCS-2/UTF-16 little endian
  `Input::Const::utf32be` | UCS-4/UTF-32 big endian
  `Input::Const::utf32le` | UCS-4/UTF-32 little endian

To override the file encoding, use `file_encoding(encoding constant)`.

⇢ [Back to contents](#)


Examples                                                      {#regex-examples}
--------

Some examples to demonstrate the concepts discussed.

### Example 1

This example illustrates the `find` and `split` methods and iterators with a
RE/flex `reflex::Matcher` and `reflex::BoostMatcher`:

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
while (matcher.find() == true)
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

### Example 2

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
  if (wordmatcher.input(words[i]).matches() == true)
    std::cout << wordmatcher.text() << ", ";
```

When executed this code prints:

    Monty, Flying, Circus, 

### Example 3

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

### Example 4

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

### Example 5

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

### Example 6

The RE/flex matcher engine `reflex::matcher` only recognizes group captures at
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

### Example 7

This example shows how a `FILE*` file descriptor is used as input.  The file
encoding is obtained from the UTF BOM, when present in the file.  Note that the
file's state is accessed through the matcher's member variable `in`:

```cpp
#include <reflex/boostmatcher.h>

using namespace reflex;

BoostMatcher matcher("\\s+", fopen("filename", "r"));

if (matcher.in.file() && matcher.in.good())
{
  switch (matcher.in.file_encoding())
  {
    case Input::Const::plain:   std::cout << "ASCII or binary file"; break;
    case Input::Const::utf8:    std::cout << "UTF-8 file";           break;
    case Input::Const::utf16be: std::cout << "UTF-16 big endian";    break;
    case Input::Const::utf16le: std::cout << "UTF-16 little endian"; break;
    case Input::Const::utf32be: std::cout << "UTF-32 big endian";    break;
    case Input::Const::utf32le: std::cout << "UTF-32 little endian"; break;
  }
  std::cout << " of " << matcher.in.size() << " converted bytes to read\n";
  matcher.buffer(); // because Boost.Regex partial_match is broken!
  if (matcher.split() == true)
    std::cout << "Starts with: " << matcher.text() << std::endl;
  std::cout << "Rest of the file is: " << matcher.rest();
  fclose(matcher.in.file());
}
```

### Example 8

This example shows how to override the file encoding, such as in cases when a file
with wide character content has no UTF BOM or when a binary file is read that
could start with a BOM that should be ignored:

```cpp
#include <reflex/boostmatcher.h>

using namespace reflex;

BoostMatcher matcher("\\s+", fopen("filename", "r"));

if (matcher.in.file() && matcher.in.good())
{
  matcher.in.file_encoding(Input::Const::plain);
  ...
  fclose(matcher.in.file());
}
```

⇢ [Back to contents](#)


Where to download                                                   {#download}
=================

[Download RE/flex](https://sourceforge.net/projects/re-flex) from SourceForge
and visit the GitHub [RE/flex repository](https://github.com/Genivia/RE-flex).

⇢ [Back to contents](#)


License and copyright                                                 {#license}
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

⇢ [Back to contents](#)

Copyright (c) 2017, Robert van Engelen, Genivia Inc. All rights reserved.
