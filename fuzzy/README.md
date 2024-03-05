FuzzyMatcher
============

A C++ class extension of the [RE/flex](https://github.com/Genivia/RE-flex)
Matcher class for efficient fuzzy matching and fuzzy search with regex patterns.
Regex patterns are of the POSIX ERE type, but also support Unicode matching,
lazy quantifiers, word boundaries and lookaheads.

- specify max error as a parameter, i.e. the max edit distance or
  [Levenshstein distance](https://en.wikipedia.org/wiki/Levenshtein_distance)

- regex patterns are compiled into DFA VM opcodes for speed

- practically linear execution time in the length of the input, using
  DFA-based matching with minimal backtracking limited by the specified max
  error parameter

- supports the full RE/flex regex pattern syntax, which is POSIX-based with
  many additions: <https://www.genivia.com/doc/reflex/html/#reflex-patterns>

- no group captures (yet), except for top-level sub-pattern group captures,
  e.g. `(foo)|(bar)|(baz)` but not `(foo(bar))`

- newlines (`\n`) and NUL (`\0`) characters are never deleted or substituted
  to ensure that fuzzy matches do not extend the pattern match beyond the
  number of lines specified by the regex pattern

- quote regex patterns with `\Q` and `\E` for fuzzy string matching and search

- FuzzyMatcher is used in the [ugrep](https://github.com/Genivia/ugrep) project

Requires
--------

[RE-Flex](https://github.com/Genivia/RE-flex) version 4.0 or greater, because
of regex pattern analysis and translation updates to RE/flex 4.0 that are
also used by [ugrep 5.0](https://github.com/Genivia/ugrep).

Examples
--------

pattern    | max | fuzzy `find()` matches            | but not
---------- | --- | --------------------------------- | -------------------------
`abc`      | 1   | `abc`, `ab`, `ac`, `axc`, `axbc`  | `a`, `axx`, `axbxc`, `bc`
`año`      | 1   | `año`, `ano`, `ao`                | `anno`, `ño`
`ab_cd`    | 2   | `ab_cd`, `ab-cd`, `ab Cd`, `abCd` | `ab\ncd`, `Ab_cd`, `Abcd`
`a[0-9]+z` | 1   | `a1z`, `a123z`, `az`, `axz`       | `axxz`, `A123z`, `123z`

Note that the first character of the pattern must match when searching a corpus
with the `find()` method.  By contrast, the `matches()` method to match a
corpus from start to end does not impose this requirement:

pattern    | max | fuzzy `matches()` matches                            | but not
---------- | --- | ---------------------------------------------------- | -------------------------
`abc`      | 1   | `abc`, `ab`, `ac`, `Abc`, `xbc` `bc`, `axc`, `axbc`  | `a`, `axx`, `Ab`, `axbxc`
`año`      | 1   | `año`, `Año`, `ano`, `ao`, `ño`                      | `anno`
`ab_cd`    | 2   | `ab_cd`, `Ab_Cd`, `ab-cd`, `ab Cd`, `Ab_cd`, `abCd`  | `ab\ncd`, `AbCd`
`a[0-9]+z` | 1   | `a1z`, `A1z`, `a123z`, `az`, `Az`, `axz`, `123z`     | `axxz`

Optimizations
-------------

Fuzzy `find()` and `split()` make a second pass over a fuzzy-matched pattern
when the match has a nonzero error.  This second pass checks if an exact match
exists or if a better match exists that overlaps with the first pattern found.
For example, the pattern `abc` is found to fuzzy match all of the text `aabc`
with one error (an extra `a`).  The second pass of `find()` detects an exact
match after skipping the first `a`.  Likewise, the pattern `abc` is found to
fuzzy match `ababc` with a match for `aba` with one error (substitution of `c`
by an `a`).  The second pass of `find()` detects an exact match after skipping
`ab` in the text.  This approach is faster than minimizing the edit distance
when searching text, while returning exact matches when possible.

Usage
-----

### Fuzzy searching

    #include "fuzzymatcher.h"

    // MAX:   optional maximum edit distance, default is 1, up to 255
    // INPUT: a string, wide string, FILE*, or std::istream object
    reflex::FuzzyMatcher matcher("PATTERN", [MAX,] INPUT);

    // find all pattern matches in the input
    while (matcher.find())
    {
      std::cout << matcher.text() << '\n'  // show each fuzzy match
      std::cout << matcher.edits() << '\n' // edit distance (when > 0 not guaranteed minimal)
    }

See the [RE/flex user guide](https://www.genivia.com/doc/reflex/html/#regex-methods)
for the full list of `Matcher` class methods available to extract match info.
The `edits()` method is a `FuzzyMatcher` extension of the `Matcher` class.

### Fuzzy matching

    #include "fuzzymatcher.h"

    // match the whole input (here in one go with a temporary fuzzy matcher object)
    if (reflex::FuzzyMatcher("PATTERN", [MAX,] INPUT).matches())
    {
      std::cout << "fuzzy pattern matched\n";
    }

### Fuzzy splitting (text between matches)

    #include "fuzzymatcher.h"

    reflex::FuzzyMatcher matcher("PATTERN", [MAX,] INPUT);

    // split the input into parts separated by pattern matches
    while (matcher.split())
    {
      std::cout << matcher.text() << '\n' // show text between fuzzy matches
    }

### Character insertion, deletion and substitution

The `MAX` parameter may be combined with one or more of the following flags:

- `reflex::FuzzyMatcher::INS` insertions allow extra character(s) in the input
- `reflex::FuzzyMatcher::DEL` deletions allow missing character(s) in the input
- `reflex::FuzzyMatcher::SUB` substitutions count as one edit
- `reflex::FuzzyMatcher::BIN` ASCII/binary fuzzy matching (default is Unicode with Unicode pattern converter, see below)

For example, to allow approximate pattern matches to include up to three
character insertions, but no deletions or substitutions (allowing insertions
only is actually the most efficient fuzzy matching possible):

    reflex::FuzzyMatcher matcher(regex, 3 | reflex::FuzzyMatcher::INS, INPUT);

To allow up to three insertions or deletions (note that a substitution counts
as two edits: one insertion and one deletion):

    reflex::FuzzyMatcher matcher(regex, 3 | reflex::FuzzyMatcher::INS | reflex::FuzzyMatcher::DEL, INPUT);

When no flags are specified with `MAX`, fuzzy matching is performed with
insertions, deletions, and substitutions, each counting as one edit.

### Full Unicode support

To support full Unicode pattern matching, such as `\p` Unicode character
classes, convert the regex pattern before using it as follows:

    std::string regex(reflex::Matcher::convert("PATTERN", reflex::convert_flag::unicode));
    reflex::FuzzyMatcher matcher(regex, [MAX,] INPUT);

### Static regex patterns

Fixed patterns should be constructed (and optionally Unicode converted) just
once statically to avoid repeated construction, e.g. in the body of loops and
in frequently executed functions:

    static const reflex::Pattern pattern(reflex::Matcher::convert("PATTERN", reflex::convert_flag::unicode));
    reflex::FuzzyMatcher matcher(pattern, [MAX,] INPUT);

Compiling
---------

Assuming `reflex` dir with RE/flex source code is locally built:

    c++ -o myapp myapp.cpp -Ireflex/include reflex/lib/libreflex.a

When the `libreflex` library is built and installed:

    c++ -o myapp myapp.cpp -lreflex

Testing
-------

    $ make ftest
    $ ./ftest 'ab_cd' 'abCd' 2
    matches(): match (2 edits)
    find():    'abCd' at 0 (2 edits)
    split():   '' at 0 (2 edits)
    split():   '' at 4 (0 edits)

License
-------

BSD-3
