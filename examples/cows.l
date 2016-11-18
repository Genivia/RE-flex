%option dotall main
%{
#include <iostream>    // std::cout etc.
static int herd = 0;   // a global static variable
%}
cow        \<[Cc]ow\>
%%
{cow}      herd++;
.          // do nothing
<<EOF>>    out() << herd << " cows!" << std::endl;
           return 0;
%%
