// C/C++ lexical structure w/o unicode
// Requires RE/flex matcher or a Perl/ECMA matcher that supports lazy quants

WHITESPACE	[[:space:]]+
ILCOMMENT	"//"([^\n]|\\\n)*\n
MLCOMMENT	"/*"(.|\n)*?"*/"
DIRECTIVE	^[[:blank:]]*#(.|\\\r?\n)+
NAME		[[:alpha:]_][[:alnum:]_]*
UDEC		0|[1-9]\d*
UHEX		0[Xx][[:xdigit:]]+
UOCT		0[0-7]+
UINT		({UDEC}|{UHEX})([Ll]{0,2}[Uu]?|[Uu][Ll]{0,2})
UFLT		\d*(\d|\.\d|\d\.)\d*([Ee][-+]?\d+)?[FfLl]?
CHAR		L?'(\\.|[^\\\n'])*'
STRING		L?\"(\\.|\\\r?\n|[^\\\n"])*\"
