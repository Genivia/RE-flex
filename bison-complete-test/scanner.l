%option bison-complete
%option bison-cc
%option bison-cc-namespace=test
%option bison-cc-parser=parser
%option bison-locations

%option exception="test.syntax_error"

%option tabs=8

%option fast
%option unicode
%option freespace

%option lexer=scanner
%option outfile="scanner.cpp"
%option header-file="scanner.hpp"

%option namespace=test

%top{
	#include "parser.hpp"
	#include "location.hpp"
%}

%%

\p{Alpha}+		{ return test::parser::make_TOKEN(str(), location()); }
[[:blank:]\n]+	{ }

<<EOF>>			{ return test::parser::make_EOF(location()); }

%%
