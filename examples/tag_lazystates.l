// A lexer to print out all XML tags in a file.
// Uses lazy quantifiers for compact expressions.
// Limitations: does not check UTF-8 encoding validity, cannot handle DTDs.

  #include <stdio.h>
  int level = 0;

%o dotall main

name                    [A-Za-z_:\x80-\xFF][-.0-9A-Za-z_:\x80-\xFF]*
pi                      <\?{name}
comment                 <!--.*?-->
open                    <{name}
close                   <\/{name}>
cdata                   <!\[CDATA\[.*?]]>
string                  \".*?\"|'.*?'

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

<*>.                    /* skip anything else */

%%
