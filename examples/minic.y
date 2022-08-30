// Mini C parser by Robert van Engelen
// A simple one-pass, syntax-directed translation of mini C to JVM bytecode
// Requires minic.l, minic.y, minic.hpp
// See minicdemo.c for a description of the mini C features
//
// Ideas for compiler improvements (from easy to hard, roughly):
// - add more library functions that compile to JVM virtual and static method invocations
// - allow polymorphic functions instead of generating redeclaration errors
// - add constant folding, i.e. Expr class contains U4, F8, and std::string constants to combine
// - add variable declaration initializers, e.g. int a=1; and/or implicit initialization, requires init method to init statics
// - allow variable declarations anywhere in code blocks, not just at the start of a function body 
// - add structs/classes (structs without methods or classes with methods)
// - add exceptions, e.g. 'try' and 'catch' blocks

%require "3.2"
%skeleton "lalr1.cc"
%language "c++"

// parser class: yy::Parser
%define api.namespace {yy}
%define api.parser.class {Parser}

// use C++ variants and generate yy::Parser::make_<TOKEN> constructors
%define api.value.type variant
%define api.token.constructor

// verbose error messages, reported with yy::Parser::error()
%define parse.error verbose

// generate parser.hpp
%defines

// generate parser.cpp
%output "parser.cpp"

// enable bison locations and generate location.hpp
%locations
%define api.location.file "location.hpp"

// pass Scanner lexer, Compiler comp, and error objects to yy::Parser::parse(), to use in semantic actions
%parse-param {yy::Scanner& lexer}
%parse-param {Compiler& comp}
%parse-param {size_t& errors}

// generate yy::Parser::token::TOKEN_<name> token constants
%define api.token.prefix {TOKEN_}

// we expect one shift-reduce conflict, for the ambiguous if-else
%expect 1

// tokens with semantic values
%token <ID> ID "identifier";
%token <U8> U8 "integer constant";
%token <F8> F8 "float constant";
%token <CS> CS "string literal";
%token EOF  0  "end of file";

%token
  BREAK    "break"
  CASE     "case"
  CONTINUE "continue"
  DEFAULT  "default"
  DO       "do"
  ELSE     "else"
  FALSE    "false"
  FLOAT    "float"
  FOR      "for"
  IF       "if"
  INT      "int"
  MAIN     "main"
  NEW      "new"
  PRINT    "print"
  PRINTLN  "println"
  RETURN   "return"
  STRING   "string"
  SWITCH   "switch"
  TRUE     "true"
  VOID     "void"
  WHILE    "while"
  PA       "+="
  NA       "-="
  TA       "*="
  DA       "/="
  MA       "%="
  AA       "&="
  XA       "^="
  OA       "|="
  LA       "<<="
  RA       ">>="
  OR       "||"
  AN       "&&"
  EQ       "=="
  NE       "!="
  LE       "<="
  GE       ">="
  LS       "<<"
  RS       ">>"
  PP       "++"
  NN       "--"
  AR       "->"
;

%token '!' '#' '$' '%' '&' '(' ')' '*' '+' ',' '-' '.' '/' ':' ';' '<' '=' '>' '?' '[' ']' '^' '{' '|' '}' '~'

// operator precedence and associativity (includes ')' for casts and '[' for array indexing)
%right     '=' PA NA TA DA MA AA XA OA LA RA
%right     '?'
%right     ':'
%left      OR
%left      AN
%left      '|'
%left      '^'
%left      '&'
%left      EQ NE LE '<' GE '>'
%left      LS RS
%left      '+' '-'
%left      '*' '/' '%'
%right     '!' '~' ')'
%nonassoc  '#' '$' PP NN
%left      '[' '.' AR

// the code to include with parser.hpp
%code requires {
  #include "minic.hpp"
  namespace yy {
    class Scanner;
  }
}

// the code to include with parser.cpp
%code {
  #include "scanner.hpp"
  #include <iostream>
  // within yy::Parser::parse() we should invoke lexer.lex() as yylex()
  #undef yylex
  #define yylex lexer.lex
}

