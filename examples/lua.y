// Lua 5.3 Bison parser and transpiler by Robert van Engelen
// Eliminated reduce/reduce conflict by expanding the prefixexp nonterminal
// Expanded the repetition nonterminal
// Renamed long nonterminal names functiondef->function, functioncall->funccall, tableconstructor->table
// Requires lua.l, lua.y, lua.hpp

%require "3.2"
%skeleton "lalr1.cc"
%language "c++"

// parser class: yy::LuaParser
%define api.namespace {yy}
%define api.parser.class {LuaParser}

// use C++ variants and generate yy::LuaParser::make_<TOKEN> constructors
%define api.value.type variant
%define api.token.constructor

// verbose error messages, reported with yy::LuaParser::error()
%define parse.error verbose

// generate LuaParser.hpp
%defines

// generate LuaParser.cpp
%output "LuaParser.cpp"

// enable bison locations and generate location.hpp
%locations
%define api.location.file "location.hpp"

// pass LuaScanner lexer, Transpiler transpiler, and error objects to yy::LuaParser::parse(), to use in semantic actions
%parse-param {yy::LuaScanner& lexer}
%parse-param {Transpiler& transpiler}
%parse-param {size_t& errors}

// generate yy::LuaParser::token::TOKEN_<name> token constants
%define api.token.prefix {TOKEN_}

// the code to include with LuaParser.hpp
%code requires {
  #include "lua.hpp"
  namespace yy {
    class LuaScanner;
  }
}

// the code to include with LuaParser.cpp
%code {
  #include "LuaScanner.hpp"
  // within yy::LuaParser::parse() we should invoke lexer.lex() as yylex()
  #undef yylex
  #define yylex lexer.lex
}

// tokens with semantic values
%token <Name>           NAME    "name";
%token <IntegerLiteral> INTEGER "integer constant";
%token <FloatLiteral>   FLOAT   "floating point constant";
%token <StringLiteral>  STRING  "string literal";
%token EOF              0       "end of file";

%token
  AND       "and"
  BREAK     "break"
  DO        "do"
  ELSE      "else"
  ELSEIF    "elseif"
  END       "end"
  FALSE     "false"
  FOR       "for"
  FUNCTION  "function"
  GOTO      "goto"
  IF        "if"
  IN        "in"
  LOCAL     "local"
  NIL       "nil"
  NOT       "not"
  OR        "or"
  REPEAT    "repeat"
  RETURN    "return"
  THEN      "then"
  TRUE      "true"
  UNTIL     "until"
  WHILE     "while"
  EQU       "=="
  NEQ       "~="
  LTE       "<="
  GTE       ">="
  CAT       ".."
  SHL       "<<"
  SHR       ">>"
  DIV       "//"
  DOTS      "..."
  COLS      "::"
;

%token '#' '%' '&' '(' ')' '*' '+' ',' '-' '.' '/' ':' ';' '<' '=' '>' '[' ']' '^' '{' '|' '}' '~'

// operator precedence and associativity
%left       OR
%left       AND
%left       EQU NEQ LTE '<' GTE '>'
%left       '|'
%left       '~'
%left       '&'
%left       SHL SHR
%right      CAT
%left       '+' '-'
%left       '*' '/' DIV '%'
%right      NOT '#'
%right      '^'

// we expect four shift-reduce conflicts due to Lua's optional semicolon, correct to shift on '('
%expect 4

%type <Transpiler::List<Transpiler::Statement>*>        block scope statlist
%type <Transpiler::Statement*>                          stat binding laststat
%type <Transpiler::List<Transpiler::Expression>*>       explist1 setlist args
%type <Transpiler::Expression*>                         exp var
%type <Transpiler::List<Transpiler::Condition>*>        conds condlist
%type <Transpiler::Condition*>                          cond
%type <Transpiler::Lambda*>                             function funcbody
%type <Transpiler::Parameters*>                         params parlist
%type <Transpiler::Call*>                               funccall
%type <Transpiler::Range*>                              explist23
%type <Transpiler::FunctionName*>                       funcname
%type <Transpiler::NameList*>                           dottedname namelist
%type <Name>                                            label
%type <Transpiler::Table*>                              table
%type <Transpiler::List<Transpiler::Field>*>            fieldlist
%type <Transpiler::Field*>                              field

%%

chunk       : block                             { transpiler.chunk = $1; }

semi        : ';'
            |

