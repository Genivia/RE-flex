// Lua 5.3 Bison parser and transpiler by Robert van Engelen

#ifndef LUA_HPP
#define LUA_HPP

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <set>
#include <stack>
#include <list>
#include <typeinfo>

#define LUA_NAME_FORMAT "lua.%s"
#define LUA_OPERATOR_FORMAT "%s"

// identifiers are efficiently compared for equality by their pointer to a string stored in the Scanner::symbols set
typedef const std::string *Name;
// scanner returns unsigned 64 bit integer literals
typedef uint64_t           IntegerLiteral;
// scanner returns double precision literals
typedef double             FloatLiteral;
// strings are stored in the Scanner::symbols set
typedef const char        *StringLiteral;

// transpiler
class Transpiler {
 public:

  // a list of expressions, statements etc.
  template<typename T> struct List : public std::list<std::unique_ptr<T>> {
    List() = default;

    List(T *x) {
      this->add(x);
    }

    List(List *x) {
      this->concat(x);
    }

    virtual ~List() = default;

    List *add(T *x)
    {
      this->emplace_back(x);
      return this;
    }

    List *concat(List *x)
    {
      this->splice(this->end(), *x, x->begin(), x->end());
      delete x;
      return this;
    }
  };

  // a list of names, each Name is a pointer to a string stored in the symbol table
  struct NameList : public std::list<Name> {
    NameList() = default;

    NameList(Name x) {
      this->add(x);
    }

    virtual ~NameList() = default;

    NameList *add(Name x)
    {
      this->emplace_back(x);
      return this;
    }
  };

  // abstract syntax tree
  struct AbstractSyntaxTree {
    virtual ~AbstractSyntaxTree() = default;
    virtual void transpile(Transpiler& transpiler) { fprintf(transpiler.out, "???"); }
  };

  // abstract expression
  struct Expression : public AbstractSyntaxTree {
    // virtual ~Expression() = default;
    virtual void transpile(Transpiler& transpiler) { fprintf(transpiler.out, "???expression???"); }
  };

  // abstract statement
  struct Statement : public AbstractSyntaxTree {
    // virtual ~Statement() = default;
    virtual void transpile(Transpiler& transpiler) { fprintf(transpiler.out, "???statement???"); }
  };

  // nil expression
  struct Nil : public Expression {
    virtual void transpile(Transpiler& transpiler) { fprintf(transpiler.out, "()"); }
  };

  // true expression
  struct True : public Expression {
    virtual void transpile(Transpiler& transpiler) { fprintf(transpiler.out, "#t"); }
  };

  // false expression
  struct False : public Expression {
    virtual void transpile(Transpiler& transpiler) { fprintf(transpiler.out, "#f"); }
  };

  // integer literal expression
  struct Integer : public Expression {
    Integer(IntegerLiteral n) : literal(n) { }
    virtual void transpile(Transpiler& transpiler) { fprintf(transpiler.out, "%llu", static_cast<unsigned long long>(literal)); }
    IntegerLiteral literal;
  };

  // floating point literal expression
  struct Float : public Expression {
    Float(FloatLiteral n) : literal(n) { }
    virtual void transpile(Transpiler& transpiler) { fprintf(transpiler.out, "%.16lg", literal); }
    FloatLiteral literal;
  };

  // string literal expression
  struct String : public Expression {
    String(StringLiteral s) : literal(s) { }
    virtual void transpile(Transpiler& transpiler)
    {
      putc('"', transpiler.out);
      for (const char *s = literal; *s; ++s)
      {
        if (*s == '\\' || *s == '"')
          fprintf(transpiler.out, "\\%c", *s);
        else if (isprint(*s) || static_cast<unsigned char>(*s) >= 128)
          putc(*s, transpiler.out);
        else
          fprintf(transpiler.out, "\\x%02x", *s);
      }
      putc('"', transpiler.out);
    }
    StringLiteral literal;
  };

  // ... expression
  struct Dots : public Expression {
    virtual void transpile(Transpiler& transpiler)
    {
      fprintf(transpiler.out, "...");
    }
  };

