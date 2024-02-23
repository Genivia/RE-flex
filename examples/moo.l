%option dotall main

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
.          // do nothing
<<EOF>>    out() << herd << " cows!" << std::endl; return 0;

%%
