#pragma once
#include "types.hpp"
#include "environment.hpp"
#include "tco.hpp"

namespace Scheme {

using EvalResult = std::variant<
  Obj,
  TailCall
>;

bool is_obj(const EvalResult&);
bool is_tailcall(const EvalResult&);

Obj& as_obj(EvalResult&);
const Obj& as_obj(const EvalResult&);

TailCall& as_tailcall(EvalResult&);
const TailCall& as_tailcall(const EvalResult&);

class Expression {
private:
  int get_size(Cons*) const;
  void assert_size(Cons*, const int, const int) const;
protected:
  string name;
public:
  Expression(const string&, Cons* = nullptr, const int = -1, const int = -1);
  string get_name() const;
  virtual EvalResult eval(Environment*) const = 0;
  virtual void tco() {}
};

Expression *classify(const Obj&);

struct Literal : public Expression {
  Obj obj;
  Literal(Obj);
  EvalResult eval(Environment*) const override;
};

struct Variable : public Expression {
  Symbol sym;
  Variable(Symbol obj);
  EvalResult eval(Environment*) const override;
};

struct Quoted : public Expression {
  Obj text_of_quotation;
  Quoted(Cons*);
  EvalResult eval(Environment*) const override;
};

struct Set : public Expression {
  Symbol variable;
  Expression *value;
  Set(Cons*);
  EvalResult eval(Environment*) const override;
};

struct If : public Expression {
  Expression *predicate, *consequent, *alternative;
  If(Cons*);
  If(Expression*, Expression*, Expression*);
  If();
  EvalResult eval(Environment*) const override;
  void tco() override;
};

struct Begin : public Expression {
  vector<Expression*> actions;
  Begin(const vector<Expression*>&);
  EvalResult eval(Environment*) const override;
  void tco() override;
};

Expression *combine_expr(Obj);

struct Lambda : public Expression {
  vector<Symbol> parameters;
  Expression *body;
  Lambda(Cons*);
  Lambda(Obj, Obj);
  EvalResult eval(Environment*) const override;
};

struct Define : public Expression {
  Symbol variable;
  Expression *value;
  Define(Cons*);
  EvalResult eval(Environment*) const override;
};

struct Let : public Expression {
  std::map<Symbol, Expression*> bindings;
  Expression *body;
  decltype(bindings) get_bindings(Obj);
  Environment *get_frame(Environment*) const;
  Let(Cons*);
  EvalResult eval(Environment*) const override;
};

struct Clause {
private:
  bool is_else_clause(Obj) const;
public:
  bool is_else;
  Expression *predicate;
  Expression *actions;
  Clause (Cons*);
};

struct Cond : public Expression {
private:
  If *cond2if() const;
public:
  vector<Clause> clauses;
  Expression *if_form;
  Cond (Obj);
  EvalResult eval(Environment*) const override;
  void tco() override;
};

struct Application : public Expression {
  Expression *op;
  vector<Expression*> params;
  bool at_tail = false;
  Application(Cons*);
  EvalResult eval(Environment*) const override;
  void tco() override;
};

struct And : public Expression {
  vector<Expression*> exprs;
  And(Cons*);
  EvalResult eval(Environment*) const override;
};

struct Or : public Expression {
  vector<Expression*> exprs;
  Or(Cons*);
  EvalResult eval(Environment*) const override;
};

struct Cxr : public Expression {
  Symbol word;
  Expression *expr;
  Cxr(Symbol, Cons*);
  EvalResult eval(Environment*) const override;
};

}