  // anonymous function (lambda) expression
  struct Parameters;
  struct Lambda : public Expression {
    Lambda(Parameters *l, List<Statement> *b) : parameters(l), block(b) { }
    virtual void transpile(Transpiler& transpiler)
    {
      transpile(transpiler, false);
    }
    void transpile(Transpiler& transpiler, bool self)
    {
      transpiler.begin_local_scope();
      fprintf(transpiler.out, "(lambda");
      parameters->transpile(transpiler, self);
      fprintf(transpiler.out, " (block @func@");
      for (auto& statement : *block)
      {
        transpiler.newline();
        statement->transpile(transpiler);
      }
      fprintf(transpiler.out, "))");
      transpiler.end_local_scope();
    }
    std::unique_ptr<Parameters> parameters;
    std::unique_ptr<List<Statement>> block;
  };

  // table constructor expression
  struct Field;
  struct Table : public Expression {
    Table(List<Field> *f) : fields(f) { }
    virtual void transpile(Transpiler& transpiler)
    {
      if (fields->empty())
      {
        fprintf(transpiler.out, "()");
      }
      else
      {
        fprintf(transpiler.out, "(list");
        size_t index = 0;
        for (auto& field : *fields)
        {
          fprintf(transpiler.out, " (cons ");
          if (field->key)
          {
            if (typeid(*field->key) == typeid(Variable))
              fprintf(transpiler.out, "'%s", dynamic_cast<Variable*>(field->key.get())->name->c_str());
            else
              field->key->transpile(transpiler);
          }
          else
          {
            fprintf(transpiler.out, "%zu", ++index);
          }
          putc(' ', transpiler.out);
          field->value->transpile(transpiler);
          putc(')', transpiler.out);
        }
        putc(')', transpiler.out);
      }
    }
    std::unique_ptr<List<Field>> fields;
  };

  // function call expression
  struct Call : public Expression {
    Call(Expression *f, List<Expression> *a, Name v) : function(f), arguments(a), colonname(v) { }
    virtual void transpile(Transpiler& transpiler)
    {
      if (colonname)
      {
        fprintf(transpiler.out, "(let ((self ");
        function->transpile(transpiler);
        fprintf(transpiler.out, ")) ((index '%s self) () self", colonname->c_str());
        for (auto& argument : *arguments)
        {
          putc(' ', transpiler.out);
          argument->transpile(transpiler);
        }
        fprintf(transpiler.out, "))");
      }
      else
      {
        putc('(', transpiler.out);
        function->transpile(transpiler);
        fprintf(transpiler.out, " ()");
        for (auto& argument : *arguments)
        {
          putc(' ', transpiler.out);
          argument->transpile(transpiler);
        }
        putc(')', transpiler.out);
      }
    }
    std::unique_ptr<Expression> function;
    std::unique_ptr<List<Expression>> arguments;
    Name colonname;
  };

  // unary operator expression
  struct UnaryOp : public Expression {
    UnaryOp(const char *s, Expression *e) : op(s), operand(e) { }
    virtual void transpile(Transpiler& transpiler)
    {
      fprintf(transpiler.out, "(" LUA_OPERATOR_FORMAT " ", op);
      operand->transpile(transpiler);
      putc(')', transpiler.out);
    }
    const char *op;
    std::unique_ptr<Expression> operand;
  };

  // binary operator expression
  struct Op : public Expression {
    Op(const char *s, Expression *e1, Expression *e2) : op(s), operand1(e1), operand2(e2) { }
    virtual void transpile(Transpiler& transpiler)
    {
      transpile(transpiler, false);
    }
    void transpile(Transpiler& transpiler, bool nested)
    {
      if (!nested)
        fprintf(transpiler.out, "(" LUA_OPERATOR_FORMAT " ", op);
      if (typeid(*operand1) == typeid(Op) && strcmp(dynamic_cast<Op*>(operand1.get())->op, op) == 0)
        dynamic_cast<Op*>(operand1.get())->transpile(transpiler, true);
      else
        operand1->transpile(transpiler);
      putc(' ', transpiler.out);
      if (typeid(*operand2) == typeid(Op) && strcmp(dynamic_cast<Op*>(operand2.get())->op, op) == 0)
        dynamic_cast<Op*>(operand2.get())->transpile(transpiler, true);
      else
        operand2->transpile(transpiler);
      if (!nested)
        putc(')', transpiler.out);
    }
    const char *op;
    std::unique_ptr<Expression> operand1, operand2;
  };

