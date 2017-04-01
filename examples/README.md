[![logo][logo-url]][reflex-url]

The examples with Bison require the installation of [Bison][bison-url].

Flex scanner examples
---------------------

- `ctokens.l` and `cdefs.l` a C/C++ tokenizer
- `jtokens.l` and `jdefs.l` a Java tokenizer, meets Java Lexical Structure requirements
- `flexexample1.l` example taken from the Flex documentation
- `flexexample2.l` example from <http://matt.might.net/articles/standalone-lexers-with-lex>
- `flexexample3.l` example from the Flex documentation with bison
- `flexexample4.l` example from the Flex documentation with bison-bridge
- `flexexample5.l` example from the Flex documentation with bison-bridge
- `flexexample6.l` example reentrant scanner with state stacks
- `flexexample7.l` example from the Flex documentation with bison-locations
- `flexexample8.l` example from the Flex documentation with bison-bridge and bison-locations
- `wc.l` word count example
- `wcu.l` word count example with Unicode matching (RE/flex only)
- `wcpp.l` word count example with lexer class variables
- `wcwc.l` word count example with custom lexer class
- `echo.l` interactive scanner to echo input
- `readline.l` interactive scanner to echo input using readline (requires readline)
- `json.l` JSON parser (verified with <http://seriot.ch/parsing_json.php>)

RE/flex scanner examples
------------------------

- `ptokens.l` and `pdefs.l` a Python tokenizer
- `calc.l` and `calc.y` calculator, supports Unicode identifier variables
- `cow.l` scanner to convert cows
- `cows.l` scanner to convert cows, uses word boundary anchors
- `indent.l` indentation detection and reformatting example
- `braille.l` a braille translator, inspired by the re2c example but expanded to be useful
- `reflexexample3.l` example modified from the Flex documentation with bison
- `reflexexample4.l` example modified from the Flex documentation with bison-bridge
- `reflexexample5.l` example modified from the Flex documentation with bison-bridge
- `reflexexample6.l` example reentrant scanner with state stacks
- `tag.l` simple lexer to print out all XML/HTML tags
- `tag_lazy.l` simple lexer to print XML/HTML tags, uses lazy quants
- `tag_lazystates.l` lexer to print XML tags, uses lazy quants and states
- `tag_unicode.l` lexer to print XML tags, uses lazy quants and states, checks UTF-8 validity
- `tag_tidy.l` lexer to tidy and normalize HTML

Stand-alone C++ regex matching example
--------------------------------------

- `cards.cpp` matches credit card numbers
- `url.cpp` capture domain, path, and query keys from URLs (requires Boost.Regex for captures)

[logo-url]: https://www.genivia.com/images/reflex-logo.png
[reflex-url]: https://www.genivia.com/get-reflex.html
[manual-url]: https://www.genivia.com/doc/reflex/html
[bison-url]: http://dinosaur.compilertools.net/#bison
