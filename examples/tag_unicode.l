// A lexer to print out all XML tags in a file.
// Uses lazy quantifiers for compact expressions.
// Checks Unicode/UTF-8 encoding validity.
// Limitations: cannot handle DTDs.

  #include <stdio.h>
  int level = 0;

%o dotall main unicode

dot                     \p{Unicode}
name                    [A-Za-z_:\p{Non_ASCII_Unicode}][-.0-9A-Za-z_:\p{Non_ASCII_Unicode}]*
pi                      <\?{name}
comment                 <!--{dot}*?-->
open                    <{name}
close                   <\/{name}>
cdata                   <!\[CDATA\[{dot}*?]]>
string                  \"{dot}*?\"|'{dot}*?'

%x ATTRIBUTES

%%

{comment}               |
{cdata}                 /* skip comments and CDATA sections */

{pi}                    start(ATTRIBUTES);

{open}                  printf("%*s%s\n", level++, "", text() + 1);
                        start(ATTRIBUTES);

{close}                 matcher().less(size() - 1);
                        printf("%*s%s\n", --level, "", text() + 2);

<<EOF>>                 printf("Tags are %sbalanced\n", level ? "im" : "");
                        return 0;

<ATTRIBUTES>"/>"        --level;
                        start(INITIAL);

<ATTRIBUTES>"?>"        |
<ATTRIBUTES>">"         start(INITIAL);

<ATTRIBUTES>{name}      |
<ATTRIBUTES>{string}    /* skip attribute names and strings */

<*>{dot}                /* skip anything else */

<*>.                    fprintf(stderr, "Invalid XML encoding\n");
                        return 0;

%%