  // variable wraps a name as expression
  struct Variable : public Expression {
    Variable(Name v) : name(v) { }
    virtual void transpile(Transpiler& transpiler)
    {
#ifdef NAME_LOOKUP
      // if unassigned/undefined Lisp variables cause errors: use a Lisp lookup special form
      fprintf(transpiler.out, "(lookup " LUA_NAME_FORMAT ")", name->c_str());
#else
      fprintf(transpiler.out, LUA_NAME_FORMAT, name->c_str());
#endif
    }
    Name name;
  };

  // var[] index accessor expression
  struct Index : public Expression {
    Index(Expression *v, Expression *e) : var(v), index(e) { }
    virtual void transpile(Transpiler& transpiler)
    {
      fprintf(transpiler.out, "(index ");
      index->transpile(transpiler);
      putc(' ', transpiler.out);
      var->transpile(transpiler);
      putc(')', transpiler.out);
    }
    std::unique_ptr<Expression> var, index;
  };

  // var.member accessor expression
  struct Member : public Expression {
    Member(Expression *v, Name m) : var(v), member(m) { }
    virtual void transpile(Transpiler& transpiler)
    {
      fprintf(transpiler.out, "(index '%s ", member->c_str());
      var->transpile(transpiler);
      putc(')', transpiler.out);
    }
    std::unique_ptr<Expression> var;
    Name member;
  };

  // do end block statement
  struct Block : public Statement {
    Block() { }
    Block(List<Statement> *b) : block(b) { }
    virtual void transpile(Transpiler& transpiler)
    {
      transpiler.begin_local_scope();
      fprintf(transpiler.out, "(begin");
      for (auto& statement : *block)
      {
        transpiler.newline();
        statement->transpile(transpiler);
      }
      putc(')', transpiler.out);
      transpiler.end_local_scope();
    }
    std::unique_ptr<List<Statement>> block;
  };

  // if statement
  struct Condition;
  struct If : public Statement {
    If(List<Condition> *c) : conditions(c) { }
    virtual void transpile(Transpiler& transpiler)
    {
      fprintf(transpiler.out, "(cond");
      for (auto& condition : *conditions)
        condition->transpile(transpiler);
      putc(')', transpiler.out);
    }
    std::unique_ptr<List<Condition>> conditions;
  };

  // while do statement
  struct While : public Statement {
    While(Expression *c, List<Statement> *b) : condition(c), block(b) { }
    virtual void transpile(Transpiler& transpiler)
    {
      transpiler.begin_local_scope();
      fprintf(transpiler.out, "(block @loop@ (while ");
      condition->transpile(transpiler);
      for (auto& statement : *block)
      {
        transpiler.newline();
        statement->transpile(transpiler);
      }
      fprintf(transpiler.out, "))");
      transpiler.end_local_scope();
    }
    std::unique_ptr<Expression> condition;
    std::unique_ptr<List<Statement>> block;
  };

  // repeat until statement
  struct Until : public Statement {
    Until(List<Statement> *b, Expression *c) : block(b), condition(c) { }
    virtual void transpile(Transpiler& transpiler)
    {
      transpiler.begin_local_scope();
      fprintf(transpiler.out, "(block @loop@ (until ");
      condition->transpile(transpiler);
      for (auto& statement : *block)
      {
        transpiler.newline();
        statement->transpile(transpiler);
      }
      fprintf(transpiler.out, "))");
      transpiler.end_local_scope();
    }
    std::unique_ptr<List<Statement>> block;
    std::unique_ptr<Expression> condition;
  };