block       : scope statlist                    { $$ = $1->concat($2); }
            | scope statlist laststat semi      { $$ = $1->concat($2)->add($3); }

scope       :                                   { $$ = new Transpiler::List<Transpiler::Statement>(); }
            | scope statlist binding semi       { $$ = $1->concat($2)->add($3); }

statlist    :                                   { $$ = new Transpiler::List<Transpiler::Statement>(); }
            | statlist stat semi                { $$ = $1->add($2); }

stat        : DO block END                      { $$ = new Transpiler::Block($2); }
            | IF conds END                      { $$ = new Transpiler::If($2); }
            | WHILE exp DO block END            { $$ = new Transpiler::While($2, $4); }
            | REPEAT block UNTIL exp            { $$ = new Transpiler::Until($2, $4); }
            | FOR NAME '=' explist23 DO block END       { $$ = new Transpiler::ForCounter($2, $4, $6); }
            | FOR namelist IN explist1 DO block END     { $$ = new Transpiler::ForIterator($2, $4, $6); }
            | FUNCTION funcname funcbody        { $$ = new Transpiler::Function($2, $3); }
            | GOTO NAME                         { $$ = new Transpiler::Goto($2); }
            | label                             { $$ = new Transpiler::Label($1); }
            | setlist '=' explist1              { $$ = new Transpiler::Assign($1, $3); }
            | funccall                          { $$ = new Transpiler::FunctionCall($1); }
            | error DO                          { yyerrok; }
            | error IF                          { yyerrok; }
            | error WHILE                       { yyerrok; }
            | error FOR                         { yyerrok; }
            | error REPEAT                      { yyerrok; }
            | error FUNCTION                    { yyerrok; }
            | error GOTO                        { yyerrok; }
            | error COLS                        { yyerrok; }
            | error LOCAL                       { yyerrok; }
            | error BREAK                       { yyerrok; }
            | error RETURN                      { yyerrok; }
            | error END                         { yyerrok; }
            | error ';'                         { yyerrok; }

conds       : condlist                          { $$ = $1; }
            | condlist ELSE block               { $$ = $1->add(new Transpiler::Condition(new Transpiler::True(), $3)); }

condlist    : cond                              { $$ = new Transpiler::List<Transpiler::Condition>($1); }
            | condlist ELSEIF cond              { $$ = $1->add($3); }

cond        : exp THEN block                    { $$ = new Transpiler::Condition($1, $3); }

laststat    : BREAK                             { $$ = new Transpiler::Break(); }
            | RETURN                            { $$ = new Transpiler::Return(new Transpiler::List<Transpiler::Expression>()); }
            | RETURN explist1                   { $$ = new Transpiler::Return($2); }

label       : COLS NAME COLS                    { $$ = $2; }

binding     : LOCAL namelist                    { $$ = new Transpiler::Local($2, new Transpiler::List<Transpiler::Expression>()); }
            | LOCAL namelist '=' explist1       { $$ = new Transpiler::Local($2, $4); }
            | LOCAL FUNCTION NAME funcbody      { $$ = new Transpiler::LocalFunction($3, $4); }

funcname    : dottedname                        { $$ = new Transpiler::FunctionName($1, false); }
            | dottedname ':' NAME               { $$ = new Transpiler::FunctionName($1->add($3), true); }

dottedname  : NAME                              { $$ = new Transpiler::NameList($1); }
            | dottedname '.' NAME               { $$ = $1->add($3); }

namelist    : NAME                              { $$ = new Transpiler::NameList($1); }
            | namelist ',' NAME                 { $$ = $1->add($3); }

explist1    : exp                               { $$ = new Transpiler::List<Transpiler::Expression>($1); }
            | explist1 ',' exp                  { $$ = $1->add($3); }

explist23   : exp ',' exp                       { $$ = new Transpiler::Range($1, $3, NULL); }
            | exp ',' exp ',' exp               { $$ = new Transpiler::Range($1, $3, $5); }

