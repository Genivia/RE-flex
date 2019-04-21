%require "3.2"

%language "c++"
%define api.namespace {test}
%define api.parser.class {parser}
%define api.value.type variant
%define api.token.constructor

%define parse.error verbose

%defines
%output "parser.cpp"

%parse-param {test::scanner& lexer}

%locations
%define api.location.file "location.hpp"

%define api.token.prefix {TOK_}
%token <std::string> TOKEN "Token"
%token EOF 0 "End of file"

%code requires {
	namespace test { class scanner; };
}

%code {
	#include "scanner.hpp"
	#undef yylex
	#define yylex lexer.lex
}

%%

rule: TOKEN {std::cout << $1 << std::endl;};

%%

void test::parser::error(const location& loc, const std::string& msg)
{
	std::cerr << loc << ": " << msg << std::endl;
}