// nonterminals with Type semantic values
%type <Type> type list args optargs exprs optexprs

// nonterminals with Proto semantic values
%type <Proto> proto

// nonterminals with Expr semantic values
%type <Expr> expr cond optcond

// for print or println functions
%type <CS> print prints

// marker nonterminals with U4 address semantic values
%type <U4> A B C G

%%

program : statics       {
                          delete comp.table[comp.scope];
                        }
        ;

statics : statics func
        | statics decl
        | /* empty */   {
                          comp.table[++comp.scope] = new Table();
                        }
        ;

func    : main '(' ')' block
                        {
                          // make sure we return from main()
                          comp.emit(return_);
                          // construct a static method "main"
                          ID id = lexer.symbol("main");
                          TD type = comp.type_function("[Ljava/lang/String;", "V");
                          Table *table = comp.table[comp.scope];
                          comp.new_method(ACC_PUBLIC | ACC_STATIC, id, type, table->locals, 256);
                          delete table;
                          if (!comp.table[--comp.scope]->enter_func(id, type, false))
                            error(@1, "redefined main");
                        }
        | proto block   {
                          // make sure we return (compiler does not check if functions return a value)
                          comp.emit(return_);
                          // constuct a static function
                          comp.new_method(ACC_PUBLIC | ACC_STATIC, $1.id, $1.type, comp.table[comp.scope]->locals, 256);
                          delete comp.table[comp.scope--];
                          if (!comp.table[comp.scope]->enter_func($1.id, $1.type, false))
                            error(@2, "redefined function");
                        }
        | proto ';'     {
                          delete comp.table[comp.scope--];
                        }
        ;

proto   : type ID '(' S optargs ')'
                        {
                          $$.type = comp.type_function($5.type, $1.type);
                          if (!comp.table[comp.scope - 1]->enter_func($2, $$.type, true))
                            error(@2, "redefined function");
                          $$.id = $2;
                        }

main    : type MAIN     {
                          // start parsing the main() function, create a local scope table
                          Table *table = new Table(comp.table[comp.scope]);
                          comp.table[++comp.scope] = table;
                          // start with offset 1 for the locals in main() since 0 is used for the argument array
                          table->locals = 1;
                          // to emit code in a buffer, copy the code to the method later
                          comp.new_method_code();
                          // this is the main() function
                          comp.main = true;
                          // main() may return int or void
                          if (!comp.type_is_void($1.type) && !comp.type_is_int($1.type))
                            error(@1, "main can only return an int or void");
                          // the function's return type
                          comp.return_type = $1.type;
                        }
        ;

S       : /* empty */   {
                          // start parsing the scope of a function, create a local scope table
                          Table *table = new Table(comp.table[comp.scope]);
                          comp.table[++comp.scope] = table;
                          // to emit code in a buffer, copy the code to the method later
                          comp.new_method_code();
                          // not the main() function
                          comp.main = false;
                          // the function's return type
                          comp.return_type = comp.decl_type;
                        }
        ;

block   : '{' decls stmts '}'
        ;

decls   : decls decl
        | /* empty */
        ;

decl    : list ';'
        ;

type    : VOID          {
                          $$.type = comp.decl_type = comp.type_void();
                        }
        | INT           {
                          $$.type = comp.decl_type = comp.type_int();
                        }
        | FLOAT         {
                          $$.type = comp.decl_type = comp.type_double();
                        }
        | STRING        {
                          $$.type = comp.decl_type = comp.type_string();
                        }
        | type '[' ']'  {
                          // array types can be declared but arrays are not fully implemented
                          $$.type = comp.decl_type = comp.type_array($1.type);
                        }
        ;

