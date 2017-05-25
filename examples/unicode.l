// very simple demonstration of Unicode pattern matching with RE/flex
// skips Unicode space, prints Unicode words, other characters as U+XXXX codes
%option fast main nodefault unicode
%%
\s+          // skip spacing
\w+          { std::cout << text() << "\n"; }
\p{Unicode}  { std::cout << "U+" << std::hex << wstr().at(0) << " \n"; }
.            { std::cerr << "invalid Unicode!\n"; }
%%
