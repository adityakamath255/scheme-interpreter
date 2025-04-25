#pragma once
#include "types.hpp"
#include "environment.hpp"

#include <memory>
#include <unordered_map>
#include <variant>

namespace Scheme {

class Interpreter;

struct TailCall {
  Obj proc;
  ArgList args;
  TailCall(Obj p, ArgList ls):
    proc {std::move(p)},
    args {std::move(ls)}
  {}
};

using EvalResult = std::variant<
  Obj,
  TailCall
>;

using ExprPtr = std::unique_ptr<Expression>;
using LambdaBody = std::shared_ptr<Expression>;
using ExprList = std::vector<ExprPtr>;

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
  std::string name;
public:
  Expression(const std::string&, Cons* = nullptr, const int = -1, const int = -1);
  std::string get_name() const;
  virtual EvalResult eval(Environment*, Interpreter&) = 0;
  virtual void tco() {}
};

Expression *classify(const Obj&);

struct Literal : public Expression {
  Obj obj;
  Literal(Obj);
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Variable : public Expression {
  Symbol sym;
  int depth;
  bool resolved;
  Variable(Symbol obj);
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Quoted : public Expression {
  Obj text_of_quotation;
  Quoted(Cons*);
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Set : public Expression {
  Symbol variable;
  ExprPtr value;
  Set(Cons*);
  EvalResult eval(Environment*, Interpreter&) override;
};

struct If : public Expression {
  ExprPtr predicate, consequent, alternative;
  If(Cons*);
  If(Expression*, Expression*, Expression*);
  If();
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override {
    consequent->tco();
    alternative->tco();
  }
};

struct Begin : public Expression {
  ExprList actions;
  Begin(ExprList);
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override {
    if (!actions.empty()) {
      actions.back()->tco();
    }
  }
};

Expression *combine_expr(Obj);

struct Lambda : public Expression {
  ParamList parameters;
  LambdaBody body;
  Lambda(Cons*);
  Lambda(Obj, Obj);
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Define : public Expression {
  Symbol variable;
  ExprPtr value;
  Define(Cons*);
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Let : public Expression {
  std::unordered_map<Symbol, ExprPtr> bindings;
  ExprPtr body;
  decltype(bindings) get_bindings(Obj);
  Let(Cons*);
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Clause {
private:
  bool is_else_clause(Obj) const;
public:
  bool is_else;
  ExprPtr predicate;
  ExprPtr actions;
  Clause (Cons*);
};

struct Cond : public Expression {
private:
  std::vector<Clause> clauses;
public:
  Cond (Obj);
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override {
    for (auto& clause : clauses) {
      clause.actions->tco();
    }
  }
};

struct Application : public Expression {
  ExprPtr op;
  ExprList params;
  bool at_tail = false;
  Application(Cons*);
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override {
    at_tail = true;
  }
};

struct And : public Expression {
  ExprList exprs;
  And(Cons*);
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Or : public Expression {
  ExprList exprs;
  Or(Cons*);
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Cxr : public Expression {
  Symbol word;
  ExprPtr expr;
  Cxr(Symbol, Cons*);
  EvalResult eval(Environment*, Interpreter&) override;
};

}