exp         : NIL                               { $$ = new Transpiler::Nil(); }
            | TRUE                              { $$ = new Transpiler::True(); }
            | FALSE                             { $$ = new Transpiler::False(); }
            | INTEGER                           { $$ = new Transpiler::Integer($1); }
            | FLOAT                             { $$ = new Transpiler::Float($1); }
            | STRING                            { $$ = new Transpiler::String($1); }
            | DOTS                              { $$ = new Transpiler::Dots(); }
            | function                          { $$ = $1; }
            | table                             { $$ = $1; }
            | var                               { $$ = $1; }
            | funccall                          { $$ = $1; }
            | NOT exp                           { $$ = new Transpiler::UnaryOp("not", $2); }
            | '#' exp                           { $$ = new Transpiler::UnaryOp("#", $2); }
            | '-' exp %prec NOT                 { $$ = new Transpiler::UnaryOp("-", $2); }
            | '~' exp %prec NOT                 { $$ = new Transpiler::UnaryOp("~", $2); }
            | exp OR exp                        { $$ = new Transpiler::Op("or", $1, $3); }
            | exp AND exp                       { $$ = new Transpiler::Op("and", $1, $3); }
            | exp '<' exp                       { $$ = new Transpiler::Op("<", $1, $3); }
            | exp LTE exp                       { $$ = new Transpiler::Op("<=", $1, $3); }
            | exp '>' exp                       { $$ = new Transpiler::Op(">", $1, $3); }
            | exp GTE exp                       { $$ = new Transpiler::Op(">=", $1, $3); }
            | exp EQU exp                       { $$ = new Transpiler::Op("==", $1, $3); }
            | exp NEQ exp                       { $$ = new Transpiler::Op("~=", $1, $3); }
            | exp '|' exp                       { $$ = new Transpiler::Op("|", $1, $3); }
            | exp '~' exp                       { $$ = new Transpiler::Op("~", $1, $3); }
            | exp '&' exp                       { $$ = new Transpiler::Op("&", $1, $3); }
            | exp SHL exp                       { $$ = new Transpiler::Op("<<", $1, $3); }
            | exp SHR exp                       { $$ = new Transpiler::Op(">>", $1, $3); }
            | exp CAT exp                       { $$ = new Transpiler::Op("..", $1, $3); }
            | exp '+' exp                       { $$ = new Transpiler::Op("+", $1, $3); }
            | exp '-' exp                       { $$ = new Transpiler::Op("-", $1, $3); }
            | exp '*' exp                       { $$ = new Transpiler::Op("*", $1, $3); }
            | exp '/' exp                       { $$ = new Transpiler::Op("/", $1, $3); }
            | exp DIV exp                       { $$ = new Transpiler::Op("//", $1, $3); }
            | exp '%' exp                       { $$ = new Transpiler::Op("%", $1, $3); }
            | exp '^' exp                       { $$ = new Transpiler::Op("^", $1, $3); }
            | '(' exp ')'                       { $$ = $2; }

setlist     : var                               { $$ = new Transpiler::List<Transpiler::Expression>($1); }
            | setlist ',' var                   { $$ = $1->add($3); }

var         : NAME                              { $$ = new Transpiler::Variable($1); }
            | var '[' exp ']'                   { $$ = new Transpiler::Index($1, $3); }
            | var '.' NAME                      { $$ = new Transpiler::Member($1, $3); }
            | funccall '[' exp ']'              { $$ = new Transpiler::Index($1, $3); }
            | funccall '.' NAME                 { $$ = new Transpiler::Member($1, $3); }
            | '(' exp ')' '[' exp ']'           { $$ = new Transpiler::Index($2, $5); }
            | '(' exp ')' '.' NAME              { $$ = new Transpiler::Member($2, $5); }

funccall    : var args                          { $$ = new Transpiler::Call($1, $2, NULL); }
            | var ':' NAME args                 { $$ = new Transpiler::Call($1, $4, $3); }
            | funccall args                     { $$ = new Transpiler::Call($1, $2, NULL); }
            | funccall ':' NAME args            { $$ = new Transpiler::Call($1, $4, $3); }
            | '(' exp ')' args                  { $$ = new Transpiler::Call($2, $4, NULL); }
            | '(' exp ')' ':' NAME args         { $$ = new Transpiler::Call($2, $6, $5); }

args        : '(' ')'                           { $$ = new Transpiler::List<Transpiler::Expression>(); }
            | '(' explist1 ')'                  { $$ = $2; }
            | table                             { $$ = new Transpiler::List<Transpiler::Expression>($1); }
            | STRING                            { $$ = new Transpiler::List<Transpiler::Expression>(new Transpiler::String($1)); }

function    : FUNCTION funcbody                 { $$ = $2; }

funcbody    : params block END                  { $$ = new Transpiler::Lambda($1, $2); }

