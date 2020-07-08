// Mini C parser by Robert van Engelen
// A simple one-pass, syntax-directed translation of mini C to JVM bytecode
// Requires minic.l, minic.y, minic.hpp
//
// Ideas for compiler improvements (from easy to hard, roughly):
// - add more library functions that compile to JVM virtual and static method invocations
// - allow polymorphic functions instead of generating redeclaration errors
// - add variable declaration initializers, e.g. int a=1; and/or implicit initialization
// - add type casts, e.g. (int)x or int(x), requires changing the grammar to avoid sr conflicts
// - add arrays, e.g. int a[]; or perhaps int[] a; requires adding expression lvalues to the grammar
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

// verbose error messages, reported with yy::Parser::syntax_error()
%define parse.error verbose

// generate parser.hpp
%defines

// generate parser.cpp
%output "parser.cpp"

// enable bison locations and generate location.hpp
%locations
%define api.location.file "location.hpp"

// pass Scanner and Compiler objects to yy::Parser::parse(), to use lexer and comp in semantic actions
%parse-param {yy::Scanner& lexer}
%parse-param {Compiler& comp}

// generate yy::Parser::token::TOKEN_<name> token constants
%define api.token.prefix {TOKEN_}

// we expect on shift-reduce conflict (for the if-else)
%expect 1

// tokens with semantic values
%token <ID> ID "Identifier";
%token <U8> U8 "Integer";
%token <F8> F8 "Float";
%token <CS> CS "String";
%token EOF  0  "End of file";

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
  PRINT    "print"
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

// precedence and associativity
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
%right     '!' '~'
%left      '[' '.' AR
%nonassoc  '#' '$' PP NN

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

// nonterminals with Expr semantic values
%type <Expr> expr cond optcond

// nonterminals with ID semantic values
%type <ID> id

// marker nonterminals with U4 address semantic values
%type <U4> switch B C L G

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
                            error(@1, "Redefined main");
                        }
        | type ID '(' A optargs ')' block
                        {
                          // make sure we return (compiler does not check if functions return a value)
                          comp.emit(return_);
                          // constuct a static function
                          TD type = comp.type_function($5.type, $1.type);
                          Table *table = comp.table[comp.scope];
                          comp.new_method(ACC_PUBLIC | ACC_STATIC, $2, type, table->locals, 256);
                          delete table;
                          if (!comp.table[--comp.scope]->enter_func($2, type, false))
                            error(@2, "Redefined function");
                        }
        | type ID '(' A optargs ')' ';'
                        {
                          // add function declaration to the table of statics (static scope 0)
                          delete comp.table[comp.scope];
                          if (!comp.table[--comp.scope]->enter_func($2, comp.type_function($5.type, $1.type), true))
                            error(@2, "Redefined function");
                        }
        ;

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

A       : /* empty */   {
                          // start parsing a function, create a local scope table
                          Table *table = new Table(comp.table[comp.scope]);
                          comp.table[++comp.scope] = table;
                          // to emit code in a buffer, copy the code to the method later
                          comp.new_method_code();
                          // not the main() function
                          comp.main = false;
                          // the function's return type
                          comp.return_type = comp.type;
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
                          $$.type = comp.type = comp.type_void();
                        }
        | INT           {
                          $$.type = comp.type = comp.type_int();
                        }
        | FLOAT         {
                          $$.type = comp.type = comp.type_double();
                        }
        | STRING        {
                          $$.type = comp.type = comp.type_string();
                        }
        | type '[' ']'  {
                          // array types can be declared but arrays are not fully implemented
                          $$.type = comp.type = comp.type_array($1.type);
                        }
        ;

list    : list ',' ID   {
                          bool ok;
                          if (comp.scope == 0)
                          {
                            // add a static variable, stored as a static field
                            ok = comp.table[0]->enter($3, $1.type, comp.new_field(ACC_STATIC, $3, $1.type));
                          }
                          else
                          {
                            // add a local variable, stored in the method frame (double takes two frame slots)
                            ok = comp.table[comp.scope]->enter($3, $1.type, comp.table[comp.scope]->locals);
                            comp.table[comp.scope]->locals += 1 + comp.type_is_double($1.type);
                          }
                          if (!ok)
                            error(@3, "Redefined variable");
                          $$ = $1;
                        }
        | type ID       {
                          bool ok;
                          if (comp.scope == 0)
                          {
                            // add a static variable, stored as a static field
                            ok = comp.table[0]->enter($2, $1.type, comp.new_field(ACC_STATIC, $2, $1.type));
                          }
                          else
                          {
                            // add a local variable, stored in the method frame (double takes two frame slots)
                            ok = comp.table[comp.scope]->enter($2, $1.type, comp.table[comp.scope]->locals);
                            comp.table[comp.scope]->locals += 1 + comp.type_is_double($1.type);
                          }
                          if (!ok)
                            error(@2, "Redefined variable");
                          $$ = $1;
                        }
        ;

