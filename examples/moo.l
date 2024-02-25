// option find generates a search engine to match the specified patterns
%option fast find main

%top{
  #include <iostream>    // std::cout etc.
}

%class{
  int herd;  // lexer class member variable (private by default)
  static const char *moo;
}

%init{
  herd = 0;  // initialize member variable in Lexer class constructor
}

%{
  const char *Lexer::moo = "Moo!"; // static data member must be initialized out of line
%}

cow        \<[Cc]ow\>

%%

{cow}      herd++;       // found a cow, bump count by one
           out() << moo << " ";
<<EOF>>    out() << herd << " cows!" << std::endl; return 0;

%%
