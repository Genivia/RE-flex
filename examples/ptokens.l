// Python tokenizer
// Conforms to the Python 3.x language reference lexical structure:
// https://docs.python.org/3.6/reference/lexical_analysis.html
// Uses RE/flex matcher for lazy quants and for indent/dedent matching.

// inject code into the Lexer class for the scanner state and auxiliary methods
%class{
  // emit token to Lexer's current stream out()
  void emit(const char *token, const char *what = NULL)
  {
    out() << token << (what ? what : "") << std::endl;
  }
  // keep track of (, [, { nesting for implicit line joins
  int join;
}

// import Python 3.x token definitions
%include "pdefs.l"

// implicit line joining is done with the JOINING start condition state
%x JOINING

// use full/fast option for speed and to eliminate tokenizer startup time
%option full

// set indent tabs=8
%option tabs=8

// dot matches \n
%option dotall
// warning: in unicode mode dot is a "catch all" and also matches invalid UTF-8
// so we use {any} to match anything that is valid Unicode/UTF-8, where {any}
// is defined in pdefs.l as all Unicode planes without the surrogate halves:
// {any} stands for \p{Unicode}

%%

^\f?\h*                 // eat nodent margin space

\r?\n                   |
#{any}*?\r?\n           emit("NEWLINE");

^\f?\h+\i               emit("INDENT");

^\f?\h*\j               |
\j                      emit("DEDENT");

(?^\h+)                 // eat space and tabs

(?^^(\f?\h*(#{any}*?)?\r?\n)+)
                        // eat blank lines and comments, anchor ^ in (?^X)

(?^\\\r?\n\f?\h*)       // explicit line joining by eating \ \n ...

[[({]                   // implicit line joining
                        join = 1;
                        matcher().push_stops(); // save the indent stops
                        start(JOINING);
                        emit("DELIMITER  ", text());

<JOINING>{

\s+                     // eat all white space

#{any}*?\r?\n           // eat comments

[[({]                   ++join;
                        emit("DELIMITER  ", text());

[])}]                   if (--join == 0)
                        {
                          matcher().pop_stops(); // restore the indent stops
                          start(INITIAL);
                        }
                        emit("DELIMITER  ", text());

}

<*>{

{stringliteral}         emit("STRING     ", text());

{bytesliteral}          emit("BYTES      ", text());

{integer}               emit("INTEGER    ", text());

{floatnumber}           emit("FLOAT      ", text());

{imagnumber}            emit("IMAG       ", text());

False                   |
None                    |
True                    |
and                     |
as                      |
assert                  |
break                   |
class                   |
continue                |
def                     |
del                     |
elif                    |
else                    |
except                  |
finally                 |
for                     |
from                    |
global                  |
if                      |
import                  |
in                      |
is                      |
lambda                  |
nonlocal                | 
not                     |
or                      |
pass                    |
raise                   |
return                  |
try                     |
while                   |
with                    |
yield                   emit("KEYWORD    ", text());

{identifier}            emit("IDENTIFIER ", text());

"+"                     |
"-"                     |
"*"                     |
"/"                     |
"%"                     |
"&"                     |
"|"                     |
"^"                     |
"~"                     |
"<"                     |
">"                     |
"**"                    |
"//"                    |
"<<"                    |
">>"                    |
"<="                    |
">="                    |
"=="                    |
"!="                    emit("OPERATOR   ", text());

"@"                     |
"->"                    |
"+="                    |
"-="                    |
"*="                    |
"/="                    |
"%="                    |
"@="                    |
"&="                    |
"|="                    |
"^="                    |
">>="                   |
"<<="                   |
"**="                   |
"//="                   |
"="                     |
","                     |
":"                     |
"."                     |
";"                     emit("DELIMITER  ", text());

.                       std::cerr << "Error: invalid input at line " << lineno() << " column " << columno() << std::endl;
                        return 0;

}

%%

int main(int argc, char **argv)
{
  // in this example we'll use the Input class with a FILE or stdin to scan UTF-8/16/32 input
  reflex::Input input;

  if (argc > 1)
  {
    input = fopen(argv[1], "r");
    if (input.file() == NULL)
    {
      perror("Cannot open file for reading");
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    input = stdin;
  }

  Lexer(input).lex();

  if (input.file() != stdin)
    fclose(input.file());

  return 0;
}
