%option dotall main
%{
#include <iostream>    // std::cout etc.
static int n = 0;      // a global static variable
%}
cow        \<[Cc]ow\>
%%
{cow}      n++;
.          // do nothing
<<EOF>>    out() << n << " cows!" << std::endl;
           return 0;
%%