list    : list ',' ID   {
                          if (comp.scope == 0)
                          {
                            // add a static variable, stored as a static field
                            if (!comp.table[0]->enter($3, $1.type, comp.new_field(ACC_STATIC, $3, $1.type)))
                              error(@3, "redefined static variable");
                          }
                          else
                          {
                            // add a local variable, stored in the method frame (double takes two frame slots)
                            if (!comp.table[comp.scope]->enter($3, $1.type, comp.table[comp.scope]->locals))
                              error(@3, "redefined variable");
                            comp.table[comp.scope]->locals += 1 + comp.type_is_double($1.type);
                          }
                          $$ = $1;
                        }
        | type ID       {
                          if (comp.scope == 0)
                          {
                            // add a static variable, stored as a static field
                            if (!comp.table[0]->enter($2, $1.type, comp.new_field(ACC_STATIC, $2, $1.type)))
                              error(@2, "redefined static variable");
                          }
                          else
                          {
                            // add a local variable, stored in the method frame (double takes two frame slots)
                            if (!comp.table[comp.scope]->enter($2, $1.type, comp.table[comp.scope]->locals))
                              error(@2, "redefined variable");
                            comp.table[comp.scope]->locals += 1 + comp.type_is_double($1.type);
                          }
                          $$ = $1;
                        }
        ;

args    : args ',' type ID
                        {
                          // add an argument variable, stored in the method frame (double takes two frame slots)
                          if (!comp.table[comp.scope]->enter($4, $3.type, comp.table[comp.scope]->locals))
                            error(@4, "redefined argument");
                          comp.table[comp.scope]->locals += 1 + comp.type_is_double($3.type);
                          // concat argument types to produce the JVM type descriptor of the method, e.g. "II" in "(II)I"
                          $$.type = comp.type_concat($1.type, $3.type);
                        }
        | type ID       {
                          // add an argument variable, stored in the method frame (double takes two frame slots)
                          if (!comp.table[comp.scope]->enter($2, $1.type, comp.table[comp.scope]->locals))
                            error(@2, "redefined argument");
                          comp.table[comp.scope]->locals += 1 + comp.type_is_double($1.type);
                          $$ = $1;
                        }
        ;

optargs : args          {
                          $$ = $1;
                        }
        | /* empty */   {
                          $$.type = comp.type_none();
                        }
        ;

stmts   : stmts stmt
        | /* empty */
        ;

stmt    : IF '(' cond ')' stmt
                        {
                          comp.backpatch($3.truelist, $3.after);
                          comp.backpatch($3.falselist, comp.addr());
                        }
        | IF '(' cond ')' stmt ELSE G A stmt
                        {
                          comp.backpatch($3.truelist, $3.after);
                          comp.backpatch($3.falselist, $8);
                          comp.backpatch($7, comp.addr());
                        }
        | switch G '{' cases '}' G
                        {
                          comp.backpatch($2, comp.addr());
                          // generate switch lookup table and backpatch the break instruction jumps
                          comp.switch_done();
                          comp.backpatch($6, comp.addr());
                        }
        | WHILE '(' C cond ')' B stmt G
                        {
                          comp.backpatch($4.truelist, $6);
                          comp.backpatch($4.falselist, comp.addr());
                          comp.backpatch($8, $3);
                          // backpatch the break and continue goto instruction jumps
                          comp.loop_done();
                        }
        | DO B stmt WHILE '(' C cond ')' ';'
                        {
                          comp.backpatch($7.truelist, $2);
                          comp.backpatch($7.falselist, comp.addr());
                          // backpatch the break and continue goto instruction jumps
                          comp.loop_done();
                        }
        | FOR '(' optexpr ';' C optcond ';' A optexpr G ')' B stmt G
                        {
                          comp.backpatch($6.truelist, $12);
                          comp.backpatch($6.falselist, comp.addr());
                          comp.backpatch($10, $5);
                          comp.backpatch($14, $8);
                          // backpatch the break and continue goto instruction jumps
                          comp.loop_done();
                        }
        | RETURN expr ';'
                        {
                          TD type = comp.rvalue($2);
                          if (comp.main)
                          {
                            if (!comp.type_is_int(comp.return_type))
                              error(@2, "return type error");
                            else if (!comp.coerce($2, comp.return_type))
                              error(@2, "type error");
                            comp.emit3(invokestatic, comp.pool_add_Method("java/lang/System", "exit", "(I)V"));
                          }
                          else
                          {
                            if (!comp.coerce($2, comp.return_type))
                              error(@2, "type error");
                            else if (comp.type_is_int(comp.return_type))
                              comp.emit(ireturn);
                            else if (comp.type_is_double(type))
                              comp.emit(freturn);
                            else
                              comp.emit(areturn);
                          }
                        }
        | RETURN ';'    {
                          if (comp.main)
                          {
                            if (!comp.type_is_void(comp.return_type))
                              error(@1, "return requires a value");
                            comp.emit(iconst_0);
                            comp.emit3(invokestatic, comp.pool_add_Method("java/lang/System", "exit", "(I)V"));
                          }
                          else
                          {
                            if (!comp.type_is_void(comp.return_type))
                              error(@1, "return requires a value");
                            comp.emit(return_);
                          }
                        }
        | prints ';'    {
                          comp.emit(pop);
                        }
        | BREAK ';'     {
                          if (!comp.emit_break())
                            error(@1, "break not in loop or switch");
                        }
        | CONTINUE ';'  {
                          if (!comp.emit_continue())
                            error(@1, "continue not in loop");
                        }
        | '{' stmts '}'
        | optexpr ';'
        | error ';'     {
                          // synchronize on ; to continue parsing
                          yyerrok;
                        }
        ;

