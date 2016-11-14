// A simple lexer to print out all HTML tags.
// Too simple: does not allow / and > in attribute values!

  #include <stdio.h>
  int level = 0;

pi		<!|<\?
comment		<!--([^-]|-[^-]|--+[^->])*-*-->
tag		<[^!/][^/>]*\/>
open		<[^!/][^/>]*>
close		<\/[^>]+>

%o main

%%

{pi}		;
{comment}	;
{tag}		printf("%*s%s\n", level, "", text());
{open}		printf("%*s%s\n", level++, "", text());
{close}		printf("%*s%s\n", --level, "", text());
.|\n		;
<<EOF>>		printf("Tags are %sbalanced\n", level ? "im" : ""); return 9;

%%