  // for = do statement
  struct Range;
  struct ForCounter : public Statement {
    ForCounter(Name v, Range *r, List<Statement> *b) : name(v), range(r), block(b) { }
    virtual void transpile(Transpiler& transpiler)
    {
      transpiler.begin_local_scope();
      fprintf(transpiler.out, "(block @loop@ (do ((" LUA_NAME_FORMAT " ", name->c_str());
      range->start->transpile(transpiler);
      fprintf(transpiler.out, " (+ " LUA_NAME_FORMAT " ", name->c_str());
      if (range->step)
        range->step->transpile(transpiler);
      else
        putc('1', transpiler.out);
      fprintf(transpiler.out, ")) ((> " LUA_NAME_FORMAT " ", name->c_str());
      range->end->transpile(transpiler);
      fprintf(transpiler.out, "))");
      for (auto& statement : *block)
      {
        transpiler.newline();
        statement->transpile(transpiler);
      }
      fprintf(transpiler.out, ")))");
      transpiler.end_local_scope();
    }
    Name name;
    std::unique_ptr<Range> range;
    std::unique_ptr<List<Statement>> block;
  };

  // for in do statement
  struct ForIterator : public Statement {
    ForIterator(NameList *v, List<Expression> *e, List<Statement> *b) : namelist(v), iterators(e), block(b) { }
    virtual void transpile(Transpiler& transpiler)
    {
      if (iterators->size() == 1)
      {
        fprintf(transpiler.out, "(");
        iterators->front()->transpile(transpiler);
        fprintf(transpiler.out, " ");
      }
      else
      {
        fprintf(transpiler.out, "(let ((yield ");
      }
      if (namelist->size() == 1)
      {
        fprintf(transpiler.out, "(lambda (" LUA_NAME_FORMAT ")", namelist->front()->c_str());
      }
      else
      {
        int i = 1, c = '(';
        fprintf(transpiler.out, "(lambda (ret) (let ");
        for (auto& name : *namelist)
        {
          fprintf(transpiler.out, "%c(" LUA_NAME_FORMAT " (nth %d ret))", c, name->c_str(), i);
          ++i;
          c = ' ';
        }
        fprintf(transpiler.out, ")");
      }
      transpiler.begin_local_scope();
      for (auto& statement : *block)
      {
        transpiler.newline();
        statement->transpile(transpiler);
      }
      transpiler.end_local_scope();
      if (namelist->size() != 1)
        fprintf(transpiler.out, ")");
      fprintf(transpiler.out, "))");
      if (iterators->size() != 1)
      {
        fprintf(transpiler.out, ")");
        for (auto& iterator : *iterators)
        {
          transpiler.newline();
          fprintf(transpiler.out, "(");
          iterator->transpile(transpiler);
          fprintf(transpiler.out, " yield)");
        }
        fprintf(transpiler.out, ")");
      }
    }
    std::unique_ptr<NameList> namelist;
    std::unique_ptr<List<Expression>> iterators;
    std::unique_ptr<List<Statement>> block;
  };

  // function statement
  struct FunctionName;
  struct Function : public Statement {
    Function(FunctionName *f, Lambda *l) : functionname(f), lambda(l) { }
    virtual void transpile(Transpiler& transpiler)
    {
      if (functionname->namelist->size() == 1)
      {
        fprintf(transpiler.out, "(define " LUA_NAME_FORMAT " ", functionname->namelist->front()->c_str());
        lambda->transpile(transpiler);
        putc(')', transpiler.out);
      }
      else
      {
        fprintf(transpiler.out, "(assign (");
        NameList::reverse_iterator iter = functionname->namelist->rbegin();
        while (iter != functionname->namelist->rend())
        {
          Name name = *iter++;
          if (iter == functionname->namelist->rend())
            fprintf(transpiler.out, LUA_NAME_FORMAT, name->c_str());
          else
            fprintf(transpiler.out, "(index '%s ", name->c_str());
        }
        for (size_t i = 0; i < functionname->namelist->size(); ++i)
          putc(')', transpiler.out);
        fprintf(transpiler.out, " (");
        lambda->transpile(transpiler, functionname->self);
        fprintf(transpiler.out, "))");
      }
    }
    std::unique_ptr<FunctionName> functionname;
    std::unique_ptr<Lambda> lambda;
  };

