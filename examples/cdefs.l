// C/C++ lexical structure w/o unicode
// Requires RE/flex matcher or a Perl/ECMA matcher that supports lazy quants

WHITESPACE	[[:space:]]+
ILCOMMENT	"//"([^\n]|\\\n)*\n
MLCOMMENT	"/*"(.|\n)*?"*/"
DIRECTIVE	^[[:blank:]]*#([^\n]|\\\r?\n)+
NAME		[[:alpha:]_][[:alnum:]_]*
UFLT		\d*(\d|\.\d|\d\.)\d*([Ee][-+]?\d+)?[FfLl]?
UOCT		0[0-7]+
UHEX		0[Xx][[:xdigit:]]+
UDEC		0|[1-9]\d*
UINT		({UOCT}|{UHEX}|{UDEC})([Ll]{0,2}[Uu]?|[Uu][Ll]{0,2})
CHAR		L?'(\\.|[^\\\n'])*'
STRING		L?\"(\\.|\\\r?\n|[^\\\n"])*\"
