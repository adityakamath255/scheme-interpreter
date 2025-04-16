#pragma once
#include "types.hpp"
#include "environment.hpp"
#include "tco.hpp"

namespace Scheme {

using EvalResult = std::variant<
  Obj,
  TailCall
>;

class Expression {
private:
  int get_size(Cons* obj) const;
  void assert_size(Cons *obj, const int lb, const int ub) const;
protected:
  string name;
public:
  Expression(const string& n, Cons *obj = nullptr, const int lb = -1, const int ub = -1);
  string get_name() const;
  virtual EvalResult eval(Environment*) const = 0;
  virtual void tco() {}
};

Expression *classify(const Obj& obj);

struct Literal : public Expression {
  Obj obj;
  Literal(Obj obj);
  EvalResult eval(Environment* env) const override;
};

struct Variable : public Expression {
  Symbol sym;
  Variable(Symbol obj);
  EvalResult eval(Environment* env) const override;
};

struct Quoted : public Expression {
  Obj text_of_quotation;
  Quoted(Cons *obj);
  EvalResult eval(Environment *env) const override;
};

struct Set : public Expression {
  Symbol variable;
  Expression *value;
  Set(Cons *obj);
  EvalResult eval(Environment *env) const override;
};

struct If : public Expression {
  Expression *predicate, *consequent, *alternative;
  If(Cons *obj);
  If(Expression *p, Expression *c, Expression *a);
  If();
  EvalResult eval(Environment *env) const override;
  void tco() override;
};

struct Begin : public Expression {
  vector<Expression*> actions;
  Begin(const vector<Expression*>& seq);
  EvalResult eval(Environment *env) const override;
  void tco() override;
};

Expression *combine_expr(Obj seq);

struct Lambda : public Expression {
  vector<Symbol> parameters;
  Expression *body;
  Lambda(Cons *obj);
  Lambda(Obj parameters_, Obj body_);
  EvalResult eval(Environment *env) const override;
};

struct Define : public Expression {
  Symbol variable;
  Expression *value;
  Define(Cons *obj);
  EvalResult eval(Environment *env) const override;
};

struct Let : public Expression {
  std::map<Symbol, Expression*> bindings;
  Expression *body;
  decltype(bindings) get_bindings(Obj li);
  Environment *get_frame(Environment *env) const;
  Let(Cons *obj);
  EvalResult eval(Environment *env) const override;
};

struct Clause {
private:
  bool is_else_clause(Obj obj) const;
public:
  bool is_else;
  Expression *predicate;
  Expression *actions;
  Clause (Cons *obj);
};

struct Cond : public Expression {
private:
  If *cond2if() const;
public:
  vector<Clause> clauses;
  Expression *if_form;
  Cond (Obj obj);
  EvalResult eval(Environment *env) const override;
  void tco() override;
};

struct Application : public Expression {
  Expression *op;
  vector<Expression*> params;
  bool at_tail = false;
  Application(Cons *obj);
  EvalResult eval(Environment *env) const override;
  void tco() override;
};

struct And : public Expression {
  vector<Expression*> exprs;
  And(Cons *obj);
  EvalResult eval(Environment *env) const override;
};

struct Or : public Expression {
  vector<Expression*> exprs;
  Or(Cons *obj);
  EvalResult eval(Environment *env) const override;
};

struct Cxr : public Expression {
  Symbol word;
  Expression *expr;
  Cxr(Symbol tag, Cons *obj);
  EvalResult eval(Environment *env) const override;
};

}
