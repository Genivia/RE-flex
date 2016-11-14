// A simple lexer to print out all XML/HTML/XHTML tags in a file.
// Uses lazy quantifiers for compact expressions.

  #include <stdio.h>
  int level = 0;

name                    [A-Za-z_:\x80-\xFF][-.0-9A-Za-z_:\x80-\xFF]*
pi                      <\?{name}
comment                 <!--.*?-->
start                   <{name}
close                   <\/{name}>
cdata                   <!\[CDATA\[.*?]]>
string                  \".*?\"|'.*?'

%o dotall main
%x ATTRIBUTES

%%

{comment}               /* skip comments */
{cdata}                 /* skip CDATA sections */
{pi}                    start(ATTRIBUTES);
{start}                 printf("%*s%s\n", level++, "", text() + 1); start(ATTRIBUTES);
{close}                 matcher().less(size() - 1); printf("%*s%s\n", --level, "", text() + 2);
<<EOF>>                 printf("Tags are %sbalanced\n", level ? "im" : ""); return 0;
<ATTRIBUTES>\/>         --level; start(INITIAL);
<ATTRIBUTES>>           start(INITIAL);
<ATTRIBUTES>{string}    /* skip string value */
<*>.                    ;

%%
