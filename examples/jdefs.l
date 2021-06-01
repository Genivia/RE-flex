// Java lexical structure
// https://docs.oracle.com/javase/specs/jls/se7/html/jls-3.html
// Note: place IntegerLiteral before FloatingPointLiteral since the latter also matches the former

%option unicode

NonASCIICodePoint               \p{Non_ASCII_Unicode}

InputCharacter                  [\x00-\x09\x0B\x0C\x0E-\x7F]|{NonASCIICodePoint}
SingleCharacter                 [\x00-\x09\x0B\x0C\x0E-\x26\x28-\x5B\x5D-\x7F]|{NonASCIICodePoint}
StringCharacter                 [\x00-\x09\x0B\x0C\x0E-\x21\x23-\x5B\x5D-\x7F]|{NonASCIICodePoint}
OctalEscape                     \\([0-7]{1,2}|[0-3][0-7]{2})
EscapeSequence                  \\[btnfr"'\\]|{OctalEscape}
LineTerminator                  \n|\r\n?
WhiteSpace                      [ \t\f]|{LineTerminator}
Sub                             \cZ\z

TraditionalComment              "/*"([^*]|(\*+[^*/]))*\*+\/
EndOfLineComment                "//"[^\r\n]*

Identifier                      \p{JavaIdentifierStart}\p{JavaIdentifierPart}*

IntegerTypeSuffix               [lL]?

DecimalNumeral                  0|[1-9][0-9_]*
DecimalIntegerLiteral           {DecimalNumeral}{IntegerTypeSuffix}?

HexNumeral                      0[xX][[:xdigit:]_]+
HexIntegerLiteral               {HexNumeral}{IntegerTypeSuffix}?

OctalNumeral                    0[0-7_]+
OctalIntegerLiteral             {OctalNumeral}{IntegerTypeSuffix}?

BinaryNumeral                   0[bB][01_]+
BinaryIntegerLiteral            {BinaryNumeral}{IntegerTypeSuffix}?

IntegerLiteral                  {DecimalIntegerLiteral}|{HexIntegerLiteral}|{OctalIntegerLiteral}|{BinaryIntegerLiteral}

FloatTypeSuffix                 [fFdD]
DecimalFloatingPointLiteral     ([0-9][0-9_]*\.?[0-9_]*|\.[0-9_]+)([eE][-+]?[0-9]+)?{FloatTypeSuffix}?
HexadecimalFloatingPointLiteral 0[xX][[:xdigit:]_]*\.[[:xdigit:]_]*[pP][-+]?[0-9]+{FloatTypeSuffix}?
FloatingPointLiteral            {DecimalFloatingPointLiteral}|{HexadecimalFloatingPointLiteral}

CharacterLiteral                '({SingleCharacter}|{EscapeSequence})'

StringLiteral                   \"({StringCharacter}|{EscapeSequence})*\"

BooleanLiteral                  true|false

NullLiteral                     null

Separator                       [][(){};,.]

Operator                        [-+=<>*/&|^%!~?:]|[-=<>!+*/&|^%]=|&&|"||"|"++"|--|<<=?|>>>?=?