  // goto statement
  struct Goto : public Statement {
    Goto(Name v) : label(v) { }
    Name label;
  };

  // :: label :: statement
  struct Label : public Statement {
    Label(Name v) : label(v) { }
    Name label;
  };

  // multi-way assignment statement
  struct Assign : public Statement {
    Assign(List<Expression> *l, List<Expression> *r) : lhs(l), rhs(r) { }
    virtual void transpile(Transpiler& transpiler)
    {
      fprintf(transpiler.out, "(assign ");
      int c = '(';
      for (auto& expression : *lhs)
      {
        putc(c, transpiler.out);
        expression->transpile(transpiler);
        c = ' ';
      }
      fprintf(transpiler.out, ") ");
      c = '(';
      for (auto& expression : *rhs)
      {
        putc(c, transpiler.out);
        expression->transpile(transpiler);
        c = ' ';
      }
      fprintf(transpiler.out, "))");
    }
    std::unique_ptr<List<Expression>> lhs, rhs;
  };

  // function call statement, wraps a function call expression
  struct FunctionCall : public Statement {
    FunctionCall(Call *c) : call(c) { }
    virtual void transpile(Transpiler& transpiler)
    {
      call->transpile(transpiler);
    }
    std::unique_ptr<Call> call;
  };

  // local binding statement
  struct Local : public Statement {
    Local(NameList *v, List<Expression> *e) : namelist(v), init(e) { }
    virtual void transpile(Transpiler& transpiler)
    {
      fprintf(transpiler.out, "(let ");
      if (namelist->size() == init->size() || init->empty())
      {
        putc('(', transpiler.out);
        transpiler.indent++;
        NameList::iterator i = namelist->begin();
        List<Expression>::iterator j = init->begin();
        while (i != namelist->end())
        {
          transpiler.newline();
          fprintf(transpiler.out, "(" LUA_NAME_FORMAT, (*i)->c_str());
          if (j != init->end())
          {
            putc(' ', transpiler.out);
            (*j)->transpile(transpiler);
            ++j;
          }
          putc(')', transpiler.out);
          ++i;
        }
        transpiler.indent--;
        putc(')', transpiler.out);
      }
      else
      {
        int c = '(';
        for (auto& name : *namelist)
        {
          fprintf(transpiler.out, "%c(" LUA_NAME_FORMAT ")", c, name->c_str());
          c = ' ';
        }
        putc(')', transpiler.out);
        if (!init->empty())
        {
          transpiler.newline();
          fprintf(transpiler.out, "(assign ");
          c = '(';
          for (auto& name : *namelist)
          {
            fprintf(transpiler.out, "%c" LUA_NAME_FORMAT, c, name->c_str());
            c = ' ';
          }
          fprintf(transpiler.out, ") ");
          c = '(';
          for (auto& expression : *init)
          {
            putc(c, transpiler.out);
            expression->transpile(transpiler);
            c = ' ';
          }
          fprintf(transpiler.out, "))");
        }
      }
      transpiler.locals++;
    }
    std::unique_ptr<NameList> namelist;
    std::unique_ptr<List<Expression>> init;
  };

  // local function binding statement
  struct LocalFunction : public Statement {
    LocalFunction(Name v, Lambda *l) : name(v), lambda(l) { }
    virtual void transpile(Transpiler& transpiler)
    {
      fprintf(transpiler.out, "(letrec ((" LUA_NAME_FORMAT " ", name->c_str());
      lambda->transpile(transpiler);
      fprintf(transpiler.out, "))");
      transpiler.locals++;
    }
    Name name;
    std::unique_ptr<Lambda> lambda;
  };

  // break statement
  struct Break : public Statement {
    virtual void transpile(Transpiler& transpiler)
    {
      fprintf(transpiler.out, "(return-from @loop@)");
    }
  };

