/* search standard input for C/C++ directives such as #define, #include etc. */
%{
#include <stdio.h>
%}

%o fast find main

directive       ^\h*#(.|\\\r?\n)+\n

%%

{directive}     echo();

%%
