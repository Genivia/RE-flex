    // simple unix-like wc (word count) with lexer inherited from yyFlexLexer
    #include <iostream>
    #include <iomanip>
    using namespace std;

    class wc : yyFlexLexer {
     public:
      int run();
     private:
      int ch;
      int wd;
      int nl;
    };

nl  \r?\n
wd  [^ \t\r\n]

%option yyclass=wc
%option lex=run
%option main

%%

{nl}	ch += yyleng; ++nl;
{wd}+	ch += yyleng; ++wd;
.	++ch;
<<EOF>>	*yyout << setw(8) << nl << setw(8) << wd << setw(8) << ch << endl; yyterminate();

%%
