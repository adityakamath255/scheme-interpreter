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

using ExprList = std::vector<Expression*>;

inline bool is_obj(const EvalResult& res) {return std::holds_alternative<Obj>(res); }
inline bool is_tailcall(const EvalResult& res) {return std::holds_alternative<TailCall>(res); }

inline Obj& as_obj(EvalResult& res) { return std::get<Obj>(res); }
inline const Obj& as_obj(const EvalResult& res) { return std::get<Obj>(res); }

inline TailCall& as_tailcall(EvalResult& res) { return std::get<TailCall>(res); }
inline const TailCall& as_tailcall(const EvalResult& res) { return std::get<TailCall>(res); }

class Expression {
public:
  Expression() = default;
  virtual ~Expression() = default;
  virtual EvalResult eval(Environment*, Interpreter&) = 0;
  virtual void tco() {}
};

struct Literal : public Expression {
  Obj obj;
  explicit Literal(Obj o): obj(std::move(o)) {}
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Variable : public Expression {
  Symbol sym;
  int depth;
  bool resolved;
  explicit Variable(Symbol s, int d = 0, bool r = false): sym(std::move(s)), depth(d), resolved(r) {}
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Quoted : public Expression {
  Obj text_of_quotation;
  explicit Quoted(Obj text): text_of_quotation {std::move(text)} {}
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Set : public Expression {
  Symbol variable;
  Expression *value;
  Set(Symbol var, Expression *val): variable {std::move(var)}, value {val} {}
  EvalResult eval(Environment*, Interpreter&) override;
};

struct If : public Expression {
  Expression *predicate;
  Expression *consequent;
  Expression *alternative;
  If(Expression *p, Expression *c, Expression *a): predicate {p}, consequent {c}, alternative {a} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override {consequent->tco(); alternative->tco();}
};

struct Begin : public Expression {
  ExprList actions;
  Begin(ExprList a): actions {std::move(a)} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override {if (!actions.empty()) {actions.back()->tco();}}
};

struct Lambda : public Expression {
  ParamList parameters;
  Expression *body;
  Lambda(ParamList p, Expression *b): parameters {std::move(p)}, body {b} {
    body->tco();
  }
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Define : public Expression {
  Symbol variable;
  Expression *value;
  Define(Symbol var, Expression *val): variable {std::move(var)}, value {val} {}
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Let : public Expression {
  std::unordered_map<Symbol, Expression*> bindings;
  Expression *body;
  Let(decltype(bindings) bn, Expression *bd): bindings {std::move(bn)}, body {bd} {}
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Clause {
  bool is_else;
  Expression *predicate;
  Expression *actions;
};

struct Cond : public Expression {
  std::vector<Clause> clauses;
  Cond(decltype(clauses) c): clauses {std::move(c)} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override {for (auto& clause : clauses) {clause.actions->tco();}}
};

struct Application : public Expression {
  Expression *op;
  ExprList params;
  bool at_tail = false;
  Application(Expression *o, ExprList p): op {o}, params {std::move(p)} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override {at_tail = true;}
};

struct And : public Expression {
  ExprList exprs;
  And(ExprList e): exprs {std::move(e)} {}
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Or : public Expression {
  ExprList exprs;
  Or(ExprList e): exprs {std::move(e)} {}
  EvalResult eval(Environment*, Interpreter&) override;
};

struct Cxr : public Expression {
  Symbol word;
  Expression *expr;
  Cxr(Symbol w, Expression *e): word {std::move(w)}, expr {e} {}
  EvalResult eval(Environment*, Interpreter&) override;
};

Expression *combine_expr(const Obj&);
Expression *classify(const Obj&);

}