  // return statement
  struct Return : public Statement {
    Return(List<Expression> *l) : list(l) { }
    virtual void transpile(Transpiler& transpiler)
    {
      fprintf(transpiler.out, "(let ((ret");
      if (!list->empty())
      {
        putc(' ', transpiler.out);
        if (list->size() == 1)
        {
          list->front()->transpile(transpiler);
        }
        else
        {
          fprintf(transpiler.out, "(list");
          for (auto& expression : *list)
          {
            putc(' ', transpiler.out);
            expression->transpile(transpiler);
          }
          putc(')', transpiler.out);
        }
      }
      fprintf(transpiler.out, "))");
      transpiler.newline();
      fprintf(transpiler.out, "(if (and yield (not (null ret))) (yield ret) (return-from @func@ ret)))");
    }
    std::unique_ptr<List<Expression>> list;
  };

  // if/elsif then, with optional test expression or NULL
  struct Condition : public AbstractSyntaxTree {
    Condition(Expression *t, List<Statement> *b) : test(t), block(b) { }
    virtual void transpile(Transpiler& transpiler)
    {
      transpiler.begin_local_scope();
      transpiler.newline();
      fprintf(transpiler.out, "(");
      test->transpile(transpiler);
      transpiler.indent++;
      for (auto& statement : *block)
      {
        transpiler.newline();
        statement->transpile(transpiler);
      }
      transpiler.indent--;
      putc(')', transpiler.out);
      transpiler.end_local_scope();
    }
    std::unique_ptr<Expression> test;
    std::unique_ptr<List<Statement>> block;
  };

  // dotted name of a function with optional last colon name (flagged as self)
  struct FunctionName : public AbstractSyntaxTree {
    FunctionName(NameList *f, bool s) : namelist(f), self(s) { }
    std::unique_ptr<NameList> namelist;
    bool self;
  };

  // parameters of a function with optional ... (dots is true)
  struct Parameters : public AbstractSyntaxTree {
    Parameters(NameList *v, bool d) : namelist(v), dots(d) { }
    virtual void transpile(Transpiler& transpiler)
    {
      transpile(transpiler, false);
    }
    void transpile(Transpiler& transpiler, bool self)
    {
      fprintf(transpiler.out, " (yield");
      if (self)
        fprintf(transpiler.out, " " LUA_NAME_FORMAT, "self");
      for (auto& name : *namelist)
      {
        if (typeid(name) == typeid(Dots))
          fprintf(transpiler.out, " . ...");
        else
          fprintf(transpiler.out, " " LUA_NAME_FORMAT, name->c_str());
      }
      putc(')', transpiler.out);
    }
    std::unique_ptr<NameList> namelist;
    bool dots;
  };

  // for counter range
  struct Range : public AbstractSyntaxTree {
    Range(Expression *e1, Expression *e2, Expression *e3) : start(e1), end(e2), step(e3) { }
    std::unique_ptr<Expression> start, end, step;
  };

  // table field with optional key or NULL and value
  struct Field : AbstractSyntaxTree {
    Field(Expression *k, Expression *v) : key(k), value(v) { }
    std::unique_ptr<Expression> key, value;
  };

  Transpiler(const std::string& s) : name(s), chunk(NULL), locals(0), indent(0), out(NULL) { }

  ~Transpiler()
  {
    if (chunk)
      delete chunk;
  }

  void begin_local_scope()
  {
    scope.push(locals);
    ++indent;
  }

  void end_local_scope()
  {
    size_t restore_locals = scope.top();
    scope.pop();
    for (; locals > restore_locals; --locals)
      putc(')', out);
    --indent;
  }

  void newline()
  {
    fprintf(out, "\n%*s", 2*indent, "");
  }

  bool save()
  {
    if (name == "-")
      out = stdout;
    else
      out = fopen(name.append(".lisp").c_str(), "w");

    if (!out)
      return false;

    fprintf(out, "; %s", name.c_str());
    for (auto& statement : *chunk)
    {
      newline();
      statement->transpile(*this);
    }
    for (; locals; --locals)
      putc(')', out);
    putc('\n', out);

    if (out != stdout)
      fclose(out);
    out = NULL;

    return true;
  }

  std::string name;
  List<Transpiler::Statement> *chunk;
  size_t locals;
  int indent;
  std::stack<size_t> scope;
  FILE *out;

};

#endif
