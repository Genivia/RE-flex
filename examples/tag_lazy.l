// A simple lexer to print out all HTML tags.
// Uses lazy quantifiers for compact expressions.
// Too simple: does not allow > in attribute values!

  #include <stdio.h>
  int level = 0;

pi		<\?.*?\?>
comment		<!--.*?-->
tag		<[^/][^>]*?\/>
open		<[^/][^/]*?>
close		<\/.+?>

%o dotall main

%%

{pi}		;
{comment}	;
{tag}		printf("%*s%s\n", level, "", text());
{open}		printf("%*s%s\n", level++, "", text());
{close}		printf("%*s%s\n", --level, "", text());
.		;
<<EOF>>		printf("Tags are %sbalanced\n", level ? "im" : ""); return 0;

%%