switch  : SWITCH '(' expr ')'
                        {
                          if (!comp.type_is_int(comp.rvalue($3)))
                            error(@3, "type error");
                          comp.switch_init();
                        }
        ;

cases   : cases case ':' stmts
        | /* empty */
        ;

case    : CASE U8       {
                          if ($2 > 0x7fffffff)
                            error(@2, "integer constant out of range");
                          else if (!comp.emit_case(static_cast<U4>($2), comp.addr()))
                            error(@2, "duplicate case value");
                        }
        | CASE '-' U8   {
                          if ($3 > 0x80000000)
                            error(@3, "integer constant out of range");
                          else if (!comp.emit_case(static_cast<U4>(-$3), comp.addr()))
                            error(@2 + @3, "duplicate case value");
                        }
        | DEFAULT       {
                          if (!comp.emit_default(comp.addr()))
                            error(@1, "duplicate default");
                        }
        ;

prints  : prints ',' D expr
                        {
                          TD type = comp.rvalue($4);
                          if (comp.type_is_int(type))
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", $1, "(I)V"));
                          else if (comp.type_is_double(type))
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", $1, "(D)V"));
                          else if (comp.type_is_string(type))
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", $1, "(Ljava/lang/String;)V"));
                          else
                            error(@4, "type error");
                          $$ = $1;
                        }
        | print expr    {
                          TD type = comp.rvalue($2);
                          if (comp.type_is_int(type))
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", $1, "(I)V"));
                          else if (comp.type_is_double(type))
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", $1, "(D)V"));
                          else if (comp.type_is_string(type))
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", $1, "(Ljava/lang/String;)V"));
                          else
                            error(@2, "type error");
                          $$ = $1;
                        }
        ;

print   : PRINT         {
                          comp.emit3(getstatic, comp.pool_add_Field("java/lang/System", "out", "Ljava/io/PrintStream;"));
                          comp.emit(dup);
                          $$ = "print";
                        }
        | PRINTLN       {
                          comp.emit3(getstatic, comp.pool_add_Field("java/lang/System", "out", "Ljava/io/PrintStream;"));
                          comp.emit(dup);
                          $$ = "println";
                        }
        ;

exprs   : exprs ',' expr
                        {
                          // concat argument types to produce the JVM type descriptor of the method, e.g. "II" in "(II)I"
                          $$.type = comp.type_concat($1.type, comp.rvalue($3));
                        }
        | expr          {
                          $$.type = comp.rvalue($1);
                        }
        ;

optexprs: exprs         {
                          $$ = $1;
                        }
        | /* empty */   {
                          $$ = comp.type_none();
                        }
        ;

