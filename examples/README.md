[![logo][logo-url]][reflex-url]

The examples with Bison require the installation of [Bison][bison-url].

Examples with RE/flex C++ lexer methods
---------------------------------------

- `minic.l,` `minic.y`, `minic.hpp`, and `minicdemo.c` Mini C compiler with Bison 3.2 C++ complete parser, generates Java bytecode in class files
- `braille.l` a braille translator, inspired by the re2c example but extended and fixed to make it usable
- `ptokens.l` and `pdefs.l` a Python source tokenizer
- `calc.l` and `calc.y` calculator with Bison 3.2 bison-bridge parser, supports Unicode identifier variables, resolves input errors
- `csv.l` scanner to parse CSV rows from a file into a C++ vector
- `yaml.l` YAML 1.2 parser and writer
- `indent1.l` indentation detection and reformatting example, basic
- `indent2.l` indentation detection and reformatting example, advanced with start conditions
- `unicode.l` a very simple Unicode word matcher to demonstrate Unicode matching
- `scanstrings.l` demonstrates string and buffer scanning
- `mmap.l` demonstrates mmap(2) to scan C/C++ source code files fast with RE/flex read-only memory scanning
- `gz.l` demonstrates scanning zlib-compressed (and uncompressed) C/C++ source code files
- `dos.l` demonstrates scanning DOS-formatted files with `reflex::BufferedInput::dos_streambuf`
- `fastfind.l` demonstrates fast search for patterns in input, rather than scanning the input
- `tag.l` simple lexer to print out all XML/HTML tags
- `tag_lazy.l` simple lexer to print XML/HTML tags, uses lazy quants
- `tag_lazystates.l` lexer to print XML tags, uses lazy quants and states
- `tag_unicode.l` lexer to print XML tags, uses lazy quants and states, checks UTF-8 validity
- `tag_tidy.l` lexer to tidy and normalize HTML
- `cow.l` scanner to convert cows
- `cows.l` scanner to convert cows, uses word boundary anchors
- `reflexexample3.l` example RE/flex scanner adapted from the Flex documentation with C bison
- `reflexexample3.lxx` example RE/flex scanner adapted from the Flex documentation with C++ bison
- `reflexexample4.l` example RE/flex scanner adapted from the Flex documentation with C bison-bridge
- `reflexexample4.lxx` example RE/flex scanner adapted from the Flex documentation with C++ bison-bridge
- `reflexexample5.l` example RE/flex scanner adapted from the Flex documentation with C bison-bridge
- `reflexexample5.lxx` example RE/flex scanner adapted from the Flex documentation with C++ bison-bridge
- `reflexexample6.l` example RE/flex reentrant scanner with state stacks
- `reflexexample7.l` example RE/flex scanner adapted from the Flex documentation with C bison-locations
- `reflexexample7.lxx` example RE/flex scanner adapted from the Flex documentation with C++ bison-locations
- `reflexexample8.l` example RE/flex scanner adapted from the Flex documentation with C bison-bridge and bison-locations
- `reflexexample8.lxx` example RE/flex scanner adapted from the Flex documentation with C++ bison-bridge and bison-locations
- `reflexexample9.lxx` example RE/flex scanner with Bison 3.0 %skeleton "lalr1.cc" C++ parser with bison-cc and bison-locations
- `reflexexample10.l` example RE/flex scanner adapted from the Flex documentation to use multiple buffers
- `reflexexample11.lxx` example RE/flex scanner with Bison 3.2 C++ bison-complete and bison-locations

Examples with the backward-compatible Flex lexer functions
----------------------------------------------------------

- `json.l` JSON parser and writer
- `ctokens.l` and `cdefs.l` a C/C++ source tokenizer
- `jtokens.l` and `jdefs.l` a Java source tokenizer, meets the official Java Lexical Structure requirements
- `readline.l` demonstrates scanning with readline(3) interactive input
- `yyscanstrings.l` demonstrates string and buffer scanning
- `echo.l` classic interactive scanner to echo input
- `wc.l` word count example
- `wcu.l` word count example with Unicode matching (RE/flex only)
- `wcpp.l` word count example with lexer class variables
- `wcwc.l` word count example with custom lexer class
- `flexexample1.l` example taken from the Flex documentation
- `flexexample2.l` example from <http://matt.might.net/articles/standalone-lexers-with-lex>
- `flexexample3.l` example from the Flex documentation with C bison
- `flexexample3.lxx` example from the Flex documentation with C++ bison
- `flexexample4.l` example from the Flex documentation with C bison-bridge
- `flexexample4.lxx` example from the Flex documentation with C++ bison-bridge
- `flexexample5.l` example from the Flex documentation with C bison-bridge
- `flexexample5.lxx` example from the Flex documentation with C++ bison-bridge
- `flexexample6.l` example reentrant scanner with state stacks
- `flexexample7.l` example from the Flex documentation with C bison-locations
- `flexexample7.lxx` example from the Flex documentation with C++ bison-locations
- `flexexample8.l` example from the Flex documentation with C bison-bridge and bison-locations
- `flexexample8.lxx` example from the Flex documentation with C++ bison-bridge and bison-locations
- `flexexample9.lxx` example Bison 3.0 %skeleton "lalr1.cc" C++ parser with bison-cc and bison-locations
- `flexexample10.l` example from the Flex documentation to use multiple buffers
- `reflexexample11.lxx` example reentrant scanner with Bison 3.2 C++ bison-complete and bison-locations

Regex pattern matching with the easy-to-use RE/flex regex API
-------------------------------------------------------------

- `cards.cpp` RE/flex regex API demo: matches credit card numbers (uses the fast RE/flex regex matcher)
- `cvt2utf` demonstrates `reflex::Input` UTF-8 normalization to convert files to UTF-8/16/32
- `fastsearch.cpp` demonstrates super fast search with regex patterns compiled to C++ (typically much faster than any grep tool)
- `ugrep.cpp` file searcher (simple version), [fully-featured high-performance version on GitHub](https://github.com/Genivia/ugrep)
- `url_boost.cpp` RE/flex regex API demo: capture domain, path, and query keys from URLs (uses the Boost.Regex library)
- `url_pcre2.cpp` RE/flex regex API demo: capture domain, path, and query keys from URLs (uses the PCRE2 library)

[logo-url]: https://www.genivia.com/images/reflex-logo.png
[reflex-url]: https://www.genivia.com/get-reflex.html
[manual-url]: https://www.genivia.com/doc/reflex/html
[bison-url]: http://dinosaur.compilertools.net/#bison
