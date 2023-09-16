/* search standard input fast for C/C++ directives such as #define, #include etc. */
/* This search uses a Bloom filter with SIMD (SSE2/AVX2/AVX512/AArch64) */
%{
#include <stdio.h>
%}

/* specify option find to search, not scan */
%o fast find main

directive       ^\h*#(.|\\\r?\n)+\n

%%

{directive}     echo();

%%
