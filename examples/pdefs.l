// Python 3.x lexical structure
// https://docs.python.org/3.6/reference/lexical_analysis.html
// Requires RE/flex matcher for lazy quants
// Enables unicode matching

%option matcher=reflex unicode

any                     \p{Unicode}

identifier              \p{PythonIdentifierStart}\p{PythonIdentifierPart}*

stringprefix            (r|u|R|U|f|F|fr|Fr|fR|FR|rf|rF|Rf|RF)
qshortchar              [^\\\n']|\\{any}
dshortchar              [^\\\n"]|\\{any}
longchar                [^\\]|\\{any}
shortstring             '{qshortchar}*'|\"{dshortchar}*\"
longstring              '''{longchar}*?'''|\"\"\"{longchar}*?\"\"\"
stringliteral           {stringprefix}?({shortstring}|{longstring})

bytesprefix             (b|B|br|Br|bR|BR|rb|rB|Rb|RB)
qshortbyte              []-\x7f\x00-&(-[]|\\\p{ASCII}
dshortbyte              []-\x7f\x00-!#-[]|\\\p{ASCII}
longbyte                [^\\]|\\\p{ASCII}
shortbytes              '{qshortbyte}*'|\"{dshortbyte}*\"
longbytes               '''{longbyte}*?'''|\"\"\"{longbyte}*?\"\"\"
bytesliteral            {bytesprefix}({shortbytes}|{longbytes})

octdigit                [0-7]
hexdigit                [0-9|A-F|a-f]
bindigit                [01]
nonzerodigit            [1-9]
digit                   [0-9]
decimalinteger          {nonzerodigit}{digit}*|0+
octinteger              0[oO]{octdigit}+
hexinteger              0[xX]{hexdigit}+
bininteger              0[bB]{bindigit}+
integer                 {decimalinteger}|{octinteger}|{hexinteger}|{bininteger}

intpart                 {digit}+
fraction                \.{digit}+
exponent                [eE][-+]{digit}+
pointfloat              {intpart}?{fraction}|{intpart}\.
exponentfloat           ({intpart}|{pointfloat}){exponent}
floatnumber             {pointfloat}|{exponentfloat}

imagnumber              ({floatnumber}|{intpart})[jJ]