expr    : expr '=' expr {
                          if (!comp.emit_assign($1, $3, nop, nop, $$))
                            error(@1 + @3, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr PA expr  {
                          if (!comp.emit_assign($1, $3, iadd, dadd, $$))
                            error(@1 + @3, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr NA expr  {
                          if (!comp.emit_assign($1, $3, isub, dsub, $$))
                            error(@1 + @3, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr TA expr  {
                          if (!comp.emit_assign($1, $3, imul, dmul, $$))
                            error(@1 + @3, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr DA expr  {
                          if (!comp.emit_assign($1, $3, idiv, ddiv, $$))
                            error(@1 + @3, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr MA expr  {
                          if (!comp.emit_assign($1, $3, irem, nop, $$))
                            error(@1 + @3, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr AA expr  {
                          if (!comp.emit_assign($1, $3, iand, nop, $$))
                            error(@1 + @3, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr XA expr  {
                          if (!comp.emit_assign($1, $3, ixor, nop, $$))
                            error(@1 + @3, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr OA expr  {
                          if (!comp.emit_assign($1, $3, ior, nop, $$))
                            error(@1 + @3, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr LA expr  {
                          if (!comp.emit_assign($1, $3, ishl, nop, $$))
                            error(@1 + @3, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr RA expr  {
                          if (!comp.emit_assign($1, $3, ishr, nop, $$))
                            error(@1 + @3, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr '?' expr ':' expr
                        {
                          U4 offset = comp.circuit($1);
                          comp.backpatch($1.truelist, $1.after);
                          comp.adjust($3, offset);
                          comp.adjust($5, offset);
                          if (comp.type_is_circuit($3.type) && comp.type_is_circuit($5.type))
                          {
                            $$.truelist = comp.merge($3.truelist, $5.truelist);
                            $$.falselist = comp.merge($3.falselist, $5.falselist);
                          }
                          else if (comp.type_is_circuit($3.type))
                          {
                            comp.circuit($5);
                            $$.truelist = comp.merge($3.truelist, $5.truelist);
                            $$.falselist = comp.merge($3.falselist, $5.falselist);
                          }
                          else if (comp.type_is_circuit($5.type))
                          {
                            comp.adjust($5, comp.circuit($3));
                            $$.truelist = comp.merge($3.truelist, $5.truelist);
                            $$.falselist = comp.merge($3.falselist, $5.falselist);
                          }
                          else
                          {
                            if (comp.type_equal($3.type, $5.type))
                            {
                              $$.type = $3.type;
                            }
                            else
                            {
                              $$.type = comp.type_wider($3.type, $5.type);
                              if ($$.type == NULL)
                                error(@3 + @5, "type error");
                              comp.coerce($5, $$.type);
                              comp.coerce($3, $$.type);
                            }
                            comp.insert3($3.after, goto_, 0);
                            comp.backpatch($3.after - 3, comp.addr());
                          }
                          comp.backpatch($1.falselist, $3.after);
                          $$.after = comp.addr();
                        }
        | expr OR expr  {
                          comp.adjust($3, comp.circuit($1));
                          comp.circuit($3);
                          $$.truelist = comp.merge($1.truelist, $3.truelist);
                          comp.backpatch($1.falselist, $1.after);
                          $$.falselist = $3.falselist;
                          $$.after = comp.addr();
                        }
        | expr AN expr  {
                          comp.adjust($3, comp.circuit($1));
                          comp.circuit($3);
                          $$.falselist = comp.merge($1.falselist, $3.falselist);
                          comp.backpatch($1.truelist, $1.after);
                          $$.truelist = $3.truelist;
                          $$.after = comp.addr();
                        }
        | expr '|' expr {
                          if (!comp.emit_oper($1, $3, ior, nop, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr '^' expr {
                          if (!comp.emit_oper($1, $3, ixor, nop, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr '&' expr {
                          if (!comp.emit_oper($1, $3, iand, nop, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr EQ expr  {
                          if (!comp.emit_compare($1, $3, ifeq, if_icmpeq, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr NE expr  {
                          if (!comp.emit_compare($1, $3, ifne, if_icmpne, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr GE expr  {
                          if (!comp.emit_compare($1, $3, ifge, if_icmpge, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr '<' expr {
                          if (!comp.emit_compare($1, $3, iflt, if_icmplt, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr LE expr  {
                          if (!comp.emit_compare($1, $3, ifle, if_icmple, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr '>' expr {
                          if (!comp.emit_compare($1, $3, ifgt, if_icmpgt, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr LS expr  {
                          if (!comp.emit_oper($1, $3, ishl, nop, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr RS expr  {
                          if (!comp.emit_oper($1, $3, ishr, nop, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr '+' expr {
                          if (!comp.emit_oper($1, $3, iadd, dadd, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr '-' expr {
                          if (!comp.emit_oper($1, $3, isub, dsub, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr '*' expr {
                          if (!comp.emit_oper($1, $3, imul, dmul, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr '/' expr {
                          if (!comp.emit_oper($1, $3, idiv, ddiv, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | expr '%' expr {
                          if (!comp.emit_oper($1, $3, irem, nop, $$))
                            error(@1 + @3, "type error");
                          $$.after = comp.addr();
                        }
        | '!' expr      {
                          comp.circuit($2);
                          $$.truelist = $2.falselist;
                          $$.falselist = $2.truelist;
                          $$.after = comp.addr();
                        }
        | '~' expr      {
                          $$.type = comp.rvalue($2);
                          if (!comp.type_is_int($$.type))
                            error(@1 + @2, "type error");
                          comp.emit(iconst_m1);
                          comp.emit(ixor);
                          $$.after = comp.addr();
                        }
        | '+' expr %prec '!'
                        {
                          $$.type = comp.rvalue($2);
                          if (!comp.type_is_int($$.type) && !comp.type_is_double($$.type))
                            error(@1 + @2, "type error");
                          $$.after = comp.addr();
                        }
        | '-' expr %prec '!'
                        {
                          $$.type = comp.rvalue($2);
                          if (comp.type_is_int($$.type))
                            comp.emit(ineg);
                          else if (comp.type_is_double($$.type))
                            comp.emit(dneg);
                          else
                            error(@1 + @2, "type error");
                          $$.after = comp.addr();
                        }
        | '(' expr ')'  {
                          $$ = $2;
                        }
        | '#' expr      {
                          TD type = comp.rvalue($2);
                          if (comp.type_is_int(type))
                            comp.emit3(invokestatic, comp.pool_add_Method("java/lang/Integer", "bitCount", "(I)I"));
                          else if (comp.type_is_string(type))
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/lang/String", "length", "()I"));
                          else if (comp.type_is_array(type))
                            comp.emit(arraylength);
                          else
                            error(@1 + @2, "type error");
                          $$.type = comp.type_int();
                          $$.after = comp.addr();
                        }
        | '#' '$'       {
                          if (!comp.main)
                            error(@2, "invalid use of $");
                          comp.emit(aload_0);
                          comp.emit(arraylength);
                          $$.type = comp.type_int();
                          $$.after = comp.addr();
                        }
        | '$' expr      {
                          if (!comp.main)
                            error(@1, "invalid use of $");
                          if (!comp.type_is_int(comp.rvalue($2)))
                            error(@1 + @2, "type error");
                          comp.emit(iconst_1);
                          comp.emit(isub);
                          comp.emit(aload_0);
                          comp.emit(swap);
                          comp.emit(aaload);
                          $$.type = comp.type_string();
                          $$.after = comp.addr();
                        }
        | PP expr       {
                          if (!comp.emit_update($2, true, true, $$))
                            error(@2, "not assignable");
                          $$.after = comp.addr();
                        }
        | NN expr       {
                          if (!comp.emit_update($2, true, false, $$))
                            error(@2, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr PP       {
                          if (!comp.emit_update($1, false, true, $$))
                            error(@1, "not assignable");
                          $$.after = comp.addr();
                        }
        | expr NN       {
                          if (!comp.emit_update($1, false, false, $$))
                            error(@1, "not assignable");
                          $$.after = comp.addr();
                        }
        | '(' type ')' expr
                        {
                          if (!comp.coerce($4, $2.type))
                            error(@4, "type error");
                          $$ = $4;
                        }
        | expr '[' expr ']'
                        {
                          if (!comp.type_is_int(comp.rvalue($3)))
                            error(@3, "type error");
                          TD type = comp.rvalue($1);
                          if (comp.type_is_string(type))
                          {
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/lang/String", "charAt", "(I)C"));
                            $$.type = comp.type_int();
                          }
                          else if (comp.type_is_array(type))
                          {
                            $$.type = comp.type_of_element(type);
                            $$.mode = Expr::ARRAY;
                          }
                          else
                          {
                            error(@1, "type error");
                          }
                          $$.after = comp.addr();
                        }
        | expr '.' ID   {
                          error(@2, "not implemented");
                          $$ = $1;
                        }
        | expr AR ID    {
                          error(@2, "invalid operation");
                          $$ = $1;
                        }
        | ID '(' optexprs ')'
                        {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                          {
                            const Library *lib = comp.library($1, $3.type);
                            if (lib != NULL)
                            {
                              // emit virtual or static method invocation of the library function
                              if (lib->virtype != NULL)
                                comp.emit3(invokevirtual, comp.pool_add_Method(lib->package, lib->method, lib->type));
                              else
                                comp.emit3(invokestatic, comp.pool_add_Method(lib->package, lib->method, lib->type));
                              // boolean and char are ints, computationally so "Z" and "C" return types are OK to use as "I"
                              $$.type = comp.type_of_return(lib->type);
                              if (comp.type_is_boolean($$.type) || comp.type_is_char($$.type))
                                $$.type = comp.type_int();
                            }
                            else
                            {
                              error(@1, "undefined function or type error in arguments");
                            }
                          }
                          else if (!comp.type_is_function(entry->type))
                          {
                            error(@1, "not a function");
                          }
                          else if (comp.type_check_args($3.type, entry->type))
                          {
                            // invoke a compiled function
                            comp.emit3(invokestatic, comp.pool_add_Method($1->c_str(), entry->type));
                            $$.type = comp.type_of_return(entry->type);
                          }
                          else
                          {
                            error(@3, "type error in arguments");
                          }
                          $$.after = comp.addr();
                        }
        | NEW type '[' expr ']'
                        {
                          if (!comp.type_is_int(comp.rvalue($4)))
                            error(@4, "type error");
                          if (comp.type_is_int($2.type))
                            comp.emit2(newarray, T_INT);
                          else if (comp.type_is_double($2.type))
                            comp.emit2(newarray, T_DOUBLE);
                          else
                            comp.emit3(anewarray, comp.pool_add_Class(comp.type_of_reference($2.type)));
                          $$.type = comp.type_array($2.type);
                          $$.after = comp.addr();
                        }
        | ID            {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry != NULL)
                          {
                            if (comp.type_is_function(entry->type))
                              error(@1, "not a variable");
                            $$.type = entry->type;
                            $$.mode = entry->table->scope > 0 ? Expr::LOCAL : Expr::STATIC;
                            $$.place = entry->place;
                          }
                          else
                          {
                            error(@1, "undefined name");
                          }
                          $$.after = comp.addr();
                        }
        | U8            {
                          if ($1 == 0)
                            comp.emit(iconst_0);
                          else if ($1 == 1)
                            comp.emit(iconst_1);
                          else if ($1 == 2)
                            comp.emit(iconst_2);
                          else if ($1 == 3)
                            comp.emit(iconst_3);
                          else if ($1 == 4)
                            comp.emit(iconst_4);
                          else if ($1 == 5)
                            comp.emit(iconst_5);
                          else if ($1 <= 0x7f)
                            comp.emit2(bipush, static_cast<U1>($1));
                          else if ($1 <= 0x7fff)
                            comp.emit3(sipush, static_cast<U2>($1));
                          else if ($1 <= 0x7fffffff)
                            comp.emit_ldc(comp.pool_add_Integer(static_cast<U4>($1)));
                          else
                            error(@1, "integer constant out of range");
                          $$.type = comp.type_int();
                          $$.after = comp.addr();
                        }
        | F8            {
                          if ($1 == 0.0)
                            comp.emit(dconst_0);
                          else if ($1 == 1.0)
                            comp.emit(dconst_1);
                          else
                            comp.emit3(ldc2_w, comp.pool_add_Double($1));
                          $$.type = comp.type_double();
                          $$.after = comp.addr();
                        }
        | CS            {
                          comp.emit_ldc(comp.pool_add_String($1));
                          $$.type = comp.type_string();
                          $$.after = comp.addr();
                        }
        | FALSE         {
                          $$.falselist = comp.backpatch_list_addr();
                          comp.emit3(goto_, 0);
                          $$.after = comp.addr();
                        }
        | TRUE          {
                          $$.truelist = comp.backpatch_list_addr();
                          comp.emit3(goto_, 0);
                          $$.after = comp.addr();
                        }
        ;

optexpr : expr          {
                          if (comp.type_is_circuit($1.type))
                          {
                            comp.backpatch($1.truelist, comp.addr());
                            comp.backpatch($1.falselist, comp.addr());
                          }
                          else if (!comp.type_is_void($1.type))
                          {
                            if ($1.mode == Expr::VALUE)
                            {
                              if (comp.type_is_double($1.type))
                                comp.emit(pop2);
                              else
                                comp.emit(pop);
                            }
                            else if ($1.mode == Expr::ARRAY)
                            {
                              comp.emit(pop2);
                            }
                          }
                        }
        | /* empty */
        ;

cond    : expr          {
                          comp.circuit($1);
                          $$ = $1;
                        }
        ;

optcond : cond          {
                          $$ = $1;
                        }
        | /* empty */   {
                          $$.truelist = comp.backpatch_list_addr();
                          comp.emit3(goto_, 0);
                          $$.after = comp.addr();
                        }
        ;

A       : /* empty */   {
                          $$ = comp.addr();
                        }
        ;

B       : /* empty */   {
                          comp.break_init();
                          $$ = comp.addr();
                        }
        ;

C       : /* empty */   {
                          comp.continue_init();
                          $$ = comp.addr();
                        }
        ;

D       : /* empty */   {
                          comp.emit(dup);
                        }
        ;

G       : /* empty */   {
                          $$ = comp.addr();
                          comp.emit3(goto_, 0);
                        }
        ;

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
  yy::Scanner lexer(file);

  // with bison-complete and bison-locations we can set the filename to display with syntax errors
  lexer.filename = argv[1];

  // the class name is the basename of the filename without path and extension suffix
  const char *s = strrchr(argv[1], '/');
  if (s == NULL)
    s = argv[1];
  else
    ++s;
  const char *e = strrchr(s, '.');
  if (e == NULL)
    e = s + strlen(s);
  std::string name(s, e - s);

  // construct a compiler for the class
  Compiler comp(name);

  // keep track of the number of errors reported with yy:Parser::error()
  size_t errors = 0;

  // construct a parser, pass the lexer, compiler and errors to use in the semantic actions
  yy::Parser parser(lexer, comp, errors);

  // parse and compile the source into a JVM class file
  if (parser.parse() || errors > 0)
  {
    std::cerr << "Compilation errors: class file " << name << ".class not saved\n";
    exit(EXIT_FAILURE);
  }

  // save the JVM class file
  comp.save();

  if (file != stdin)
    fclose(file);

  exit(EXIT_SUCCESS);
}

// display error and location in the source code
void yy::Parser::error(const location& loc, const std::string& msg)
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
      yy::Scanner::Matcher *m = lexer.new_matcher(file); // new matcher
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