params      : '(' parlist ')'                   { $$ = $2; }

parlist     :                                   { $$ = new Transpiler::Parameters(new Transpiler::NameList(), false); }
            | namelist                          { $$ = new Transpiler::Parameters($1, false); }
            | DOTS                              { $$ = new Transpiler::Parameters(new Transpiler::NameList(), true); }
            | namelist ',' DOTS                 { $$ = new Transpiler::Parameters($1, true); }

table       : '{' '}'                           { $$ = new Transpiler::Table(new Transpiler::List<Transpiler::Field>()); }
            | '{' fieldlist '}'                 { $$ = new Transpiler::Table($2); }
            | '{' fieldlist ',' '}'             { $$ = new Transpiler::Table($2); }
            | '{' fieldlist ';' '}'             { $$ = new Transpiler::Table($2); }

fieldlist   : field                             { $$ = new Transpiler::List<Transpiler::Field>($1); }
            | fieldlist ',' field               { $$ = $1->add($3); }
            | fieldlist ';' field               { $$ = $1->add($3); }

field       : exp                               { $$ = new Transpiler::Field(NULL, $1); }
            | NAME '=' exp                      { $$ = new Transpiler::Field(new Transpiler::Variable($1), $3); }
            | '[' exp ']' '=' exp               { $$ = new Transpiler::Field($2, $5); }

%%

int main(int argc, char **argv)
{
  FILE *file;

  // open the specified source code file for reading
  if (argc <= 1 || (file = fopen(argv[1], "r")) == NULL)
  {
    std::cerr << "Cannot open file for reading\n";
    exit(EXIT_FAILURE);
  }

  // construct a lexer taking input from the specified file encoded in UTF-8/16/32
  yy::LuaScanner lexer(file);

  // with bison-complete and bison-locations we can set the filename to display with syntax errors
  lexer.filename = argv[1];

  // basename of the filename without path and extension suffix
  const char *s = strrchr(argv[1], '/');
  if (s == NULL)
    s = argv[1];
  else
    ++s;
  const char *e = strrchr(s, '.');
  if (e == NULL)
    e = s + strlen(s);
  std::string name(s, e - s);

  Transpiler transpiler(name);

  // keep track of the number of errors reported with yy:LuaParser::error()
  size_t errors = 0;

  // construct a parser, pass the lexer, transpiler and errors to use in the semantic actions
  yy::LuaParser parser(lexer, transpiler, errors);

  // parse and transpile
  if (parser.parse() || errors > 0)
  {
    std::cerr << "Compilation errors\n";
    exit(EXIT_FAILURE);
  }

  if (file != stdin)
    fclose(file);

  // save the transpilation
  if (transpiler.save())
    printf("Saved %s.lisp\n", name.c_str());
  else
    printf("Cannot save\n");

  exit(EXIT_SUCCESS);
}

// display error and location in the source code
void yy::LuaParser::error(const location& loc, const std::string& msg)
{
  ++errors;

  std::cerr << loc << ": " << msg << std::endl;
  if (loc.begin.line == loc.end.line && loc.begin.line == static_cast<int>(lexer.lineno()))
  {
    // the error is on the current line and spans only one line
    std::cerr << lexer.matcher().line() << std::endl;
    for (int i = 0; i < loc.begin.column; ++i)
      std::cerr << " ";
    for (int i = loc.begin.column; i <= loc.end.column; ++i)
      std::cerr << "~";
    std::cerr << std::endl;
  }
  else
  {
    // the error is not on the current line or spans multiple lines
    FILE *file = lexer.in().file(); // the current file being scanned
    if (file != NULL)
    {
      yy::LuaScanner::Matcher *m = lexer.new_matcher(file); // new matcher
      lexer.push_matcher(m); // save the current matcher
      off_t pos = ftell(file); // save current position in the file
      fseek(file, 0, SEEK_SET); // go to the start of the file
      for (int i = 1; i < loc.begin.line; ++i)
        m->skip('\n'); // skip to the next line
      for (int i = loc.begin.line; i <= loc.end.line; ++i)
      {
        std::cerr << m->line() << std::endl; // display offending line
        m->skip('\n'); // next line
      }
      fseek(file, pos, SEEK_SET); // restore position in the file to continue scanning
      lexer.pop_matcher(); // restore matcher
    }
  }
  if (lexer.size() == 0) // if token is unknown (no match)
    lexer.matcher().winput(); // skip character
}