args    : args ',' type ID
                        {
                          // add an argument variable, stored in the method frame (double takes two frame slots)
                          if (!comp.table[comp.scope]->enter($4, $3.type, comp.table[comp.scope]->locals))
                            error(@4, "Redefined argument");
                          comp.table[comp.scope]->locals += 1 + comp.type_is_double($3.type);
                          // concat argument types to produce the JVM type descriptor of the method, e.g. "II" in "(II)I"
                          $$.type = comp.type_concat($1.type, $3.type);
                        }
        | type ID       {
                          // add an argument variable, stored in the method frame (double takes two frame slots)
                          if (!comp.table[comp.scope]->enter($2, $1.type, comp.table[comp.scope]->locals))
                            error(@2, "Redefined argument");
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

stmt    : IF '(' cond ')' L stmt
                        {
                          comp.backpatch($3.truelist, $5);
                          comp.backpatch($3.falselist, comp.addr());
                        }
        | IF '(' cond ')' L stmt ELSE G L stmt
                        {
                          comp.backpatch($3.truelist, $5);
                          comp.backpatch($3.falselist, $9);
                          comp.backpatch($8, comp.addr());
                        }
        | switch G '{' cases '}' G
                        {
                          comp.backpatch($2, comp.addr());
                          // generate switch lookup table and backpatch break instruction jumps
                          comp.switch_done();
                          comp.backpatch($6, comp.addr());
                        }
        | WHILE '(' C cond ')' B stmt G
                        {
                          comp.backpatch($4.truelist, $6);
                          comp.backpatch($4.falselist, comp.addr());
                          comp.backpatch($8, $3);
                          // backpatch break and continue goto instruction jumps
                          comp.loop_done();
                        }
        | DO B stmt WHILE '(' C cond ')' ';'
                        {
                          comp.backpatch($7.truelist, $2);
                          comp.backpatch($7.falselist, comp.addr());
                          // backpatch break and continue goto instruction jumps
                          comp.loop_done();
                        }
        | FOR '(' optexpr ';' C optcond ';' L optexpr G ')' B stmt G
                        {
                          comp.backpatch($6.truelist, $12);
                          comp.backpatch($6.falselist, comp.addr());
                          comp.backpatch($10, $5);
                          comp.backpatch($14, $8);
                          // backpatch break and continue goto instruction jumps
                          comp.loop_done();
                        }
        | RETURN expr ';'
                        {
                          TD type = comp.decircuit($2);
                          if (comp.main)
                          {
                            if (comp.type_is_int(comp.return_type))
                            {
                              if (comp.coerce($2, comp.return_type))
                                comp.emit3(invokestatic, comp.pool_add_Method("java/lang/System", "exit", "(I)V"));
                              else
                                error(@2, "Return type error");
                            }
                            else
                            {
                              error(@2, "Return type error");
                            }
                          }
                          else
                          {
                            if (!comp.coerce($2, comp.return_type))
                              error(@2, "Type error");
                            else if (comp.type_is_int(comp.return_type))
                              comp.emit(ireturn);
                            else if (comp.type_is_double(type))
                              comp.emit(freturn);
                            else if (comp.type_is_string(type) || comp.type_is_array(type))
                              comp.emit(areturn);
                          }
                        }
        | RETURN ';'
                        {
                          if (comp.main)
                          {
                            if (comp.type_is_void(comp.return_type))
                            {
                              comp.emit(iconst_0);
                              comp.emit3(invokestatic, comp.pool_add_Method("java/lang/System", "exit", "(I)V"));
                            }
                            else
                            {
                              error(@1, "Return requires a value");
                            }
                          }
                          else
                          {
                            if (comp.type_is_void(comp.return_type))
                              comp.emit(return_);
                            else
                              error(@1, "Return requires a value");
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
                          yyerrok;
                        }
        ;

switch  : SWITCH '(' expr ')'
                        {
                          if (!comp.type_is_int(comp.decircuit($3)))
                            error(@3, "Type error");
                          comp.switch_init();
                        }
        ;

cases   : cases case ':' stmts
        | /* empty */
        ;

case    : CASE U8       {
                          if ($2 > 0x7fffffff)
                            error(@2, "Integer constant out of range");
                          else if (!comp.emit_case(static_cast<U4>($2), comp.addr()))
                            error(@2, "Duplicate case value");
                        }
        | CASE '-' U8   {
                          if ($3 > 0x80000000)
                            error(@3, "Integer constant out of range");
                          else if (!comp.emit_case(static_cast<U4>(-$3), comp.addr()))
                            error(@2 + @3, "Duplicate case value");
                        }
        | DEFAULT       {
                          if (!comp.emit_default(comp.addr()))
                            error(@1, "Duplicate default");
                        }
        ;

prints  : prints ',' D expr
                        {
                          TD type = comp.decircuit($4);
                          if (comp.type_is_int(type))
                          {
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", "print", "(I)V"));
                          }
                          else if (comp.type_is_double(type))
                          {
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", "print", "(D)V"));
                          }
                          else if (comp.type_is_string(type))
                          {
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", "print", "(Ljava/lang/String;)V"));
                          }
                          else
                          {
                            error(@4, "Type error");
                          }
                        }
        | print expr    {
                          TD type = comp.decircuit($2);
                          if (comp.type_is_int(type))
                          {
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", "print", "(I)V"));
                          }
                          else if (comp.type_is_double(type))
                          {
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", "print", "(D)V"));
                          }
                          else if (comp.type_is_string(type))
                          {
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/io/PrintStream", "print", "(Ljava/lang/String;)V"));
                          }
                          else
                          {
                            error(@2, "Type error");
                          }
                        }
        ;

print   : PRINT         {
                          comp.emit3(getstatic, comp.pool_add_Field("java/lang/System", "out", "Ljava/io/PrintStream;"));
                          comp.emit(dup);
                        }
        ;

exprs   : exprs ',' expr
                        {
                          // concat argument types to produce the JVM type descriptor of the method, e.g. "II" in "(II)I"
                          $$.type = comp.type_concat($1.type, comp.decircuit($3));
                        }
        | expr          {
                          $$.type = comp.decircuit($1);
                        }
        ;

optexprs: exprs         {
                          $$ = $1;
                        }
        | /* empty */   {
                          $$ = comp.type_none();
                        }
        ;

expr    : ID   '=' expr {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                            error(@1, "Undefined name");
                          bool ok;
                          $$ = comp.emit_asg(entry, $3, nop, nop, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | id   PA  expr {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                            error(@1, "Undefined name");
                          bool ok;
                          $$ = comp.emit_asg(entry, $3, iadd, dadd, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | id   NA  expr {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                            error(@1, "Undefined name");
                          bool ok;
                          $$ = comp.emit_asg(entry, $3, isub, dsub, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | id   TA  expr {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                            error(@1, "Undefined name");
                          bool ok;
                          $$ = comp.emit_asg(entry, $3, imul, dmul, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | id   DA  expr {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                            error(@1, "Undefined name");
                          bool ok;
                          $$ = comp.emit_asg(entry, $3, idiv, ddiv, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | id   MA  expr {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                            error(@1, "Undefined name");
                          bool ok;
                          $$ = comp.emit_asg(entry, $3, irem, nop, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | id   AA  expr {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                            error(@1, "Undefined name");
                          bool ok;
                          $$ = comp.emit_asg(entry, $3, iand, nop, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | id   XA  expr {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                            error(@1, "Undefined name");
                          bool ok;
                          $$ = comp.emit_asg(entry, $3, ixor, nop, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | id   OA  expr {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                            error(@1, "Undefined name");
                          bool ok;
                          $$ = comp.emit_asg(entry, $3, ior, nop, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | id   LA  expr {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                            error(@1, "Undefined name");
                          bool ok;
                          $$ = comp.emit_asg(entry, $3, ishl, nop, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | id   RA  expr {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                            error(@1, "Undefined name");
                          bool ok;
                          $$ = comp.emit_asg(entry, $3, ishr, nop, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr '?' G L expr ':' G L expr
                        {
                          BackpatchList *bl = NULL;
                          if (comp.type_is_circuit($1.type))
                          {
                            comp.backpatch($1.truelist, $4);
                            comp.backpatch($1.falselist, $8);
                          }
                          else
                          {
                            bl = comp.backpatch_addr();
                            comp.emit3(goto_, 0);
                            comp.backpatch($3, comp.addr());
                            bool ok;
                            Expr cond = comp.circuit($1, ok);
                            if (!ok)
                              error(@1, "Type error");
                            comp.backpatch(cond.truelist, $4);
                            comp.backpatch(cond.falselist, $8);
                          }
                          comp.backpatch($7, comp.addr());
                          if (comp.type_is_circuit($5.type) && comp.type_is_circuit($9.type))
                          {
                            $$.truelist = comp.merge($5.truelist, $9.truelist);
                            $$.falselist = comp.merge($5.falselist, $9.falselist);
                          }
                          else if (comp.type_is_circuit($5.type))
                          {
                            bool ok;
                            Expr expr = comp.circuit($9, ok);
                            if (!ok)
                              error(@9, "Type error");
                            $$.truelist = comp.merge($5.truelist, expr.truelist);
                            $$.falselist = comp.merge($5.falselist, expr.falselist);
                          }
                          else if (comp.type_is_circuit($9.type))
                          {
                            bool ok;
                            Expr expr = comp.circuit($5, ok);
                            if (!ok)
                              error(@5, "Type error");
                            $$.truelist = comp.merge(expr.truelist, $9.truelist);
                            $$.falselist = comp.merge(expr.falselist, $9.falselist);
                          }
                          else if (comp.type_equal($5.type, $9.type))
                          {
                            $$.type = $5.type;
                          }
                          else
                          {
                            error(@5 + @9, "Type mismatch error");
                          }
                          if (bl != NULL)
                            comp.backpatch(bl, comp.addr());
                          else
                            comp.backpatch($3, comp.addr());
                        }
        | expr OR L expr
                        {
                          if (comp.type_is_circuit($1.type) && comp.type_is_circuit($4.type))
                          {
                            $$.truelist = comp.merge($1.truelist, $4.truelist);
                            comp.backpatch($1.falselist, $3);
                            $$.falselist = $4.falselist;
                          }
                          else if (comp.type_is_circuit($1.type))
                          {
                            bool ok;
                            Expr expr = comp.circuit($4, ok);
                            if (!ok)
                              error(@4, "Type error");
                            $$.truelist = comp.merge($1.truelist, expr.truelist);
                            comp.backpatch($1.falselist, $3);
                            $$.falselist = expr.falselist;
                          }
                          else if (comp.type_is_circuit($4.type))
                          {
                            bool ok;
                            Expr expr = comp.circuit($1, ok);
                            if (!ok)
                              error(@1, "Type error");
                            $$.truelist = comp.merge(expr.truelist, $4.truelist);
                            comp.backpatch(expr.falselist, $3);
                            $$.falselist = $4.falselist;
                          }
                          else if (comp.type_is_int($1.type))
                          {
                            comp.emit3(ifeq, 5);
                            comp.emit(pop);
                            comp.emit(iconst_1);
                            bool ok;
                            $$ = comp.circuit($4, ok);
                            if (!ok)
                              error(@4, "Type error");
                          }
                          else
                          {
                            error(@1 + @4, "Type error");
                          }
                        }
        | expr AN L expr 
                        {
                          if (comp.type_is_circuit($1.type) && comp.type_is_circuit($4.type))
                          {
                            $$.falselist = comp.merge($1.falselist, $4.falselist);
                            comp.backpatch($1.truelist, $3);
                            $$.truelist = $4.truelist;
                          }
                          else if (comp.type_is_circuit($1.type))
                          {
                            bool ok;
                            Expr expr = comp.circuit($4, ok);
                            if (!ok)
                              error(@4, "Type error");
                            $$.falselist = comp.merge($1.falselist, expr.falselist);
                            comp.backpatch($1.truelist, $3);
                            $$.truelist = expr.truelist;
                          }
                          else if (comp.type_is_circuit($4.type))
                          {
                            bool ok;
                            Expr expr = comp.circuit($1, ok);
                            if (!ok)
                              error(@1, "Type error");
                            $$.falselist = comp.merge(expr.falselist, $4.falselist);
                            comp.backpatch(expr.truelist, $3);
                            $$.truelist = $4.truelist;
                          }
                          else if (comp.type_is_int($1.type))
                          {
                            comp.emit3(ifne, 5);
                            comp.emit(pop);
                            comp.emit(iconst_0);
                            bool ok;
                            $$ = comp.circuit($4, ok);
                            if (!ok)
                              error(@4, "Type error");
                          }
                          else
                          {
                            error(@1 + @4, "Type error");
                          }
                        }
        | expr '|' expr {
                          bool ok;
                          $$ = comp.emit_op($1, $3, ior, nop, ok);
                        }
        | expr '^' expr {
                          bool ok;
                          $$ = comp.emit_op($1, $3, ixor, nop, ok);
                        }
        | expr '&' expr {
                          bool ok;
                          $$ = comp.emit_op($1, $3, iand, nop, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr EQ expr  {
                          bool ok;
                          $$ = comp.emit_rel($1, $3, ifeq, if_icmpeq, dsub, true, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr NE expr  {
                          bool ok;
                          $$ = comp.emit_rel($1, $3, ifne, if_icmpne, dsub, false, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr GE expr  {
                          bool ok;
                          $$ = comp.emit_rel($1, $3, ifge, if_icmpge, dcmpl, true, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr '<' expr {
                          bool ok;
                          $$ = comp.emit_rel($1, $3, iflt, if_icmplt, dcmpl, false, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr LE expr  {
                          bool ok;
                          $$ = comp.emit_rel($1, $3, ifle, if_icmple, dcmpg, true, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr '>' expr {
                          bool ok;
                          $$ = comp.emit_rel($1, $3, ifgt, if_icmpgt, dcmpg, false, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr LS expr  {
                          bool ok;
                          $$ = comp.emit_op($1, $3, ishl, nop, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr RS expr  {
                          bool ok;
                          $$ = comp.emit_op($1, $3, ishr, nop, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr '+' expr {
                          bool ok;
                          $$ = comp.emit_op($1, $3, iadd, dadd, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr '-' expr {
                          bool ok;
                          $$ = comp.emit_op($1, $3, isub, dsub, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr '*' expr {
                          bool ok;
                          $$ = comp.emit_op($1, $3, imul, dmul, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr '/' expr {
                          bool ok;
                          $$ = comp.emit_op($1, $3, idiv, ddiv, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | expr '%' expr {
                          bool ok;
                          $$ = comp.emit_op($1, $3, irem, nop, ok);
                          if (!ok)
                            error(@1 + @3, "Type error");
                        }
        | '!' expr      {
                          bool ok;
                          $$ = comp.circuit($2, ok);
                          if (!ok)
                            error(@2, "Type error");
                          BackpatchList *tmp = $$.truelist;
                          $$.truelist = $$.falselist;
                          $$.falselist = tmp;
                        }
        | '~' expr      {
                          $$.type = comp.decircuit($2);
                          if (!comp.type_is_int($$.type))
                            error(@1 + @2, "Type error");
                          comp.emit(iconst_m1);
                          comp.emit(ixor);
                        }
        | '+' expr %prec '!'
                        {
                          $$.type = comp.decircuit($2);
                          if (!comp.type_is_int($$.type) && !comp.type_is_double($$.type))
                            error(@2, "Type error");
                        }
        | '-' expr %prec '!'
                        {
                          $$.type = comp.decircuit($2);
                          if (comp.type_is_int($$.type))
                            comp.emit(ineg);
                          else if (comp.type_is_double($$.type))
                            comp.emit(dneg);
                          else
                            error(@1 + @2, "Type error");
                        }
        | '(' expr ')'  {
                          $$ = $2;
                        }
        | '#' expr      {
                          TD type = comp.decircuit($2);
                          if (comp.type_is_int(type))
                          {
                            comp.emit3(invokestatic, comp.pool_add_Method("java/lang/Integer", "bitCount", "(I)I"));
                          }
                          else if (comp.type_is_string(type))
                          {
                            comp.emit3(invokevirtual, comp.pool_add_Method("java/lang/String", "length", "()I"));
                          }
                          else if (comp.type_is_array(type))
                          {
                            comp.emit(arraylength);
                          }
                          else
                          {
                            error(@2, "Type error");
                          }
                          $$.type = comp.type_int();
                        }
        | '#' '$'       {
                          if (comp.main)
                          {
                            comp.emit(aload_0);
                            comp.emit(arraylength);
                          }
                          else
                          {
                            error(@1, "Invalid use of $");
                          }
                          $$.type = comp.type_int();
                        }
        | '$' expr      {
                          if (comp.main)
                          {
                            if (comp.type_is_int(comp.decircuit($2)))
                            {
                              comp.emit(iconst_1);
                              comp.emit(isub);
                              comp.emit(aload_0);
                              comp.emit(swap);
                              comp.emit(aaload);
                            }
                            else
                            {
                              error(@2, "Type error");
                            }
                          }
                          else
                          {
                            error(@1, "Invalid use of $");
                          }
                          $$.type = comp.type_string();
                        }
        | expr '[' expr ']'
                        {
                          if (!comp.type_is_int(comp.decircuit($3)))
                          {
                            error(@3, "Type error");
                          }
                          else
                          {
                            TD type = comp.type_get_element($1.type);
                            if (comp.type_is_int(type))
                            {
                              comp.emit(iaload);
                            }
                            else if (comp.type_is_double(type))
                            {
                              comp.emit(daload);
                            }
                            else if (comp.type_is_string(type) || comp.type_is_array(type))
                            {
                              comp.emit(aaload);
                            }
                            else
                            {
                              error(@1, "Type error");
                            }
                            $$.type = type;
                          }
                        }
        | expr '.' ID   {
                          error(@1 + @3, "Not implemented");
                        }
        | expr AR ID    {
                          error(@1 + @3, "Not implemented");
                        }
        | PP ID         {
                          Entry *entry = comp.table[comp.scope]->lookup($2);
                          if (entry == NULL)
                          {
                            error(@2, "Undefined name");
                          }
                          else if (!comp.type_is_int(entry->type))
                          {
                            error(@2, "Type error");
                          }
                          else if (entry->table->scope == 0)
                          {
                            comp.emit3(getstatic, entry->place);
                            comp.emit(iconst_1);
                            comp.emit(iadd);
                            comp.emit(dup);
                            comp.emit3(putstatic, entry->place);
                          }
                          else
                          {
                            comp.emit3(iinc, entry->place, 1);
                            bool ok;
                            comp.emit_load(entry->type, entry->place, ok);
                            if (!ok)
                              error(@2, "Type error");
                          }
                          $$.type = comp.type_int();
                        }
        | NN ID         {
                          Entry *entry = comp.table[comp.scope]->lookup($2);
                          if (entry == NULL)
                          {
                            error(@2, "Undefined name");
                          }
                          else if (!comp.type_is_int(entry->type))
                          {
                            error(@2, "Type error");
                          }
                          else if (entry->table->scope == 0)
                          {
                            comp.emit3(getstatic, entry->place);
                            comp.emit(iconst_1);
                            comp.emit(isub);
                            comp.emit(dup);
                            comp.emit3(putstatic, entry->place);
                          }
                          else
                          {
                            comp.emit3(iinc, entry->place, 0xff);
                            bool ok;
                            comp.emit_load(entry->type, entry->place, ok);
                            if (!ok)
                              error(@2, "Type error");
                          }
                          $$.type = comp.type_int();
                        }
        | ID PP         {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                          {
                            error(@1, "Undefined name");
                          }
                          else if (!comp.type_is_int(entry->type))
                          {
                            error(@1, "Type error");
                          }
                          else if (entry->table->scope == 0)
                          {
                            comp.emit3(getstatic, entry->place);
                            comp.emit(dup);
                            comp.emit(iconst_1);
                            comp.emit(iadd);
                            comp.emit3(putstatic, entry->place);
                          }
                          else
                          {
                            bool ok;
                            comp.emit_load(entry->type, entry->place, ok);
                            if (!ok)
                              error(@1, "Type error");
                            comp.emit3(iinc, entry->place, 1);
                          }
                          $$.type = comp.type_int();
                        }
        | ID NN         {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                          {
                            error(@1, "Undefined name");
                          }
                          else if (!comp.type_is_int(entry->type))
                          {
                            error(@1, "Type error");
                          }
                          else if (entry->table->scope == 0)
                          {
                            comp.emit3(getstatic, entry->place);
                            comp.emit(dup);
                            comp.emit(iconst_1);
                            comp.emit(isub);
                            comp.emit3(putstatic, entry->place);
                          }
                          else
                          {
                            bool ok;
                            comp.emit_load(entry->type, entry->place, ok);
                            if (!ok)
                              error(@1, "Type error");
                            comp.emit3(iinc, entry->place, 0xff);
                          }
                          $$.type = comp.type_int();
                        }
        | ID            {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                          {
                            error(@1, "Undefined name");
                          }
                          else
                          {
                            if (entry->table->scope == 0)
                            {
                              comp.emit3(getstatic, entry->place);
                            }
                            else
                            {
                              bool ok;
                              comp.emit_load(entry->type, entry->place, ok);
                              if (!ok)
                                error(@1, "Type error");
                            }
                            $$.type = entry->type;
                          }
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
                            error(@1, "Integer constant out of range");
                          $$.type = comp.type_int();
                        }
        | F8            {
                          if ($1 == 0.0)
                            comp.emit(dconst_0);
                          else if ($1 == 1.0)
                            comp.emit(dconst_1);
                          else
                            comp.emit3(ldc2_w, comp.pool_add_Double($1));
                          $$.type = comp.type_double();
                        }
        | CS            {
                          comp.emit_ldc(comp.pool_add_String($1));
                          $$.type = comp.type_string();
                        }
        | FALSE         {
                          $$.falselist = comp.backpatch_addr();
                          comp.emit3(goto_, 0);
                        }
        | TRUE          {
                          $$.truelist = comp.backpatch_addr();
                          comp.emit3(goto_, 0);
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
                              $$.type = comp.type_get_return(lib->type);
                              if (comp.type_is_boolean($$.type) || comp.type_is_char($$.type))
                                $$.type = comp.type_int();
                            }
                            else
                            {
                              error(@1, "Undefined function or type error in arguments");
                            }
                          }
                          else if (entry->table->scope != 0)
                          {
                            error(@1, "Not a function");
                          }
                          else if (comp.type_check_args($3.type, entry->type))
                          {
                            // invoke a compiled function
                            comp.emit3(invokestatic, comp.pool_add_Method($1->c_str(), entry->type));
                            $$.type = comp.type_get_return(entry->type);
                          }
                          else
                          {
                            error(@3, "Type error in arguments");
                          }
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
                            if (comp.type_is_double($1.type))
                              comp.emit(pop2);
                            else
                              comp.emit(pop);
                          }
                        }
        | /* empty */
        ;

cond    : expr          {
                          bool ok;
                          $$ = comp.circuit($1, ok);
                          if (!ok)
                            error(@1, "Type error");
                        }
        ;

optcond : cond          {
                          $$ = $1;
                        }
        | /* empty */   {
                          $$.truelist = comp.backpatch_addr();
                          comp.emit3(goto_, 0);
                        }
        ;

id      : ID            {
                          Entry *entry = comp.table[comp.scope]->lookup($1);
                          if (entry == NULL)
                          {
                            error(@1, "Undefined name");
                          }
                          else if (entry->table->scope == 0)
                          {
                            comp.emit3(getstatic, entry->place);
                          }
                          else
                          {
                            bool ok;
                            comp.emit_load(entry->type, entry->place, ok);
                            if (!ok)
                              error(@1, "Type error");
                          }
                          $$ = $1;
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

L       : /* empty */   {
                          $$ = comp.addr();
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

  // construct a parser, needs the lexer and compiler for semantic actions
  yy::Parser parser(lexer, comp);

  // parse and compile the source into a JVM class file
  if (parser.parse())
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
  std::cerr << loc << ": " << msg << std::endl;
  if (loc.begin.line == loc.end.line && loc.begin.line == lexer.lineno())
  {
    // the error is on the current line and spans only one line
    std::cerr << lexer.matcher().line() << std::endl;
    for (size_t i = 0; i < loc.begin.column; ++i)
      std::cerr << " ";
    for (size_t i = loc.begin.column; i <= loc.end.column; ++i)
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
      for (size_t i = 1; i < loc.begin.line; ++i)
        m->skip('\n'); // skip to the next line
      for (size_t i = loc.begin.line; i <= loc.end.line; ++i)
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
